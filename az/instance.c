#define __AZ_INSTANCE_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdlib.h>
#include <string.h>

#include <az/base.h>
#include <az/boxed-interface.h>
#include <az/boxed-value.h>
#include <az/class.h>
#include <az/field.h>
#include <az/function.h>
#include <az/instance.h>
#include <az/object.h>
#include <az/string.h>
#include <az/types.h>

 /*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. Initializes parent type
 * 2. Initializes all local interfaces
 * 3. instance_init
 */

void
az_instance_init_recursive (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int zeroed)
{
	/* Fundamental tyles do not have constructors */
	if (klass->parent && (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass->parent)) >= AZ_NUM_FUNDAMENTAL_TYPES)) {
		az_instance_init_recursive (klass->parent, impl, inst, zeroed);
	}
	/* Interfaces */
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		if (!zeroed && (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY)) memset (sub_inst, 0, sub_class->instance_size);
		az_instance_init_recursive (sub_class, sub_impl, sub_inst, zeroed || (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY));
		ifentry += 1;
	}
	/* Instance itself */
	if (klass->instance_init) klass->instance_init (impl, inst);
}

/*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. instance_finalize
 * 2. Finalizes all local interfaces
 * 3. Finalizes parent type
 */

void
az_instance_finalize_recursive (const AZClass *klass, const AZImplementation *impl, void *inst)
{
	if (klass->instance_finalize) klass->instance_finalize (impl, inst);
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		az_instance_finalize_recursive (sub_class, sub_impl, sub_inst);
		ifentry += 1;
	}
	if (klass->parent && (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass->parent)) >= AZ_NUM_FUNDAMENTAL_TYPES)) {
		az_instance_finalize_recursive (klass->parent, impl, inst);
	}
}

void
az_instance_init (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail(impl != NULL);
	arikkei_return_if_fail(inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	if (klass->impl.flags & AZ_FLAG_CONSTRUCT) az_instance_init_recursive (klass, impl, inst, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
}

void
az_instance_finalize (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	if (klass->impl.flags & AZ_FLAG_CONSTRUCT) az_instance_finalize_recursive (klass, impl, inst);
}

void *
az_instance_new (unsigned int type)
{
	AZClass *klass;
	void *inst;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) != 0, NULL);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = AZ_CLASS_FROM_TYPE(type);
	arikkei_return_val_if_fail (!(klass->impl.flags & AZ_FLAG_ABSTRACT), NULL);
	if (klass->allocator && klass->allocator->allocate) {
		inst = klass->allocator->allocate (klass);
	} else {
		inst = malloc (klass->instance_size);
	}
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	az_instance_init_recursive (klass, &klass->impl, inst, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
	return inst;
}

void *
az_instance_new_array (unsigned int type, unsigned int nelements)
{
	AZClass *klass;
	void *elements;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (type != 0, NULL);
	arikkei_return_val_if_fail (type < az_num_types, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = az_type_get_class (type);
	if (klass->allocator && klass->allocator->allocate_array) {
		elements = klass->allocator->allocate_array (klass, nelements);
	} else {
		elements = malloc (nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	}
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (elements, 0, nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		az_instance_init_recursive (klass, &klass->impl, instance, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
	}
	return elements;
}

void
az_instance_delete (unsigned int type, void *instance)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_types);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	az_instance_finalize_recursive (klass, &klass->impl, instance);
	if (klass->allocator && klass->allocator->free) {
		klass->allocator->free (klass, instance);
	} else {
		free (instance);
	}
}

void
az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements)
{
	AZClass *klass;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_types);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		az_instance_finalize_recursive (klass, &klass->impl, instance);
	}
	if (klass->allocator && klass->allocator->free_array) {
		klass->allocator->free_array (klass, elements, nelements);
	} else {
		free (elements);
	}
}

unsigned int
az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	return (klass->serialize) ? klass->serialize (impl, inst, d, dlen, ctx) : 0;
}

/* fixme: Make signature correct */

unsigned int
az_instance_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int dlen)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
	return klass->to_string (&klass->impl, inst, d, dlen);
}

const AZImplementation *
az_instance_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (impl == AZ_BOXED_INTERFACE_IMPL) {
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
	for (uint16_t i = 0; i < klass->n_ifaces_all; i++) {
		if (az_type_is_a(ifentry->type, if_type)) {
			AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
			if (if_inst) *if_inst = (char *) inst + ifentry->inst_offset;
			return sub_impl;
		}
		ifentry += 1;
	}
	if (if_inst) *if_inst = NULL;
	return NULL;
}

// fixme: This is wrong as inst can be NULL
unsigned int
az_instance_get_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val)
{
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	int idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (sub_class, AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, sub_inst, idx, dst_impl, dst_val, 64, NULL);
}

unsigned int
az_instance_get_function_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val)
{
	const AZClass *def_class;
	const AZImplementation *def_impl;
	void *def_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	int idx = az_class_lookup_function (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, sig, &def_class, &def_impl, &def_inst);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (def_class, AZ_CLASS_FROM_IMPL(def_impl), def_impl, def_inst, idx, dst_impl, (AZValue64 *) dst_val, 16, NULL);
}

unsigned int
az_instance_set_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_set_property_by_id (sub_class, sub_impl, sub_inst, idx, prop_impl, prop_inst, ctx);
}

#ifdef AZ_HAS_PROPERTIES

unsigned int
az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (!klass->props_self[idx].is_final, 0);
	arikkei_return_val_if_fail (klass->props_self[idx].write != AZ_FIELD_WRITE_NONE, 0);
	const AZField *prop = &klass->props_self[idx];
	if (!strcmp((const char *) prop->key->str, "cameraController")) {
		fprintf (stderr, ".");
	}
	if (klass->props_self[idx].is_interface) {
		if (prop_impl && !az_type_implements(AZ_IMPL_TYPE(prop_impl), klass->props_self[idx].type)) {
			fprintf (stderr, ".");
		}
		arikkei_return_val_if_fail (!prop_impl || az_type_implements(AZ_IMPL_TYPE(prop_impl), klass->props_self[idx].type), 0);
		if (klass->props_self[idx].is_function && prop_impl) {
			AZFunctionInstance *func_inst;
			const AZFunctionImplementation *func_impl = (const AZFunctionImplementation *) az_instance_get_interface (prop_impl, prop_inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
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

unsigned int
az_instance_get_property_by_id (const AZClass *def_klass, const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **val_impl, AZValue64 *val, unsigned int val_size, AZContext *ctx)
{
	arikkei_return_val_if_fail (def_klass != NULL, 0);
	arikkei_return_val_if_fail (klass != NULL, 0);
	arikkei_return_val_if_fail (val_impl != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
	arikkei_return_val_if_fail (def_klass->props_self[idx].read != AZ_FIELD_READ_NONE, 0);

	const AZField *prop = &def_klass->props_self[idx];

	switch(prop->read) {
		case AZ_FIELD_READ_VALUE: {
			/* Bare value inside instance/implementation/class */
			AZValue *src;
			if (prop->spec == AZ_FIELD_INSTANCE) {
				arikkei_return_val_if_fail(inst != NULL, 0);
				src = (AZValue *) ((char *) inst + prop->offset);
			} else if (prop->spec == AZ_FIELD_IMPLEMENTATION) {
				arikkei_return_val_if_fail(impl != NULL, 0);
				src = (AZValue *) ((char *) impl + prop->offset);
			} else {
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
			if (AZ_TYPE_IS_OBJECT(prop->type)) {
				az_value_set_object (val_impl, &val->value, (AZObject *) src->reference);
			} else {
				AZClass *prop_class = AZ_CLASS_FROM_TYPE(prop->type);
				if (!AZ_CLASS_IS_FINAL(prop_class)) {
					fprintf(stderr, ".");
				}
				arikkei_return_val_if_fail (AZ_CLASS_IS_FINAL(prop_class), 0);
				*val_impl = az_value_copy_autobox (&prop_class->impl, &val->value, src, val_size);
			}
			break;
		}
		case AZ_FIELD_READ_PACKED: {
			/* Packed value inside instance */
			AZPackedValue *src;
			if (prop->spec == AZ_FIELD_INSTANCE) {
				arikkei_return_val_if_fail(inst != NULL, 0);
				src = (AZPackedValue *) ((char *) inst + prop->offset);
			} else if (prop->spec == AZ_FIELD_IMPLEMENTATION) {
				arikkei_return_val_if_fail(impl != NULL, 0);
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
