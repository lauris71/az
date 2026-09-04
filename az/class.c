#define __AZ_CLASS_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <malloc.h>
#endif

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/extend.h>
#include <az/function-native.h>
#include <az/function-value.h>
#include <az/packed-value.h>
#include <az/private.h>
#include <az/string.h>

static unsigned char zero_val[16] = { 0 };

/* Index 0 is the default (malloc/free) allocator */
const AZInstanceAllocator *az_class_allocators[AZ_MAX_CLASS_ALLOCATORS] = { NULL };
static unsigned int az_num_class_allocators = 1;

unsigned int
az_class_register_allocator (const AZInstanceAllocator *allocator)
{
	arikkei_return_val_if_fail (allocator != NULL, 0);
	arikkei_return_val_if_fail (az_num_class_allocators < AZ_MAX_CLASS_ALLOCATORS, 0);
	unsigned int idx = az_num_class_allocators++;
	az_class_allocators[idx] = allocator;
	return idx;
}

/* Method implementations */
static unsigned int impl_call_setStaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
static unsigned int impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

/* Properties */

enum {
	FUNC_SETSTATICPROPERTY,
	FUNC_GETSTATICPROPERTY,
	NUM_PROPERTIES
};

void
az_impl_class_post_init (void)
{
	az_class_set_num_properties (&AZImplKlass, NUM_PROPERTIES);
	az_class_define_method_va (&AZImplKlass, FUNC_SETSTATICPROPERTY, (const unsigned char *) "setStaticProperty", impl_call_setStaticProperty, AZ_TYPE_NONE, 2, AZ_TYPE_STRING, AZ_TYPE_ANY);
	az_class_define_method_va (&AZImplKlass, FUNC_GETSTATICPROPERTY, (const unsigned char *) "getStaticProperty", impl_call_getstaticProperty, AZ_TYPE_ANY, 1, AZ_TYPE_STRING);
}

static unsigned int
impl_call_setStaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *def_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &def_class, &sub_impl, &sub_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &def_class->props_self[prop_idx];
	arikkei_return_val_if_fail (AZ_FIELD_SPEC(prop) == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (!AZ_FIELD_IS_FINAL(prop), 0);
	arikkei_return_val_if_fail (AZ_FIELD_WRITE(prop) != AZ_FIELD_WRITE_NONE, 0);
	az_instance_set_property_by_id (def_class, sub_impl, NULL, prop_idx, arg_impls[2], az_value_get_inst(arg_impls[2], arg_vals[2]), ctx);
	return 1;
}

static unsigned int
impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *def_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &def_class, &sub_impl, &sub_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &def_class->props_self[prop_idx];
	arikkei_return_val_if_fail (AZ_FIELD_SPEC(prop) == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (AZ_FIELD_READ(prop) != AZ_FIELD_READ_NONE, 0);
	az_instance_get_property_by_id (def_class, AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, NULL, prop_idx, ret_impl, &ret_val->value, 64, NULL);
	return 1;
}

void
az_class_class_post_init (void)
{
	az_class_set_num_properties (&AZClassKlass, 1);
	az_class_define_property (&AZClassKlass, 0, (const unsigned char *) "parent", AZ_TYPE_CLASS, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZClass, parent), NULL, NULL);
}

AZClass *
az_class_new (const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
#endif
	arikkei_return_val_if_fail (!AZ_TYPE_IS_FINAL(parent_type), 0);

	/* Classes are cache-line aligned and padded so they never share line tails */
	unsigned int alloc_size = (class_size + AZ_CLASS_ALIGNMENT - 1) & ~(unsigned int) (AZ_CLASS_ALIGNMENT - 1);
#if defined(_WIN32)
	AZClass *klass = (AZClass *) _aligned_malloc (alloc_size, AZ_CLASS_ALIGNMENT);
#else
	AZClass *klass = (AZClass *) aligned_alloc (AZ_CLASS_ALIGNMENT, alloc_size);
#endif
	arikkei_return_val_if_fail (klass != NULL, NULL);
	memset (klass, 0, alloc_size);
	if (parent_type) {
		/* The parent class is constructed on demand */
		AZClass *parent_class = az_type_get_class (parent_type);
		arikkei_return_val_if_fail (parent_class != NULL, NULL);
#ifdef AZ_SAFETY_CHECKS
		arikkei_return_val_if_fail (class_size >= parent_class->class_size, NULL);
		arikkei_return_val_if_fail (instance_size >= parent_class->instance_size, NULL);
#endif
		memcpy (klass, parent_class, parent_class->class_size);
		/* Overwrite values from supertype */
		/* ABSTRACT and HAS_DEFAULT are never propagated; the work flags are recomputed in az_class_post_init */
		klass->impl.flags &= ~(AZ_FLAG_ABSTRACT | AZ_FLAG_HAS_DEFAULT | AZ_COMPUTED_FLAG_MASK);
		klass->impl.type = 0;
		klass->parent = parent_class;
		klass->n_ifaces_self = 0;
		klass->n_props_self = 0;
		klass->props_self = NULL;
	}
	klass->impl.flags |= flags;
	klass->name = name;
	klass->class_size = class_size;
	klass->instance_size = instance_size;
	klass->instance_init = instance_init;
	klass->instance_finalize = instance_finalize;

	/* The typecode is assigned and the class published by the caller (az_type_construct) */
	return klass;
}

void
az_class_new_with_value (AZClass *klass)
{
	/* Fundamental classes are statically initialized; publish immediately */
	/* (az_init runs before any concurrent access) */
	az_class_publish (klass);
}

void
az_class_set_num_interfaces (AZClass *klass, unsigned int n_ifaces)
{
	arikkei_return_if_fail (n_ifaces <= UINT8_MAX);
	klass->n_ifaces_self = n_ifaces;
	if (n_ifaces > 2) {
		/* Temporary storage for the declarations; az_class_post_init either
		 * reuses it as the full interface list or frees it again */
		klass->ifaces_all = (AZIFEntry *) malloc(n_ifaces * sizeof(AZIFEntry));
#ifdef VERBOSE
		static unsigned int n_allocations = 0, allocated = 0;
		n_allocations += 1;
		allocated += n_ifaces;
		fprintf(stderr, "az_class_set_num_interfaces(): Allocated %u (%u %u)\n", n_ifaces, n_allocations, allocated);
#endif
#ifdef AZ_SAFETY_CHECKS
        memset (klass->ifaces_all, 0, n_ifaces * sizeof (AZIFEntry));
#endif
	} else {
#ifdef AZ_SAFETY_CHECKS
        memset (klass->ifaces, 0, n_ifaces * sizeof (AZIFEntry));
#endif
	}
}

void
az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_ifaces_self);
	arikkei_return_if_fail (AZ_TYPE_IS_INTERFACE(type));
	arikkei_return_if_fail (impl_offset <= UINT16_MAX);
	arikkei_return_if_fail (inst_offset <= UINT16_MAX);
#endif
	/*
	 * The interface class is needed to initialize the embedded implementation and to
	 * resolve the interface chain in az_class_post_init; az_type_get_class constructs
	 * it on demand. NULL means a genuine extends/implements cycle (the interface is
	 * already being constructed), which is rejected with a clear warning.
	 */
	AZClass *iface_class = az_type_get_class (type);
	if (!iface_class) {
		fprintf (stderr, "az_class_declare_interface: %s cannot declare interface type %u:"
			" circular interface implementation\n",
			(const char *) klass->name, type);
		return;
	}
#ifdef AZ_SAFETY_CHECKS
	/*
	 * Value type instances are memcpy'd and cleared without destruction, so a value
	 * class may only implement interfaces whose instance state is destructor-free
	 * (see "Value type (struct) requirements" in class.h)
	 */
	if (!(klass->impl.flags & AZ_FLAG_BLOCK)) {
		if (iface_class->instance_finalize) {
			fprintf (stderr, "az_class_declare_interface: value class %s implements interface %s"
				" which has a finalizer; value instances are memcpy'd and never destructed\n",
				(const char *) klass->name, (const char *) iface_class->name);
		}
		for (unsigned int i = 0; i < iface_class->n_ifaces_all; i++) {
			const AZIFEntry *sub_entry = az_class_iface_all (iface_class, i);
			if (!sub_entry->type) continue;
			AZClass *sub_class = AZ_CLASS_FROM_TYPE (sub_entry->type);
			if (sub_class->instance_finalize) {
				fprintf (stderr, "az_class_declare_interface: value class %s implements interface %s"
					" whose super-interface %s has a finalizer; value instances are memcpy'd and never destructed\n",
					(const char *) klass->name, (const char *) iface_class->name, (const char *) sub_class->name);
			}
		}
	}
#endif
	AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[idx] : &klass->ifaces_all[idx];
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!ifentry->type);
#endif
	*ifentry = (AZIFEntry) {type, impl_offset, inst_offset};
	/* if class is interface, sub-implementations are defined in it's standalone implementations instead */
	if (!AZ_CLASS_IS_INTERFACE(klass)) {
		/* Init implementation */
		az_implementation_init_by_type ((AZImplementation *) ((char *) klass + ifentry->impl_offset), ifentry->type);
	}
}

#define noVERBOSE

void
az_class_post_init (AZClass *klass)
{
	unsigned int i;
	/*
	 * Until this point the self interface declarations live in the inline array
	 * (n_ifaces_self <= 2) or in a temporary heap array pointed to by ifaces_all
	 * (n_ifaces_self > 2). The full list cannot be built earlier because its
	 * size is only known once the transitive closures are counted below.
	 */
	AZIFEntry *self = (klass->n_ifaces_self <= 2) ? klass->ifaces : klass->ifaces_all;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!(klass->alignment & (klass->alignment + 1)));
	/*
	 * Value type instances are memcpy'd and cleared without destruction, so a
	 * value class must not rely on a finalizer (see "Value type (struct)
	 * requirements" in class.h)
	 */
	if (!(klass->impl.flags & (AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT)) && klass->instance_finalize) {
		fprintf (stderr, "az_class_post_init: value class %s has a finalizer;"
			" value instances are memcpy'd and never destructed\n", (const char *) klass->name);
	}
	for (i = 0; i < klass->n_ifaces_self; i++) {
		if (!self[i].type) {
			fprintf (stderr, "az_class_post_init: Klass %s interface %u is not defined\n", klass->name, i);
		}
	}
	for (i = 0; i < klass->n_props_self; i++) {
		if (!klass->props_self[i].key) {
			fprintf (stderr, "az_class_post_init: Klass %s property %u is not defined\n", klass->name, i);
		}
	}
#endif
	if (klass->n_ifaces_self) {
		/*
		 * Count all interfaces (n_ifaces_all has the parent value initially).
		 * Rejected declarations (type == 0) keep their slot in the list but do
		 * not contribute a transitive closure.
		 */
		for (i = 0; i < klass->n_ifaces_self; i++) {
			klass->n_ifaces_all += 1;
			if (self[i].type) {
				AZClass *iface_class = AZ_CLASS_FROM_TYPE(self[i].type);
				klass->n_ifaces_all += iface_class->n_ifaces_all;
			}
		}
		AZIFEntry *ifaces;
		if (klass->n_ifaces_all <= 2) {
			/* The full list fits inline; self is inline too (n_ifaces_all >= n_ifaces_self) */
			ifaces = klass->ifaces;
		} else if ((klass->n_ifaces_self > 2) && (klass->n_ifaces_all == klass->n_ifaces_self)) {
			/* No transitives and no inherited interfaces: the temporary array is the full list */
			ifaces = self;
		} else {
#ifdef VERBOSE
			static unsigned int n_allocations = 0, allocated = 0;
			n_allocations += 1;
			allocated += klass->n_ifaces_all;
			fprintf(stderr, "az_class_post_init(): Allocated all %u (%u %u)\n", klass->n_ifaces_all, n_allocations, allocated);
#endif
			ifaces = (AZIFEntry *) malloc(klass->n_ifaces_all * sizeof(AZIFEntry));
			/*
			 * A class may implement the same interface multiple times: directly and
			 * transitively through other interfaces. Direct (self) declarations come
			 * first so they win the az_instance_get_interface lookup.
			 */
			memcpy (ifaces, self, klass->n_ifaces_self * sizeof (AZIFEntry));
		}
		unsigned int idx = klass->n_ifaces_self;
		/* Transitive closures: the offsets have to be composed with the self entry's */
		for (i = 0; i < klass->n_ifaces_self; i++) {
			/* Rejected declarations have no closure */
			if (!ifaces[i].type) continue;
			AZClass *iface_class = AZ_CLASS_FROM_TYPE(ifaces[i].type);
			for (unsigned int j = 0; j < iface_class->n_ifaces_all; j++) {
				const AZIFEntry *sub_entry = az_class_iface_all(iface_class, j);
				ifaces[idx].type = sub_entry->type;
				ifaces[idx].impl_offset = ifaces[i].impl_offset + sub_entry->impl_offset;
				ifaces[idx].inst_offset = ifaces[i].inst_offset + sub_entry->inst_offset;
				idx += 1;
			}
		}
		/* Parent offsets are class-absolute (the parent is embedded at offset 0) */
		if (klass->parent) {
			memcpy (&ifaces[idx], az_class_iface_all(klass->parent, 0), klass->parent->n_ifaces_all * sizeof (AZIFEntry));
		}
		if (ifaces != self) {
			/* Release the temporary declaration array */
			if (klass->n_ifaces_self > 2) free (self);
			if (ifaces != klass->ifaces) klass->ifaces_all = ifaces;
		}
	}
	/*
	 * Compute the construction work flags (single-word tests on the hot paths).
	 * Everything below is derived from the just-built interface list, the class
	 * virtuals set by class_init and the (already post-initialized) parent class.
	 */
	if (klass->impl.flags & AZ_FLAG_HAS_DEFAULT) {
#ifdef AZ_SAFETY_CHECKS
		/* A default value is only valid for concrete value types with fixed storage */
		arikkei_return_if_fail (!(klass->impl.flags & (AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT)));
		arikkei_return_if_fail (klass->instance_size > 0);
		arikkei_return_if_fail (klass->default_value != NULL);
#endif
	} else if (klass->instance_init) {
		/* The union holds default_value instead when AZ_FLAG_HAS_DEFAULT is set */
		klass->impl.flags |= AZ_FLAG_HAS_INSTANCE_INIT;
	}
	if (klass->instance_finalize) klass->impl.flags |= AZ_FLAG_HAS_INSTANCE_FINALIZE;
	for (i = 0; i < klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self (klass, i);
		/* az_type_reserve sets CONSTRUCT for interfaces needing (de)construction */
		if (!ifentry->type || !(AZ_TYPE_FLAGS(ifentry->type) & AZ_FLAG_CONSTRUCT)) continue;
		AZClass *iface_class = AZ_CLASS_FROM_TYPE (ifentry->type);
		/*
		 * An interface whose only construction work is zeroing its region does not
		 * need the interface walk: promote the zeroing to the whole instance
		 * (if any sub-component needs zeroed memory, the instance is zeroed).
		 * Interface classes are fully constructed at this point, so their work
		 * flags are final (and the promotion cascades through interface inheritance).
		 */
		if (!(iface_class->impl.flags & (AZ_INIT_WORK_MASK | AZ_FINALIZE_WORK_MASK))) {
			if (iface_class->impl.flags & AZ_FLAG_ZERO_MEMORY) klass->impl.flags |= AZ_FLAG_ZERO_MEMORY;
			continue;
		}
		klass->impl.flags |= AZ_FLAG_HAS_IFACE_CONSTRUCT;
	}
	if (klass->parent) {
		uint32_t pflags = klass->parent->impl.flags;
		if (pflags & AZ_INIT_WORK_MASK) klass->impl.flags |= AZ_FLAG_PARENT_CONSTRUCT;
		/* A default value replaces the whole parent subtree, so nothing below it needs finalization */
		if (!(pflags & AZ_FLAG_HAS_DEFAULT) && (pflags & AZ_FINALIZE_WORK_MASK)) klass->impl.flags |= AZ_FLAG_PARENT_FINALIZE;
	}
#ifdef VERBOSE
	if (klass->n_ifaces_all) {
		fprintf (stderr, "Class %s\n", klass->name);
		fprintf (stderr, "  Self %u\n", klass->n_ifaces_self);
		for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
			const AZIFEntry *ifentry = az_class_iface_self(klass, i);
			fprintf (stderr, "    %d: type %d\n", i, ifentry->type);
		}
		fprintf (stderr, "  All %u\n", klass->n_ifaces_all);
		for (uint16_t i = 0; i < klass->n_ifaces_all; i++) {
			const AZIFEntry *ifentry = az_class_iface_all(klass, i);
			fprintf (stderr, "    %d: type %d\n", i, ifentry->type);
		}
	}
#endif
}

void
az_class_set_num_properties (AZClass *klass, unsigned int nproperties)
{
	klass->n_props_self = nproperties;
	klass->props_self = (AZField *) malloc (nproperties * sizeof (AZField));
	memset (klass->props_self, 0, nproperties * sizeof (AZField));
}

void az_class_define_property_value (AZClass *klass, unsigned int idx, const uint8_t *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int write, unsigned int offset)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_props_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_value (klass->props_self + idx, key, type, is_final, spec, AZ_FIELD_READ_VALUE, write, offset);
}

void az_class_define_property_packed (AZClass *klass, unsigned int idx, const uint8_t *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int write, unsigned int offset)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_props_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_value (klass->props_self + idx, key, type, is_final, spec, AZ_FIELD_READ_PACKED, write, offset);
}

void az_class_define_property (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_props_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail (!impl || (az_type_is_assignable_to(AZ_IMPL_TYPE(impl), type)));
#endif
	if ((read == AZ_FIELD_READ_VALUE) || (read == AZ_FIELD_READ_INSTANCE) || (read == AZ_FIELD_READ_PACKED)) {
		az_field_setup_value (klass->props_self + idx, key, type, is_final, spec, read, write, offset);
	} else if (read == AZ_FIELD_READ_METHOD) {
		az_field_setup_method (klass->props_self + idx, key, type, is_final, spec, read, write);
	} else {
		az_field_setup_stored (klass->props_self + idx, key, type, is_final, spec, read, write, impl, inst);
	}
}

void
az_class_define_property_function_val (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_props_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_function (klass->props_self + idx, key, is_final, spec, read, write, sig, impl, inst);
}

void
az_class_define_property_function_packed (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_props_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_function_packed (klass->props_self + idx, key, is_final, spec, read, write, sig, offset);
}

void
az_class_define_method (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *))
{
	AZFunctionSignature *sig;
	AZFunctionValue fval;
	sig = az_function_signature_new(AZ_CLASS_TYPE(klass), ret_type, n_args, arg_types);
	az_function_value_setup (&fval, sig, invoke);
	az_class_define_property_function_val (klass, idx, key, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_STORED_STATIC, AZ_FIELD_WRITE_NONE, sig,
		(AZImplementation *) az_type_get_class (AZ_TYPE_FUNCTION_VALUE), &fval);
}

void
az_class_define_method_va(AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail(n_args < 64);
	va_start(ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg(ap, unsigned int);
	}
	va_end(ap);
	az_class_define_method(klass, idx, key, ret_type, n_args, arg_types, invoke);
}

void
az_class_define_static_method (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *))
{
	AZFunctionSignature *sig;
	AZFunctionValue fval;
	sig = az_function_signature_new (AZ_TYPE_NONE, ret_type, n_args, arg_types);
	az_function_value_setup (&fval, sig, invoke);
	az_class_define_property_function_val (klass, idx, key, 1, AZ_FIELD_CLASS, AZ_FIELD_READ_STORED_STATIC, AZ_FIELD_WRITE_NONE, sig,
		(AZImplementation *) az_type_get_class(AZ_TYPE_FUNCTION_VALUE), &fval);
}

void az_class_define_static_method_native (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	void (*invoke) (void))
{
	AZFunctionSignature *sig;
	AZFunctionNative fval;
	sig = az_function_signature_new (AZ_TYPE_NONE, ret_type, n_args, arg_types);
	az_function_native_setup (&fval, sig, invoke);
	az_class_define_property_function_val (klass, idx, key, 1, AZ_FIELD_CLASS, AZ_FIELD_READ_STORED_STATIC, AZ_FIELD_WRITE_NONE, sig,
		(AZImplementation *) az_type_get_class(AZ_TYPE_FUNCTION_NATIVE), &fval);
}

void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail (n_args < 64);

	va_start (ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg (ap, unsigned int);
	}
	va_end (ap);
	az_class_define_static_method (klass, idx, key, ret_type, n_args, arg_types, invoke);
}

void az_class_define_static_method_native_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	void (*invoke) (void),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail (n_args < 64);

	va_start (ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg (ap, unsigned int);
	}
	va_end (ap);
	az_class_define_static_method_native(klass, idx, key, ret_type, n_args, arg_types, invoke);
}

int
az_class_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (uint16_t i = 0; i < (int) klass->n_props_self; i++) {
		if (az_string_equals(key, klass->props_self[i].key)) {
			*def_class = klass;
			if (sub_impl) *sub_impl = impl;
			if (sub_inst) *sub_inst = inst;
			return i;
		}
	}
	/* interfaces */
	for (uint16_t i = 0; i < (int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		/* Skip entries whose declaration was rejected during construction */
		if (!ifentry->type) continue;
		AZClass *if_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *if_impl = (impl) ? (AZImplementation *) ((char *) impl + ifentry->impl_offset) : NULL;
		void *if_inst = (inst) ? (void *) ((char *) inst + ifentry->inst_offset) : NULL;
		/* Check properties of this interface */
		int result = az_class_lookup_property (if_class, if_impl, if_inst, key, def_class, sub_impl, sub_inst);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		int result = az_class_lookup_property (klass->parent, impl, inst, key, def_class, sub_impl, sub_inst);
		if (result >= 0) return result;
	}
	return -1;
}

#define noVERBOSE

int
az_class_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	int result;
	//arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
#ifdef VERBOSE
	fprintf(stderr, "Lookup function in %s: %s (", (const char *) klass->name, (const char *) key->str);
	for (unsigned int i = 0; i < sig->n_args; i++) {
		AZClass *arg_class = AZ_CLASS_FROM_TYPE(sig->arg_types[i]);
		fprintf(stderr, "%s ", (arg_class) ? (const char *) arg_class->name : "NULL");
	}
	AZClass *ret_class = (sig->ret_type) ? AZ_CLASS_FROM_TYPE(sig->ret_type) : NULL;
	fprintf(stderr, ") -> %s\n", ret_class ? (const char *) ret_class->name : "NULL");
#endif
	for (uint16_t i = 0; i < klass->n_props_self; i++) {
		if (az_string_equals(key, klass->props_self[i].key) && AZ_FIELD_IS_FUNCTION(&klass->props_self[i])) {
			const AZFunctionSignature *prop_sig = klass->props_self[i].signature;
#ifdef VERBOSE
			fprintf(stderr, "    (");
			for (unsigned int j = 0; j < prop_sig->n_args; j++) {
				AZClass *arg_class = AZ_CLASS_FROM_TYPE(prop_sig->arg_types[j]);
				fprintf(stderr, "%s ", arg_class ? (const char *) arg_class->name : "NULL");
			}
			ret_class = (prop_sig->ret_type) ? AZ_CLASS_FROM_TYPE(prop_sig->ret_type) : NULL;
			fprintf(stderr, ") -> %s", ret_class ? (const char *) ret_class->name : "NULL");
#endif
			if (prop_sig && !az_function_signature_is_assignable_to (prop_sig, sig, 0)) {
#ifdef VERBOSE
				fprintf(stderr, " -\n");
#endif
				continue;
			}
#ifdef VERBOSE
			fprintf(stderr, " OK\n");
#endif
			*def_class = klass;
			if (sub_impl) *sub_impl = impl;
			if (sub_inst) *sub_inst = inst;
			return i;
		}
	}
	fprintf (stderr, "    (not found)\n");
	/* interfaces */
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		/* Skip entries whose declaration was rejected during construction */
		if (!ifentry->type) continue;
		AZClass *if_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *if_impl = (impl) ? (AZImplementation *) ((char *) impl + ifentry->impl_offset) : NULL;
		void *if_inst = (inst) ? (void *) ((char *) inst + ifentry->inst_offset) : NULL;
		/* Check properties of this interface */
		result = az_class_lookup_function (if_class, if_impl, if_inst, key, sig, def_class, sub_impl, sub_inst);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_class_lookup_function (klass->parent, impl, inst, key, sig, def_class, sub_impl, sub_inst);
		if (result >= 0) return result;
	}
	return -1;
}
