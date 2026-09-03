#define __AZ_PRIVATE_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/class.h>
#include <az/extend.h>
#include <az/interface.h>

#include "private.h"

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	#include <arikkei/arikkei-threads.h>
#endif

/*
 * Type slots hold one of:
 * - NULL - type not registered
 * - tagged (LSB set) pointer to AZTypeDescriptor - typecode reserved, class not yet constructed
 * - pointer to AZClass - fully constructed and published class
 *
 * Registration reserves the typecode immediately (descriptor in the slot); the class
 * is constructed eagerly for top-level registrations (and interfaces), or lazily by
 * az_type_get_class for registrations nested inside another class construction. This
 * makes class construction order independent of the registration entry point:
 * property/signature references are satisfied by the typecode alone, and the only
 * construction-forcing relations are extends/implements, which form a DAG in any
 * valid type graph (true cycles are detected and reported).
 */

struct _AZTypeDescriptor {
	/* Full typecode */
	uint32_t type;
	const uint8_t *name;
	uint32_t parent_type;
	uint32_t class_size;
	uint32_t instance_size;
	uint32_t flags;
	uint32_t n_ifaces_self;
	uint32_t n_props_self;
	void (*class_init) (AZClass *);
	void (*class_init_ex) (AZClass *, void *);
	void *data;
	void (*instance_init) (const AZImplementation *, void *);
	void (*instance_finalize) (const AZImplementation *, void *);
	/* Interfaces */
	uint32_t implementation_size;
	void (*implementation_init) (AZImplementation *);
	/* Construction is in progress (circular construction detection) */
	uint32_t constructing;
};

#if defined(AZ_GLOBALS_STATIC)
	static mtx_t mutex;
	_Atomic(uintptr_t) az_types[AZ_MAX_TYPES];
	_Atomic unsigned int az_num_types = 0;
	#define REGISTRY_LOCK() mtx_lock(&mutex)
	#define REGISTRY_UNLOCK() mtx_unlock(&mutex)
	static inline void ensure_type (void) {}
	static inline uintptr_t slot_load (unsigned int idx) {
		return atomic_load_explicit (&az_types[idx], memory_order_acquire);
	}
	static inline void slot_store (unsigned int idx, uintptr_t p) {
		atomic_store_explicit (&az_types[idx], p, memory_order_release);
	}
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	uintptr_t *az_types = NULL;
	unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	#define REGISTRY_LOCK()
	#define REGISTRY_UNLOCK()
	static inline void ensure_type (void) {
		if (az_num_types >= az_num_types_allocated) {
			unsigned int old_allocated = az_num_types_allocated;
			az_num_types_allocated += 32;
			az_types = (uintptr_t *) realloc (az_types, az_num_types_allocated * sizeof(uintptr_t));
			/* New slots have to be NULL: NULL denotes "not registered" */
			memset (az_types + old_allocated, 0, 32 * sizeof (uintptr_t));
		}
	}
	static inline uintptr_t slot_load (unsigned int idx) { return az_types[idx]; }
	static inline void slot_store (unsigned int idx, uintptr_t p) { az_types[idx] = p; }
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	static mtx_t mutex;
	static uintptr_t *az_types = NULL;
	static unsigned int az_num_types = 0;
	static unsigned int az_num_types_allocated = 0;
	#define REGISTRY_LOCK() mtx_lock(&mutex)
	#define REGISTRY_UNLOCK() mtx_unlock(&mutex)
	static inline void ensure_type (void) {
		if (az_num_types >= az_num_types_allocated) {
			unsigned int old_allocated = az_num_types_allocated;
			az_num_types_allocated += 32;
			az_types = (uintptr_t *) realloc (az_types, az_num_types_allocated * sizeof(uintptr_t));
			/* New slots have to be NULL: NULL denotes "not registered" */
			memset (az_types + old_allocated, 0, 32 * sizeof (uintptr_t));
		}
	}
	static inline uintptr_t slot_load (unsigned int idx) { return az_types[idx]; }
	static inline void slot_store (unsigned int idx, uintptr_t p) { az_types[idx] = p; }
#endif

/* Number of in-flight class construction frames (the registry lock is held) */
static unsigned int az_construction_depth = 0;

void
az_globals_init (void)
{
	if (az_num_types) return;

#if defined(AZ_GLOBALS_STATIC)
	/* Fixed-length array is zero-initialized statically */
	mtx_init(&mutex, mtx_plain | mtx_recursive);
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (uintptr_t *) malloc (az_num_types_allocated * sizeof(uintptr_t));
	/* All slots have to be NULL: NULL denotes "not registered" */
	memset(az_types, 0, az_num_types_allocated * sizeof(uintptr_t));
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_init(&mutex, mtx_plain | mtx_recursive);
	az_num_types_allocated = AZ_NUM_BASE_TYPES + 32;
	az_types = (uintptr_t *) malloc (az_num_types_allocated * sizeof(uintptr_t));
	/* All slots have to be NULL: NULL denotes "not registered" */
	memset(az_types, 0, az_num_types_allocated * sizeof(uintptr_t));
#endif

	az_num_types = AZ_NUM_BASE_TYPES;
}

void
az_class_publish (AZClass *klass)
{
	unsigned int idx = AZ_TYPE_INDEX(klass->impl.type);
#ifdef AZ_SAFETY_CHECKS
	/* The CONSTRUCT bit has to agree between the typecode and the class flags */
	/* (az_instance_init_by_type/finalize_by_type gate on the typecode) */
	arikkei_return_if_fail (!(klass->impl.type & AZ_FLAG_CONSTRUCT) == !(klass->impl.flags & AZ_FLAG_CONSTRUCT));
#endif
#if defined(AZ_GLOBALS_STATIC)
#ifdef AZ_SAFETY_CHECKS
	uintptr_t old = slot_load (idx);
	/* The slot may be empty, hold this class, or hold the descriptor of the class under construction */
	arikkei_return_if_fail (!old || (old == (uintptr_t) klass) || (old & 1));
#endif
	/* Release store pairs with the acquire load in the AZ_CLASS_FROM_TYPE fast path */
	slot_store (idx, (uintptr_t) klass);
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
#ifdef AZ_SAFETY_CHECKS
	uintptr_t old = slot_load (idx);
	arikkei_return_if_fail (!old || (old == (uintptr_t) klass) || (old & 1));
#endif
	slot_store (idx, (uintptr_t) klass);
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	mtx_lock(&mutex);
	uintptr_t old = az_types[idx];
	unsigned int ok = !old || (old == (uintptr_t) klass) || (old & 1);
	if (ok) az_types[idx] = (uintptr_t) klass;
	mtx_unlock(&mutex);
	arikkei_return_if_fail (ok);
#endif
}

/* The registry lock is held by the caller */
static AZTypeDescriptor *
az_type_reserve (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *), void (*class_init_ex) (AZClass *, void *), void *data,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	unsigned int implementation_size, void (*implementation_init) (AZImplementation *))
{
	if ((flags & AZ_FLAG_ZERO_MEMORY) || n_interfaces_self || instance_init || instance_finalize) {
		flags |= AZ_FLAG_CONSTRUCT;
	}
	ensure_type();
	unsigned int idx = az_num_types++;
	/* The typecode inherits the parent typecode flags plus the registration flags */
	/* (AZ_FLAG_ABSTRACT/AZ_FLAG_BOXED are class flags and never enter the typecode) */
	uint32_t typecode = idx | ((parent_type | flags) & ~AZ_TYPE_MASK);
	AZTypeDescriptor *desc = (AZTypeDescriptor *) calloc (1, sizeof (AZTypeDescriptor));
	desc->type = typecode;
	desc->name = name;
	desc->parent_type = parent_type;
	desc->class_size = class_size;
	desc->instance_size = instance_size;
	desc->flags = flags;
	desc->n_ifaces_self = n_interfaces_self;
	desc->n_props_self = n_properties_self;
	desc->class_init = class_init;
	desc->class_init_ex = class_init_ex;
	desc->data = data;
	desc->instance_init = instance_init;
	desc->instance_finalize = instance_finalize;
	desc->implementation_size = implementation_size;
	desc->implementation_init = implementation_init;
	/* The descriptor has to be in the slot before the typecode is published: a thread
	 * that sees the typecode is guaranteed to see the descriptor */
	slot_store (idx, (uintptr_t) desc | 1);
	/* The typecode is published before construction so it is available for circular
	 * type references in class constructors */
	atomic_store_explicit((_Atomic unsigned int *)type, typecode, memory_order_release);
	return desc;
}

/* The registry lock is held by the caller */
static AZClass *
az_type_construct (AZTypeDescriptor *desc)
{
	if (desc->constructing) {
		fprintf (stderr, "az_type_construct: circular class construction involving type 0x%08x (%s)\n",
			desc->type, (const char *) desc->name);
		return NULL;
	}
	desc->constructing = 1;
	az_construction_depth += 1;
	/* az_class_new force-constructs the parent class */
	AZClass *klass = az_class_new (desc->name, desc->parent_type, desc->class_size, desc->instance_size, desc->flags,
		desc->instance_init, desc->instance_finalize);
	if (klass) {
		klass->impl.type = desc->type;
		if (AZ_CLASS_IS_INTERFACE(klass)) {
			AZInterfaceClass *if_klass = (AZInterfaceClass *) klass;
			if_klass->implementation_size = desc->implementation_size;
			if_klass->implementation_init = desc->implementation_init;
		}
		if (desc->n_ifaces_self) az_class_set_num_interfaces (klass, desc->n_ifaces_self);
		if (desc->n_props_self) az_class_set_num_properties (klass, desc->n_props_self);
		if (desc->class_init) desc->class_init (klass);
		if (desc->class_init_ex) desc->class_init_ex (klass, desc->data);
		az_class_post_init (klass);
		az_class_publish (klass);
		free (desc);
	} else {
		desc->constructing = 0;
	}
	az_construction_depth -= 1;
	return klass;
}

AZClass *
az_type_register_internal (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *), void (*class_init_ex) (AZClass *, void *), void *data,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	unsigned int implementation_size, void (*implementation_init) (AZImplementation *),
	unsigned int force_construct)
{
	AZClass *klass = NULL;
	REGISTRY_LOCK();
	AZTypeDescriptor *desc = az_type_reserve (type, name, parent_type, class_size, instance_size, flags,
		n_interfaces_self, n_properties_self,
		class_init, class_init_ex, data,
		instance_init, instance_finalize,
		implementation_size, implementation_init);
	/* Top-level registrations (and interfaces) construct eagerly, nested ones are deferred */
	if (desc && (force_construct || !az_construction_depth)) {
		klass = az_type_construct (desc);
	}
	REGISTRY_UNLOCK();
	return klass;
}

AZClass *
az_type_get_class (unsigned int type)
{
#if defined(AZ_GLOBALS_SINGLE_THREAD)
	if (!az_num_types) az_init ();
#endif
	AZClass *klass = NULL;
	REGISTRY_LOCK();
	uintptr_t p = 0;
	if (AZ_TYPE_INDEX(type) < az_num_types) p = slot_load (AZ_TYPE_INDEX(type));
	if (p & 1) {
		/* Reserved but not yet constructed: construct on demand */
		klass = az_type_construct ((AZTypeDescriptor *) (p & ~(uintptr_t) 1));
	} else {
		klass = (AZClass *) p;
	}
	REGISTRY_UNLOCK();
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
#endif
	return klass;
}

#if defined(AZ_GLOBALS_MULTI_THREAD)
unsigned int
az_type_is_valid (uint32_t type)
#else
unsigned int
az_type_is_valid_deferred (uint32_t type)
#endif
{
	unsigned int valid = 0;
	REGISTRY_LOCK();
	uintptr_t p = 0;
	if (AZ_TYPE_INDEX(type) && (AZ_TYPE_INDEX(type) < az_num_types)) p = slot_load (AZ_TYPE_INDEX(type));
	if (p & 1) {
		valid = (((AZTypeDescriptor *) (p & ~(uintptr_t) 1))->type == type);
	} else if (p) {
		valid = (((AZClass *) p)->impl.type == type);
	}
	REGISTRY_UNLOCK();
	return valid;
}

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	void
	az_types_lock()
	{
		mtx_lock(&mutex);
	}

	void
	az_types_unlock()
	{
		mtx_unlock(&mutex);
	}
#endif

/* The trailing-ones mask of the instance alignment (0 = 1 byte ... 127 = 128 bytes) */
static unsigned int
az_alignment_bucket (uint8_t alignment)
{
	unsigned int k = 0;
	while ((k < 7) && (alignment & (1u << k))) k += 1;
	return k;
}

void
az_classes_print_stats (void)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
#endif
	unsigned int n_classes = 0, n_reserved = 0, n_fundamental = 0;
	unsigned int n_struct = 0, n_block = 0, n_iface = 0, n_ref = 0, n_obj = 0;
	unsigned int n_abstract = 0, n_final = 0, n_zero_memory = 0, n_construct = 0;
	unsigned int n_init = 0, n_finalize = 0;
	unsigned int n_self_ifaces = 0, n_inherited_ifaces = 0;
	unsigned int n_props = 0;
	unsigned int n_struct_gt16 = 0, n_struct_gt64 = 0;
	unsigned int n_custom_allocator = 0, n_custom_delegate = 0;
	unsigned int max_ifaces_self = 0, max_ifaces_all = 0, max_props = 0;
	unsigned int max_class_size = 0, max_instance_size = 0;
	const AZClass *max_class_size_class = NULL, *max_instance_size_class = NULL;
	uint64_t sum_class_size = 0, sum_ifaces_all = 0, sum_props = 0;
	unsigned int n_class_le64 = 0, n_class_le128 = 0, n_class_gt128 = 0;
	unsigned int iface_hist[4] = { 0 }; /* n_ifaces_all: 0, 1, 2, >2 */
	unsigned int align_hist[8] = { 0 }; /* instance alignment 1 << k */
	for (unsigned int i = 0; i < az_num_types; i++) {
		uintptr_t p = slot_load (i);
		if (!p) continue;
		/* Tagged (LSB set) slot: reserved but not constructed yet */
		if (p & 1) {
			n_reserved += 1;
			continue;
		}
		const AZClass *klass = (const AZClass *) p;
		uint32_t flags = klass->impl.flags;
		n_classes += 1;
		if (i < AZ_NUM_FUNDAMENTAL_TYPES) n_fundamental += 1;
		/* Categories are exclusive by priority: iface > object > reference > block > struct */
		if (flags & AZ_FLAG_INTERFACE) {
			n_iface += 1;
		} else if (flags & AZ_FLAG_OBJECT) {
			n_obj += 1;
		} else if (flags & AZ_FLAG_REFERENCE) {
			n_ref += 1;
		} else if (flags & AZ_FLAG_BLOCK) {
			n_block += 1;
		} else if (!(flags & AZ_FLAG_ABSTRACT)) {
			unsigned int vsize = az_class_value_size (klass);
			n_struct += 1;
			if (vsize > 16) n_struct_gt16 += 1;
			if (vsize > 64) n_struct_gt64 += 1;
			if (klass->instance_size > max_instance_size) {
				max_instance_size = klass->instance_size;
				max_instance_size_class = klass;
			}
		}
		if (flags & AZ_FLAG_ABSTRACT) n_abstract += 1;
		if (flags & AZ_FLAG_FINAL) n_final += 1;
		if (flags & AZ_FLAG_ZERO_MEMORY) n_zero_memory += 1;
		if (flags & AZ_FLAG_CONSTRUCT) n_construct += 1;
		if (klass->instance_init) n_init += 1;
		if (klass->instance_finalize) n_finalize += 1;
		if (klass->n_ifaces_self) n_self_ifaces += 1;
		if (klass->n_ifaces_all > klass->n_ifaces_self) n_inherited_ifaces += 1;
		if (klass->n_props_self) n_props += 1;
		if (klass->allocator_idx) n_custom_allocator += 1;
		if (klass->delegate_idx) n_custom_delegate += 1;
		if (klass->n_ifaces_self > max_ifaces_self) max_ifaces_self = klass->n_ifaces_self;
		if (klass->n_ifaces_all > max_ifaces_all) max_ifaces_all = klass->n_ifaces_all;
		if (klass->n_props_self > max_props) max_props = klass->n_props_self;
		if (klass->class_size > max_class_size) {
			max_class_size = klass->class_size;
			max_class_size_class = klass;
		}
		sum_class_size += klass->class_size;
		sum_ifaces_all += klass->n_ifaces_all;
		sum_props += klass->n_props_self;
		if (klass->class_size <= 64) {
			n_class_le64 += 1;
		} else if (klass->class_size <= 128) {
			n_class_le128 += 1;
		} else {
			n_class_gt128 += 1;
		}
		iface_hist[(klass->n_ifaces_all > 2) ? 3 : klass->n_ifaces_all] += 1;
		align_hist[az_alignment_bucket (klass->alignment)] += 1;
	}
	fprintf (stdout, "az class statistics\n");
	fprintf (stdout, "  classes: %u (%u fundamental, %u registered but not constructed)\n", n_classes, n_fundamental, n_reserved);
	fprintf (stdout, "  categories: %u structs, %u blocks, %u interfaces, %u references, %u objects\n", n_struct, n_block, n_iface, n_ref, n_obj);
	fprintf (stdout, "  struct value sizes: %u > 16 bytes, %u > 64 bytes\n", n_struct_gt16, n_struct_gt64);
	fprintf (stdout, "  flags: %u abstract, %u final, %u zero-memory, %u construct\n", n_abstract, n_final, n_zero_memory, n_construct);
	fprintf (stdout, "  lifecycle: %u instance_init, %u instance_finalize\n", n_init, n_finalize);
	fprintf (stdout, "  interfaces: %u with self, %u with inherited/transitive (all > self)\n", n_self_ifaces, n_inherited_ifaces);
	fprintf (stdout, "  iface list sizes: 0: %u, 1: %u, 2: %u, >2 (heap): %u\n", iface_hist[0], iface_hist[1], iface_hist[2], iface_hist[3]);
	fprintf (stdout, "  iface entries: %llu total, max self %u, max all %u\n", (unsigned long long) sum_ifaces_all, max_ifaces_self, max_ifaces_all);
	fprintf (stdout, "  properties: %u classes, %llu total fields, max %u per class\n", n_props, (unsigned long long) sum_props, max_props);
	fprintf (stdout, "  class size: max %u", max_class_size);
	if (max_class_size_class) fprintf (stdout, " (%s)", max_class_size_class->name);
	fprintf (stdout, ", avg %u, <=64: %u, 65..128: %u, >128: %u\n",
		(n_classes) ? (unsigned int) (sum_class_size / n_classes) : 0, n_class_le64, n_class_le128, n_class_gt128);
	fprintf (stdout, "  instance size: max %u", max_instance_size);
	if (max_instance_size_class) fprintf (stdout, " (%s)", max_instance_size_class->name);
	fprintf (stdout, "\n");
	fprintf (stdout, "  instance alignment: 1: %u, 2: %u, 4: %u, 8: %u, 16: %u, 32: %u, 64: %u, 128: %u\n",
		align_hist[0], align_hist[1], align_hist[2], align_hist[3], align_hist[4], align_hist[5], align_hist[6], align_hist[7]);
	fprintf (stdout, "  custom allocators: %u, delegates: %u\n", n_custom_allocator, n_custom_delegate);
}
