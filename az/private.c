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
	/* The typecode flags replicate the az_class_new inheritance (parent flags, ABSTRACT cleared) */
	uint32_t typecode = idx | (((AZ_TYPE_FLAGS(parent_type) & ~AZ_FLAG_ABSTRACT) | flags) & ~AZ_TYPE_MASK);
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
