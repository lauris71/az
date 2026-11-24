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

static int64_t
packed_value_to_string (const AZImplementation *impl, void *inst, uint8_t *buf, uint64_t len)
{
	AZPackedValue *pval = (AZPackedValue *) inst;
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, (const uint8_t *) "Packed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0,AZ_CLASS_FROM_IMPL(pval->impl)->name);
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

AZ_CLASS_ALIGN AZClass AZPackedValueKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_ZERO_MEMORY | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_PACKED_VALUE },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "packed value",
	.alignment = 15,
	.class_size = sizeof(AZPackedValue),
	.instance_size = 0,
	.to_string = packed_value_to_string
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
		impl = az_value_set_from_inst_autobox(impl, &dst->v, AZ_PACKED_VALUE_MAX_SIZE, inst);
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
		impl = az_value_set_from_inst_autobox(impl, &dst->v.value, 64, inst);
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

unsigned int
az_packed_value_can_convert (const AZPackedValue *from, unsigned int to_type, AZConversionType conversion_type)
{
	/* Untyped null has no implementation - the type table handles it as NONE */
	unsigned int from_type = (from->impl) ? AZ_PACKED_VALUE_TYPE (from) : AZ_TYPE_NONE;
	return az_type_get_conversion_to (from_type, to_type) <= conversion_type;
}

AZConversionResult
az_packed_value_convert (AZPackedValue *dst, const AZPackedValue *from, unsigned int to_type, AZConversionType conversion_type)
{
	if (dst == from) {
		/* In-place conversion consumes the old content */
		return az_value_convert_in_place_autobox (&dst->impl, &dst->v, sizeof (dst->v), to_type, conversion_type);
	}
	/* Convert into scratch storage so dst is untouched unless the conversion succeeds */
	AZPackedValue tmp;
	const AZImplementation *tmp_impl;
	AZConversionResult result = az_value_convert_autobox (&tmp_impl, &tmp.v, sizeof (tmp.v), from->impl, &from->v, to_type, conversion_type);
	if (result == AZ_CONVERSION_FAILED) return result;
	az_packed_value_clear (dst);
	dst->impl = tmp_impl;
	dst->v = tmp.v;
	return result;
}
