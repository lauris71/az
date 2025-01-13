#define __AZ_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <az/boxed-interface.h>
#include <az/class.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference-of.h>

#include <az/value.h>

#ifdef AZ_SAFETY_CHECKS
void
az_clear_value (const AZImplementation *impl, AZValue *val)
{
	AZClass *klass;
	arikkei_return_if_fail (impl != 0);
	arikkei_return_if_fail (val != NULL);
	klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_CLASS_IS_REFERENCE) {
		if (val->reference) az_reference_unref ((AZReferenceClass *) impl, val->reference);
	}
}

void
az_transfer_value (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	AZClass *klass;
	arikkei_return_if_fail (impl != 0);
	arikkei_return_if_fail (dst != NULL);
	arikkei_return_if_fail (src != NULL);
	klass = az_type_get_class (impl->type);
	if (klass->value_size) memcpy (dst, src, klass->value_size);
}

void *
az_instance_from_value (const AZImplementation *impl, const AZValue *value)
{
	AZClass *klass;
	arikkei_return_val_if_fail (impl != 0, NULL);
	arikkei_return_val_if_fail (value != NULL, NULL);
	klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_CLASS_IS_BLOCK) {
		return value->block;
	} else {
		return (void *) value;
	}
}
#endif

unsigned int
az_value_equals (const AZImplementation *impl, const AZValue *lhs, const AZValue *rhs)
{
	AZClass *klass;
	klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_CLASS_IS_BLOCK) {
		return lhs->block == rhs->block;
	}
	if ((klass->flags & AZ_CLASS_IS_VALUE) && klass->value_size) {
		return !memcmp (lhs, rhs, klass->value_size);
	}
	return 0;
}

unsigned int
az_value_equals_instance (const AZImplementation *impl, const AZValue *lhs, const void *rhs)
{
	AZClass *klass;
	klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_CLASS_IS_BLOCK) {
		return lhs->block == rhs;
	}
	if ((klass->flags & AZ_CLASS_IS_VALUE) && klass->value_size) {
		return !memcmp (lhs, rhs, klass->value_size);
	}
	return 0;
}


void
az_copy_value (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != 0);
	arikkei_return_if_fail (dst != NULL);
	arikkei_return_if_fail (src != NULL);
#endif
	klass = az_type_get_class (impl->type);
	if (klass->value_size) memcpy (dst, src, klass->value_size);
	if (klass->flags & AZ_CLASS_IS_REFERENCE) {
		if (src->reference) az_reference_ref (src->reference);
	}
}

void
az_set_value_from_instance (const AZImplementation *impl, AZValue *dst, void *inst)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != 0);
	arikkei_return_if_fail (dst != NULL);
	arikkei_return_if_fail (inst != NULL);
#endif
	klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_CLASS_IS_BLOCK) {
		dst->block = inst;
		if (klass->flags & AZ_CLASS_IS_REFERENCE) {
			if (inst) az_reference_ref (dst->reference);
		}
	} else {
		if (klass->value_size) memcpy (dst, inst, klass->value_size);
	}
}

unsigned int
az_value_convert_auto (const AZImplementation **dst_impl, AZValue *dst_val, const AZImplementation **src_impl, const AZValue *src_val, unsigned int to_type)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	AZClass *to_klass = az_type_get_class (to_type);
	/* None can be converted to null reference */
	if (!src_impl) {
		if ((to_type == AZ_TYPE_ANY) || (to_klass->flags & AZ_CLASS_IS_BLOCK)) {
			*dst_impl = NULL;
			dst_val->reference = NULL;
			return 1;
		} else {
			return 0;
		}
	}
	/* Anything can be converted to the same or supertype (this includes Any) */
	if (az_type_is_a ((*src_impl)->type, to_type)) {
		*dst_impl = *src_impl;
		az_copy_value (*src_impl, dst_val, src_val);
		return 1;
	}
	/* Anything can be converted to implemented interface */
	if ((to_klass->flags & AZ_CLASS_IS_INTERFACE) && az_type_implements ((*src_impl)->type, to_type)) {
		dst_val->reference = (AZReference *) az_boxed_interface_new_from_impl_value (*src_impl, src_val, to_type);
		*dst_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_INTERFACE);
		return 1;
	}
	/* Arithemtic types */
	if (AZ_TYPE_IS_ARITHMETIC (to_type) && AZ_TYPE_IS_ARITHMETIC ((*src_impl)->type)) {
		if (az_primitive_can_convert (to_type, (*src_impl)->type) <= AZ_CONVERT_CONDITIONAL) {
			*dst_impl = &to_klass->implementation;
			az_convert_arithmetic_type (to_type, dst_val, (*src_impl)->type, src_val);
			return 1;
		} else {
			return 0;
		}
	}
	/* Nothing else can be converted */
	return 0;
}

unsigned int
az_value_convert_in_place (const AZImplementation **impl, AZValue *val, unsigned int to_type)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	AZClass* to_klass = az_type_get_class (to_type);
	/* None can be converted to null reference */
	if (!*impl) {
		if ((to_type == AZ_TYPE_ANY) || (to_klass->flags & AZ_CLASS_IS_BLOCK)) {
			*impl = NULL;
			val->reference = NULL;
			return 1;
		}
		return 0;
	}
	/* Anything can be converted to supertype (this includes Any) */
	if (az_type_is_a ((*impl)->type, to_type)) {
		return 1;
	}
	/* Anything can be converted to implemented interface */
	if ((to_klass->flags & AZ_CLASS_IS_INTERFACE) && az_type_implements ((*impl)->type, to_type)) {
		val->reference = (AZReference *) az_boxed_interface_new_from_impl_value (*impl, val, to_type);
		*impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_INTERFACE);
		return 1;
	}
	/* Arithmetic types */
	if (AZ_TYPE_IS_ARITHMETIC (to_type) && AZ_TYPE_IS_ARITHMETIC ((*impl)->type)) {
		if (az_primitive_can_convert (to_type, (*impl)->type) <= AZ_CONVERT_CONDITIONAL) {
			az_convert_arithmetic_type (to_type, val, (*impl)->type, val);
			*impl = &to_klass->implementation;
			return 1;
		}
		return 0;
	}
	/* Nothing else can be converted */
	return 0;
}

AZImplementation *
az_value_box (AZValue *dst_val, const AZImplementation *src_impl, const AZValue *src_val)
{
	dst_val->reference = (AZReference *) az_reference_of_new_value (src_impl->type, src_val);
	return (AZImplementation *) az_type_get_class (AZ_TYPE_REFERENCE_OF (src_impl->type));
}

AZImplementation *
az_value_debox (AZValue *dst_val, const AZImplementation *src_impl, const AZValue *src_val)
{
	AZImplementation *dst_impl = (AZImplementation *) az_type_get_class (((AZReferenceOfClass *) src_impl)->instance_type);
	void *src_inst = az_reference_of_get_instance ((AZReferenceOfClass *) src_impl, (AZReferenceOf *) src_val->reference);
	az_copy_value (dst_impl, dst_val, src_inst);
	return dst_impl;
}
