#define __AZ_PACKED_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2018
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <az/class.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference.h>
#include <az/string.h>

#include <az/packed-value.h>

static AZClass *value_class = NULL;

void
az_init_packed_value_class (void)
{
	value_class = az_class_new_with_type (AZ_TYPE_PACKED_VALUE, AZ_TYPE_BLOCK, sizeof (AZClass), sizeof (AZPackedValue), AZ_FLAG_FINAL | AZ_FLAG_ZERO_MEMORY, (const uint8_t *) "value");
	value_class->alignment = 15;
}

void
az_packed_value_set_reference (AZPackedValue *val, unsigned int type, AZReference *ref)
{
	if (val->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(val)) && val->v.reference) {
		az_reference_unref ((AZReferenceClass *) val->impl, val->v.reference);
	}
	val->impl = AZ_IMPL_FROM_TYPE(type);
	val->v.reference = ref;
	if (ref) az_reference_ref (ref);
}

void
az_packed_value_copy_indirect (AZPackedValue *dst, const AZPackedValue *src)
{
	if (dst == src) return;
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	*dst = *src;
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_ref (dst->v.reference);
	}
}

#define AZ_VALUE_IS_NULL(val) ((AZ_IMPL_TYPE((val)->impl) == AZ_TYPE_STRUCT) && ((val)->v.block == NULL))

unsigned int
az_packed_value_can_convert (unsigned int to_type, const AZPackedValue *from)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	/* None can be converted to NULL block */
	if (!from->impl) {
		AZClass *klass = az_type_get_class (to_type);
		return (klass->flags & AZ_FLAG_REFERENCE) != 0;
	}
	/* Anything can be converted to supertype */
	if (az_type_is_a(AZ_PACKED_VALUE_TYPE(from), to_type)) return 1;
	/* Primitive table */
	if ((to_type <= AZ_TYPE_POINTER) && (AZ_PACKED_VALUE_TYPE(from) <= AZ_TYPE_POINTER)) {
		return az_primitive_can_convert (to_type, AZ_PACKED_VALUE_TYPE(from)) <= AZ_CONVERT_CONDITIONAL;
	}
	/* No other automatic conversions possible */
	/* fixme: Sort out NULL pointer */
	if (AZ_VALUE_IS_NULL(from)) {
		return 1;
	}
	return 0;
}

unsigned int
az_packed_value_convert (AZPackedValue *dst, unsigned int to_type, const AZPackedValue *from)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	/* None can be converted to NULL reference */
	if (!from->impl) {
		AZClass *klass = az_type_get_class (to_type);
		if (klass->flags & AZ_FLAG_REFERENCE) {
			az_packed_value_clear (dst);
			return 1;
		}
		return 0;
	}
	/* Anything can be converted to supertype (this includes Any) */
	if (az_type_is_a (AZ_PACKED_VALUE_TYPE(from), to_type)) {
		az_packed_value_copy (dst, from);
		return 1;
	}
	/* Booleans cannot be converted automatically */
	if (to_type == AZ_TYPE_BOOLEAN) {
		return 0;
	}
	if (AZ_PACKED_VALUE_TYPE(from) == AZ_TYPE_BOOLEAN) {
		return 0;
	}
	/* Arithemtic types */
	if (AZ_TYPE_IS_ARITHMETIC (to_type) && AZ_TYPE_IS_ARITHMETIC(AZ_PACKED_VALUE_TYPE(from))) {
		if (az_primitive_can_convert (to_type, AZ_PACKED_VALUE_TYPE(from)) <= AZ_CONVERT_CONDITIONAL) {
			if (dst != from) az_packed_value_clear (dst);
			az_convert_arithmetic_type (to_type, &dst->v, AZ_PACKED_VALUE_TYPE(from), &from->v);
			dst->impl = AZ_IMPL_FROM_TYPE(to_type);
			return 1;
		}
		return 0;
	}
	/* fixme: Sort out NULL pointer */
	if (AZ_VALUE_IS_NULL(from)) {
		az_packed_value_clear (dst);
		//dst->impl = &az_classes[to_type]->implementation;
		return 1;
	}
	return 0;
}

void *
az_packed_value_get_instance (AZPackedValue *value)
{
	if (!value->impl) return NULL;
	if (az_type_is_a (AZ_PACKED_VALUE_TYPE(value), AZ_TYPE_BLOCK)) return value->v.block;
	return &value->v;
}

void
az_packed_value_set_from_type_value (AZPackedValue *dst, unsigned int type, const void *src)
{
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (!type) {
		dst->impl = NULL;
	} else {
		AZClass *klass = AZ_CLASS_FROM_TYPE(type);
		if (az_class_value_size(klass) > AZ_PACKED_VALUE_MAX_SIZE) {
			fprintf (stderr, "az_packed_value_set_from_type_value: value size too big (%u)\n", az_class_value_size(klass));
			dst->impl = NULL;
			return;
		}
		dst->impl = &klass->implementation;
		if (az_class_value_size(klass)) {
			memcpy (&dst->v, src, az_class_value_size(klass));
		}
		if ((klass->flags & AZ_FLAG_REFERENCE) && dst->v.reference) {
			az_reference_ref (dst->v.reference);
		}
	}
}

void
az_packed_value_set_from_impl_value (AZPackedValue *dst, const AZImplementation *impl, const void *src)
{
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (!impl) {
		dst->impl = NULL;
	} else {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (az_class_value_size(klass) > AZ_PACKED_VALUE_MAX_SIZE) {
			fprintf (stderr, "az_packed_value_set_from_type_value: value size too big (%u)\n", az_class_value_size(klass));
			dst->impl = NULL;
			return;
		}
		dst->impl = impl;
		if (az_class_value_size(klass)) {
			memcpy (&dst->v, src, az_class_value_size(klass));
		}
		if ((klass->flags & AZ_FLAG_REFERENCE) && dst->v.reference) {
			az_reference_ref (dst->v.reference);
		}
	}
}

void
az_packed_value_set_from_type_instance (AZPackedValue *dst, unsigned int type, void *inst)
{
	if (dst->impl && AZ_PACKED_VALUE_TYPE(dst) >= AZ_TYPE_REFERENCE) az_packed_value_clear (dst);
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (!type) {
		dst->impl = NULL;
	} else {
		AZClass *klass = AZ_CLASS_FROM_TYPE(type);
		if (az_class_value_size(klass) > AZ_PACKED_VALUE_MAX_SIZE) {
			fprintf (stderr, "az_packed_value_set_from_type_value: value size too big (%u)\n", az_class_value_size(klass));
			dst->impl = NULL;
			return;
		}
		dst->impl = &klass->implementation;
		if (klass->flags & AZ_FLAG_VALUE) {
			if (az_class_value_size(klass)) {
				memcpy (&dst->v, inst, az_class_value_size(klass));
			}
		} else {
			dst->v.block = inst;
			if ((klass->flags & AZ_FLAG_REFERENCE) && dst->v.reference) {
				az_reference_ref (dst->v.reference);
			}
		}
	}
}

void
az_packed_value_set_from_impl_instance (AZPackedValue *dst, const AZImplementation *impl, void *inst)
{
	if (dst->impl && AZ_PACKED_VALUE_TYPE(dst) >= AZ_TYPE_REFERENCE) az_packed_value_clear (dst);
	if (dst->impl && AZ_TYPE_IS_REFERENCE(AZ_PACKED_VALUE_TYPE(dst)) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (!impl) {
		dst->impl = NULL;
	} else {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (az_class_value_size(klass) > AZ_PACKED_VALUE_MAX_SIZE) {
			fprintf (stderr, "az_packed_value_set_from_type_value: value size too big (%u)\n", az_class_value_size(klass));
			dst->impl = NULL;
			return;
		}
		dst->impl = impl;
		if (klass->flags & AZ_FLAG_VALUE) {
			if (az_class_value_size(klass)) {
				memcpy (&dst->v, inst, az_class_value_size(klass));
			}
		} else {
			dst->v.block = inst;
			if ((klass->flags & AZ_FLAG_REFERENCE) && dst->v.reference) {
				az_reference_ref (dst->v.reference);
			}
		}
	}
}
