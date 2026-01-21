#define __AZ_PACKED_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2018
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference.h>
#include <az/string.h>

#include <az/packed-value.h>

static unsigned int
packed_value_to_string (const AZImplementation *impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZPackedValue *pval = (AZPackedValue *) inst;
	unsigned int pos;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Packed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0,AZ_CLASS_FROM_IMPL(pval->impl)->name);
	if (pos < len) buf[pos] = 0;
	return pos;
}

AZClass AZPackedValueKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_ZERO_MEMORY | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_PACKED_VALUE},
	&AZBlockKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "packed value",
	15, sizeof(AZPackedValue), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, packed_value_to_string,
	NULL, NULL
};

void
az_init_packed_value_class (void)
{
	az_class_new_with_value(&AZPackedValueKlass);
}

void
az_packed_value_set(AZPackedValue *dst, const AZImplementation *impl, void *inst)
{
	if (dst->impl && AZ_IMPL_IS_REFERENCE(dst->impl) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (impl) {
		az_value_set_from_inst(impl, &dst->v, inst);
	}
	dst->impl = impl;
}

void
az_packed_value_set_autobox(AZPackedValue *dst, const AZImplementation *impl, void *inst)
{
	if (dst->impl && AZ_IMPL_IS_REFERENCE(dst->impl) && dst->v.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.reference);
	}
	if (impl) {
		impl = az_value_set_from_inst_autobox(impl, &dst->v, inst, AZ_PACKED_VALUE_MAX_SIZE);
	}
	dst->impl = impl;
}

void
az_packed_value_64_set_autobox(AZPackedValue64 *dst, const AZImplementation *impl, void *inst)
{
	if (dst->impl && AZ_IMPL_IS_REFERENCE(dst->impl) && dst->v.value.reference) {
		az_reference_unref ((AZReferenceClass *) dst->impl, dst->v.value.reference);
	}
	if (impl) {
		impl = az_value_set_from_inst_autobox(impl, &dst->v.value, inst, 64);
	}
	dst->impl = impl;
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

#define AZ_VALUE_IS_NULL(val) ((AZ_IMPL_TYPE((val)->impl) == AZ_TYPE_STRUCT) && ((val)->v.block == NULL))

unsigned int
az_packed_value_can_convert (unsigned int to_type, const AZPackedValue *from)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	/* None can be converted to NULL block */
	if (!from->impl) {
		AZClass *klass = az_type_get_class (to_type);
		return (AZ_CLASS_FLAGS(klass) & AZ_FLAG_REFERENCE) != 0;
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
		if (AZ_CLASS_FLAGS(klass) & AZ_FLAG_REFERENCE) {
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
