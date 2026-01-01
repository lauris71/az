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

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/extend.h>
#ifdef AZ_HAS_PROPERTIES
#include <az/function-value.h>
#include <az/packed-value.h>
#include <az/private.h>
#endif
#include <az/string.h>

static void az_class_pre_init (AZClass *klass, unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name);

static unsigned char zero_val[16] = { 0 };

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
	const AZClass *sub_class;
	const AZImplementation *prop_impl;
	void *prop_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &sub_class, &prop_impl, &prop_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &sub_class->props_self[prop_idx];
	arikkei_return_val_if_fail (prop->spec == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (!prop->is_final, 0);
	arikkei_return_val_if_fail (prop->write != AZ_FIELD_WRITE_NONE, 0);
	az_instance_set_property_by_id (sub_class, prop_impl, NULL, prop_idx, arg_impls[2], az_value_get_inst(arg_impls[2], arg_vals[2]), ctx);
	return 1;
}

static unsigned int
impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *sub_class;
	const AZImplementation *prop_impl;
	void *prop_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &sub_class, &prop_impl, &prop_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &sub_class->props_self[prop_idx];
	arikkei_return_val_if_fail (prop->spec == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (prop->read != AZ_FIELD_READ_NONE, 0);
	az_instance_get_property_by_id (sub_class, prop_impl, NULL, prop_idx, ret_impl, ret_val, 64, NULL);
	return 1;
}

void
az_class_class_post_init (void)
{
	az_class_set_num_properties (&AZClassKlass, 1);
	az_class_define_property (&AZClassKlass, 0, (const unsigned char *) "parent", AZ_TYPE_CLASS, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZClass, parent), NULL, NULL);
}

AZClass *
az_class_new (uint32_t *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail ((parent_type == AZ_TYPE_NONE) || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size), 0);
	arikkei_return_val_if_fail ((parent_type == AZ_TYPE_NONE) || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size), 0);
#endif
	arikkei_return_val_if_fail (!AZ_TYPE_IS_FINAL(parent_type), 0);
	unsigned int idx = az_reserve_type();

	AZClass *klass = az_class_new_with_type(idx, parent_type, class_size, instance_size, flags, name);
	klass->instance_init = instance_init;
	klass->instance_finalize = instance_finalize;
	*type = klass->impl.type;
	return klass;
}

AZClass *
az_class_new_with_type (unsigned int type, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name)
{
	AZClass *klass = (AZClass *) malloc (class_size);
	az_class_pre_init (klass, type, parent_type, class_size, instance_size, flags, name);
	az_types[AZ_TYPE_INDEX(type)].klass = klass;
	/* We have to use class flags here because of parent chaining */
	az_types[AZ_TYPE_INDEX(type)].flags = klass->impl.flags;
	az_types[AZ_TYPE_INDEX(type)].pidx = parent_type;
	return klass;
}

void
az_class_new_with_value (AZClass *klass)
{
	unsigned int idx = AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass));
	az_types[idx].klass = klass;
	/* We have to use class flags here because of parent chaining */
	az_types[idx].flags = klass->impl.flags;
	az_types[idx].pidx = klass->parent ? AZ_CLASS_TYPE(klass->parent) : AZ_TYPE_NONE;
}

static void
az_class_pre_init (AZClass *klass, unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name)
{
	memset (klass, 0, class_size);
	if (parent) {
		AZClass *parent_class = AZ_CLASS_FROM_TYPE(parent);
		memcpy (klass, parent_class, parent_class->class_size);
		/* Overwrite values from supertype */
		klass->impl.flags &= ~AZ_FLAG_ABSTRACT;
		klass->parent = parent_class;
		klass->n_ifaces_self = 0;
#ifdef AZ_HAS_PROPERTIES
		klass->n_props_self = 0;
		klass->props_self = NULL;
#endif
	}
	klass->impl.flags |= flags;
	klass->impl.type = type;
	klass->name = name;
	klass->class_size = class_size;
	klass->instance_size = instance_size;
}

void
az_class_set_num_interfaces (AZClass *klass, unsigned int n_ifaces)
{
	klass->n_ifaces_self = n_ifaces;
	if (n_ifaces > 2) {
		static unsigned int n_allocations = 0, allocated = 0;
		klass->ifaces_self = (AZIFEntry *) malloc(n_ifaces * sizeof(AZIFEntry));
		n_allocations += 1;
		allocated += n_ifaces;
		fprintf(stderr, "az_class_set_num_interfaces(): Allocated %u (%u %u)\n", n_ifaces, n_allocations, allocated);
#ifdef AZ_SAFETY_CHECKS
        memset (klass->ifaces_self, 0, n_ifaces * sizeof (AZIFEntry));
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
	AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[idx] : &klass->ifaces_self[idx];
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!ifentry->type);
#endif
	*ifentry = (AZIFEntry) {type, impl_offset, inst_offset};
	/* if class is interface, sub-implementations are defined in it's standalone implementations instead */
	if (!AZ_CLASS_IS_INTERFACE(klass)) {
		/* Init implementation */
		AZClass *iface_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		az_implementation_init_by_type ((AZImplementation *) ((char *) klass + ifentry->impl_offset), ifentry->type);
	}
}

void
az_class_post_init (AZClass *klass)
{
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!(klass->alignment & (klass->alignment + 1)));
	for (i = 0; i < klass->n_ifaces_self; i++) {
		AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[i] : &klass->ifaces_self[i];
		if (!ifentry->type) {
			fprintf (stderr, "az_class_post_init: Klass %s interface %u is not defined\n", klass->name, i);
		}
	}
#ifdef AZ_HAS_PROPERTIES
	for (i = 0; i < klass->n_props_self; i++) {
		if (!klass->props_self[i].key) {
			fprintf (stderr, "az_class_post_init: Klass %s property %u is not defined\n", klass->name, i);
		}
	}
#endif
#endif
	if (klass->n_ifaces_self || klass->instance_init || klass->instance_finalize) {
		klass->impl.flags |= AZ_FLAG_CONSTRUCT;
		az_types[AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass))].flags |= AZ_FLAG_CONSTRUCT;
	}
	if (klass->n_ifaces_self) {
		/* Count all interfaces */
		/* Initially n_ifaces_all has the value from parent class */
		for (i = 0; i < klass->n_ifaces_self; i++) {
			AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[i] : &klass->ifaces_self[i];
			AZClass *iface_class = AZ_CLASS_FROM_TYPE(ifentry->type);
			klass->n_ifaces_all += (1 + iface_class->n_ifaces_all);
		}
		/*
		* n_ifaces_self == n_ifaces_all:
		*   n_ifaces_self <= 2 : self, all = ifaces[0..1]
		*   n_ifaces_self > 2 : self, all = ifaces_self
		* n_ifaces_self < n_ifaces_all:
		* 	 n_ifaces_self == 0:
		*     n_ifaces_all <= 2 : all = ifaces[0..1]
		*     n_ifaces_all > 2 : all = ifaces_all
		*   n_ifaces_self == 1:
		*     n_ifaces_all == 2 : self, all = ifaces[0..1]
		*     n_ifaces_all > 2 : self = ifaces[0], all = ifaces_all
		*   n_ifaces_self > 1 : self = ifaces_self, all = ifaces_all
		*/
		if (klass->n_ifaces_self == klass->n_ifaces_all) {
			/* Share interface definitions */
			if (klass->n_ifaces_self > 2) {
				klass->ifaces_all = klass->ifaces_self;
			}
		} else {
			if (klass->n_ifaces_self == 2) {
				/* Have to move self interfaces */
				static unsigned int n_allocations = 0, allocated = 0;
				AZIFEntry *ifaces = (AZIFEntry *) malloc(klass->n_ifaces_self * sizeof(AZIFEntry));
				n_allocations += 1;
				allocated += klass->n_ifaces_self;
				fprintf(stderr, "az_class_post_init(): Allocated self %u (%u %u)\n", klass->n_ifaces_self, n_allocations, allocated);
				memcpy(ifaces, klass->ifaces, 2 * sizeof(AZIFEntry));
				klass->ifaces_self = ifaces;
			}
			AZIFEntry *ifaces;
			if ((klass->n_ifaces_self == 1) && (klass->n_ifaces_all <= 2)) {
				ifaces = klass->ifaces;
			} else {
				static unsigned int n_allocations = 0, allocated = 0;
				klass->ifaces_all = (AZIFEntry *) malloc(klass->n_ifaces_all * sizeof(AZIFEntry));
				n_allocations += 1;
				allocated += klass->n_ifaces_all;
				fprintf(stderr, "az_class_post_init(): Allocated all %u (%u %u)\n", klass->n_ifaces_all, n_allocations, allocated);
				ifaces = klass->ifaces_all;
			}
			unsigned int idx = 0;
			for (i = 0; i < klass->n_ifaces_self; i++) {
				ifaces[idx] = *az_class_iface_self(klass, i);
				AZClass *iface_class = AZ_CLASS_FROM_TYPE(az_class_iface_self(klass, i)->type);
				idx += 1;
				memcpy(&ifaces[idx], az_class_iface_all(iface_class, 0), iface_class->n_ifaces_all * sizeof (AZIFEntry *));
				idx += iface_class->n_ifaces_all;
			}
			memcpy (&ifaces[idx], az_class_iface_all(klass->parent, 0), klass->parent->n_ifaces_all * sizeof (AZIFEntry *));
		}
	}
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
}

#ifdef AZ_HAS_PROPERTIES
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
	if ((read == AZ_FIELD_READ_VALUE) || (read == AZ_FIELD_READ_PACKED)) {
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
		(AZImplementation *) az_type_get_class (AZ_TYPE_FUNCTION_VALUE), &fval);
}

void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail (n_args < 64);

	//uint64_t *p = (uint64_t *) &n_args + 1;

	va_start (ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg (ap, unsigned int);
		//fprintf (stderr, "%u %llu\n", arg_types[i], p[i] & 0xffffffff);
	}
	va_end (ap);
	az_class_define_static_method (klass, idx, key, ret_type, n_args, arg_types, invoke);
}

int
az_class_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst)
{
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (uint16_t i = 0; i < (int) klass->n_props_self; i++) {
		if (az_string_equals(key, klass->props_self[i].key)) {
			*def_class = klass;
			if (def_impl) *def_impl = impl;
			if (def_inst) *def_inst = inst;
			return i;
		}
	}
	/* interfaces */
	for (uint16_t i = 0; i < (int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (impl) ? (AZImplementation *) ((char *) impl + ifentry->impl_offset) : NULL;
		void *sub_inst = (inst) ? (void *) ((char *) inst + ifentry->inst_offset) : NULL;
		/* Check properties of this interface */
		int result = az_class_lookup_property (sub_class, sub_impl, sub_inst, key, def_class, def_impl, def_inst);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		int result = az_class_lookup_property (klass->parent, impl, inst, key, def_class, def_impl, def_inst);
		if (result >= 0) return result;
	}
	return -1;
}

int
az_class_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst)
{
	int result, i;
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (i = 0; i < (int) klass->n_props_self; i++) {
		if (!strcmp ((const char *) key, (const char *) klass->props_self[i].key->str) && klass->props_self->is_function) {
			if (klass->props_self[i].signature && !az_function_signature_is_assignable_to (klass->props_self[i].signature, sig, 0)) {
				continue;
			}
			*def_class = klass;
			if (def_impl) *def_impl = impl;
			if (def_inst) *def_inst = inst;
			return i;
		}
	}
	/* interfaces */
	for (i = 0; i < ( int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (impl) ? (AZImplementation *) ((char *) impl + ifentry->impl_offset) : NULL;
		void *sub_inst = (inst) ? (void *) ((char *) inst + ifentry->inst_offset) : NULL;
		/* Check properties of this interface */
		result = az_class_lookup_function (sub_class, sub_impl, sub_inst, key, sig, def_class, def_impl, def_inst);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_class_lookup_function (klass->parent, impl, inst, key, sig, def_class, def_impl, def_inst);
		if (result >= 0) return result;
	}
	return -1;
}

#endif
