#define __AZ_TYPES_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/boxed-value.h>
#include <az/class.h>
#include <az/function.h>
#include <az/interface.h>
#include <az/object.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference.h>
#include <az/string.h>
#include <az/boxed-interface.h>
#ifdef AZ_HAS_PACKED_VALUE
#include <az/packed-value.h>
#endif
#ifdef AZ_HAS_PROPERTIES
#include <az/field.h>
#endif

#include <az/types.h>

void
az_init (void)
{
	static unsigned int initialized = 0;
	if (initialized) return;
	initialized = 1;
	az_num_types = AZ_NUM_BASE_TYPES;
	az_globals_init();
	az_init_primitive_classes();
	az_init_base_classes();

	az_init_interface_class();
#ifdef AZ_HAS_PROPERTIES
	az_init_field_class();
#endif
	az_init_function_classes();
	az_init_reference_class();
	az_init_string_class();
	az_init_boxed_value_class();
	az_init_boxed_interface_class();
#ifdef AZ_HAS_PACKED_VALUE
	az_init_packed_value_class();
#endif
	az_init_object_class();

	az_post_init_primitive_classes();
}

unsigned int
az_type_get_parent_primitive (unsigned int type)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
#endif
	if (AZ_TYPE_INDEX(type) < AZ_NUM_FUNDAMENTAL_TYPES) return type;
	klass = AZ_CLASS_FROM_TYPE(type)->parent;
	while (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass)) >= AZ_NUM_FUNDAMENTAL_TYPES) {
		klass = klass->parent;
	}
	return AZ_CLASS_TYPE(klass);
}

unsigned int
az_type_is_a (unsigned int type, unsigned int test)
{
	AZClass *klass;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
	arikkei_return_val_if_fail (test < az_num_types, 0);
#endif
	if (!type) return 0;
	if (type == test) return 1;
	test = AZ_TYPE_INDEX(test);
	uint32_t idx = az_types[AZ_TYPE_INDEX(type)].pidx;
	while (idx) {
		if (idx == test) return 1;
		idx = az_types[idx].pidx;
	}
	return 0;
}

unsigned int
az_type_implements (unsigned int type, unsigned int test)
{
	if (!type) return 0;
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
	arikkei_return_val_if_fail (test < az_num_types, 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE(test), 0);
#endif
	if (!type) return 0;
	return az_get_interface (&AZ_CLASS_FROM_TYPE(type)->impl, NULL, test, NULL) != NULL;
}

unsigned int
az_type_is_assignable_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) != 0, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) < az_num_types, 0);
#endif
	if (!type) {
		/* None can be assigned to any */
		if (test == AZ_TYPE_ANY) return 1;
		/* None can be assigned to blocks */
		if (AZ_TYPE_IS_BLOCK(test)) return 1;
	}
	if (az_type_is_a (type, test)) return 1;
	if (AZ_TYPE_IS_INTERFACE(test)) {
		return az_type_implements (type, test);
	}
	return 0;
}

const AZImplementation *
az_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	AZImplementation *sub_impl;
	unsigned int i;
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (AZ_IMPL_TYPE(impl) == AZ_TYPE_BOXED_INTERFACE) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	if (az_type_is_a (AZ_IMPL_TYPE(impl), if_type)) {
		if (if_inst) *if_inst = inst;
		return impl;
	}
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	const AZIFEntry *ifentry = az_class_iface_all(klass, 0);
	for (i = 0; i < klass->n_ifaces_all; i++) {
		if (az_type_is_a(ifentry->type, if_type)) {
			sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
			if (if_inst) *if_inst = (char *) inst + ifentry->inst_offset;
			return sub_impl;
		}
		ifentry += 1;
	}
	if (if_inst) *if_inst = NULL;
	return NULL;
}

unsigned int
az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return (klass->serialize) ? klass->serialize (impl, inst, d, dlen, ctx) : 0;
}

unsigned int
az_deserialize_value (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (value != NULL, 0);
#endif
	klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return (klass->deserialize) ? klass->deserialize (impl, value, s, slen, ctx) : 0;
}

/* fixme: Make signature correct */

unsigned int
az_instance_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZClass* klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return klass->to_string (&klass->impl, inst, buf, len);
}

AZClass *
az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
#endif
	AZClass *klass = az_class_new (type, name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	arikkei_return_val_if_fail (*type, NULL);
	if (class_init) class_init (klass);
	az_class_post_init (klass);
	return klass;
}

AZClass *
az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
#endif
	AZClass *klass = az_class_new (type, name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	arikkei_return_val_if_fail (*type, NULL);
	if (class_init) class_init (klass, data);
	az_class_post_init (klass);
	return klass;
}

#ifdef AZ_HAS_PROPERTIES
int
az_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop)
{
	int result, i;
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (i = 0; i < (int) klass->n_props_self; i++) {
		if (az_string_equals(key, klass->props_self[i].key)) {
			*def_class = klass;
			*def_impl = impl;
			if (def_inst) *def_inst = inst;
			if (prop) *prop = &klass->props_self[i];
			return i;
		}
	}
	/* interfaces */
	for (i = 0; i < (int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		/* Check properties of this interface */
		result = az_lookup_property (AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, sub_inst, key, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_lookup_property (klass->parent, impl, inst, key, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	return -1;
}

int
az_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop)
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
			*def_impl = impl;
			if (def_inst) *def_inst = inst;
			if (prop) *prop = &klass->props_self[i];
			return i;
		}
	}
	/* interfaces */
	for (i = 0; i < ( int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZImplementation *c_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *c_inst = (void *) ((char *) inst + ifentry->inst_offset);
		/* Check properties of this interface */
		result = az_lookup_function (AZ_CLASS_FROM_IMPL(c_impl), c_impl, c_inst, key, sig, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_lookup_function (klass->parent, impl, inst, key, sig, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	return -1;
}

unsigned int
az_instance_set_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	idx = az_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst, NULL);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_set_property_by_id (sub_class, sub_impl, sub_inst, idx, prop_impl, prop_inst, ctx);
}

unsigned int
az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (!klass->props_self[idx].is_final, 0);
	arikkei_return_val_if_fail (klass->props_self[idx].write != AZ_FIELD_WRITE_NONE, 0);
	if (klass->props_self[idx].is_interface) {
		if (prop_impl && !az_type_implements(AZ_IMPL_TYPE(prop_impl), klass->props_self[idx].type)) {
			fprintf (stderr, ".");
		}
		arikkei_return_val_if_fail (!prop_impl || az_type_implements(AZ_IMPL_TYPE(prop_impl), klass->props_self[idx].type), 0);
		if (klass->props_self[idx].is_function && prop_impl) {
			AZFunctionInstance *func_inst;
			const AZFunctionImplementation *func_impl = (const AZFunctionImplementation *) az_get_interface (prop_impl, prop_inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
			const AZFunctionSignature *sig = az_function_get_signature (func_impl, func_inst);
			if (klass->props_self[idx].signature && !az_function_signature_is_assignable_to (sig, klass->props_self[idx].signature, 1)) {
				fprintf (stderr, ".");
			}
			arikkei_return_val_if_fail (!klass->props_self[idx].signature || az_function_signature_is_assignable_to (sig, klass->props_self[idx].signature, 1), 0);
		}
	} else {
		arikkei_return_val_if_fail (!prop_impl || az_type_is_a(AZ_IMPL_TYPE(prop_impl), klass->props_self[idx].type), 0);
	}
	if (klass->props_self[idx].write == AZ_FIELD_WRITE_VALUE) {
		AZValue *val;
		if (klass->props_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZValue *) ((char *) inst + klass->props_self[idx].offset);
		} else if (klass->props_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZValue *) ((char *) impl + klass->props_self[idx].offset);
		} else {
			val = (AZValue *) ((char *) klass + klass->props_self[idx].offset);
		}
		az_value_set_from_inst (prop_impl, val, prop_inst);
	} else if (klass->props_self[idx].write == AZ_FIELD_WRITE_PACKED) {
		AZPackedValue *val;
		if (klass->props_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZPackedValue *) ((char *) inst + klass->props_self[idx].offset);
		} else if (klass->props_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZPackedValue *) ((char *) impl + klass->props_self[idx].offset);
		} else {
			val = (AZPackedValue *) ((char *) klass + klass->props_self[idx].offset);
		}
		az_packed_value_set_from_impl_instance (val, prop_impl, prop_inst);
	} else if (klass->props_self[idx].write == AZ_FIELD_WRITE_METHOD) {
		return klass->set_property (impl, inst, idx, prop_impl, prop_inst, NULL);
	}
	return 1;
}

// fixme: Tis is wrong as inst can be NULL
unsigned int
az_instance_get_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	idx = az_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst, NULL);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (sub_class, sub_impl, sub_inst, idx, dst_impl, dst_val, 64, NULL);
}

unsigned int
az_instance_get_function (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	idx = az_lookup_function (AZ_CLASS_FROM_IMPL(impl), impl, inst, key, sig, &sub_class, &sub_impl, &sub_inst, NULL);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (sub_class, sub_impl, sub_inst, idx, dst_impl, (AZValue64 *) dst_val, 16, NULL);
}

unsigned int
az_instance_get_property_by_id (const AZClass *def_klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **val_impl, AZValue64 *val, unsigned int val_size, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (val_impl != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
	arikkei_return_val_if_fail (def_klass->props_self[idx].read != AZ_FIELD_READ_NONE, 0);

	const AZField *prop = &def_klass->props_self[idx];

	switch(prop->read) {
		case AZ_FIELD_READ_VALUE: {
			/* Bare value inside instance/implementation/class */
			AZValue *src;
			if (prop->spec == AZ_FIELD_INSTANCE) {
				src = (AZValue *) ((char *) inst + prop->offset);
			} else if (prop->spec == AZ_FIELD_IMPLEMENTATION) {
				src = (AZValue *) ((char *) impl + prop->offset);
			} else {
				AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
				src = (AZValue *) ((char *) klass + prop->offset);
			}
			if (prop->mask) {
				if (prop->type == AZ_TYPE_BOOLEAN) {
					uint32_t v = ((src->uint32_v & prop->mask) >> prop->shift) ^ prop->bits;
					*val_impl = &AZBooleanKlass.impl;
					val->value.boolean_v = (v != 0);
					return 1;
				} else {
					// fixme: handle integral types
					return 0;
				}
			}
			if (az_type_is_a (prop->type, AZ_TYPE_OBJECT)) {
				az_value_set_object (val_impl, &val->value, (AZObject *) src->reference);
			} else {
				AZClass *prop_class = AZ_CLASS_FROM_TYPE(prop->type);
				arikkei_return_val_if_fail (prop_class->impl.flags & AZ_FLAG_FINAL, 0);
				*val_impl = az_value_copy_autobox (&prop_class->impl, &val->value, src, val_size);
			}
			break;
		}
		case AZ_FIELD_READ_PACKED: {
			/* Packed value inside instance */
			AZPackedValue *src;
			if (prop->spec == AZ_FIELD_INSTANCE) {
				src = (AZPackedValue *) ((char *) inst + prop->offset);
			} else if (prop->spec == AZ_FIELD_IMPLEMENTATION) {
				src = (AZPackedValue *) ((char *) impl + prop->offset);
			} else {
				AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
				src = (AZPackedValue *) ((char *) klass + prop->offset);
			}
			*val_impl = az_value_copy_autobox (src->impl, &val->value, &src->v, val_size);
			break;
		}
		case AZ_FIELD_READ_STORED_STATIC:
			/* Packed value inside field definition */
			if (prop->value->impl) {
				*val_impl = az_value_copy_autobox (prop->value->impl, &val->value, &prop->value->v, val_size);
			} else {
				*val_impl = NULL;
			}
			break;
		case AZ_FIELD_READ_METHOD:
			return def_klass->get_property (impl, inst, idx, val_impl, &val->value, ctx);
		default:
			/* Not readable */
			return 0;
	}
	return 1;
}
#endif
