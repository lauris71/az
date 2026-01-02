#ifndef __AZ_PACKED_VALUE_H__
#define __AZ_PACKED_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2019
*/

typedef struct _AZPackedValue AZPackedValue;
typedef struct _AZPackedValue64 AZPackedValue64;

#include <stdio.h>

#include <az/types.h>
#include <az/value.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AZ_PACKED_VALUE_MAX_SIZE 16

struct _AZPackedValue {
	const AZImplementation *impl;
	uint64_t filler;
	AZValue v;
} ARIKKEI_ALIGN_16;

#define AZ_PACKED_VALUE_TYPE(v) AZ_IMPL_TYPE((v)->impl)

struct _AZPackedValue64 {
	union {
		struct {
			const AZImplementation* impl;
			uint64_t filler;
			AZValue64 v;
		};
		struct {
			AZPackedValue packed_val;
			uint8_t data[48];
		};
	};
} ARIKKEI_ALIGN_16;

ARIKKEI_INLINE void
az_value_set_from_packed_value (const AZImplementation **dst_impl, AZValue *dst_val, const AZPackedValue *src)
{
	*dst_impl = src->impl;
	if (src->impl) az_value_copy (src->impl, dst_val, &src->v);
}

/* Packed values */

ARIKKEI_INLINE void
az_packed_value_clear (AZPackedValue *val)
{
	if (val->impl && AZ_IMPL_IS_REFERENCE(val->impl) && val->v.reference) az_reference_unref ((AZReferenceClass *) val->impl, val->v.reference);
	val->impl = NULL;
}


ARIKKEI_INLINE void
az_packed_value_set_boolean (AZPackedValue *val, unsigned int boolean_v)
{
	if (val->impl && AZ_IMPL_IS_REFERENCE(val->impl) && val->v.reference) az_reference_unref ((AZReferenceClass *) val->impl, val->v.reference);
	val->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_BOOLEAN);
	val->v.boolean_v = boolean_v;
}

ARIKKEI_INLINE void
az_packed_value_set_int (AZPackedValue *value, unsigned int type, int val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(type);
	value->v.int32_v = val;
}

ARIKKEI_INLINE void
az_packed_value_set_unsigned_int (AZPackedValue *value, unsigned int type, unsigned int val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(type);
	value->v.uint32_v = val;
}

ARIKKEI_INLINE void
az_packed_value_set_i64 (AZPackedValue *value, int64_t val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_INT64);
	value->v.int64_v = val;
}

ARIKKEI_INLINE void
az_packed_value_set_u64 (AZPackedValue *value, uint64_t val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_UINT64);
	value->v.uint64_v = (uint64_t) val;
}

ARIKKEI_INLINE void
az_packed_value_set_float (AZPackedValue *value, float val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_FLOAT);
	value->v.float_v = val;
}

ARIKKEI_INLINE void
az_packed_value_set_double (AZPackedValue *value, double val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_DOUBLE);
	value->v.double_v = val;
}

ARIKKEI_INLINE void
az_packed_value_set_pointer (AZPackedValue *value, const void *val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_POINTER);
	value->v.pointer_v = (void *) val;
}

ARIKKEI_INLINE void
az_packed_value_set_implementation (AZPackedValue *value, AZImplementation *val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_IMPLEMENTATION);
	value->v.block = val;
}

ARIKKEI_INLINE void
az_packed_value_set_class (AZPackedValue *value, AZClass *val)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_CLASS);
	value->v.block = val;
}

void az_packed_value_set_reference (AZPackedValue *value, unsigned int type, AZReference *ref);
#define az_packed_value_set_string(v, s) az_packed_value_set_reference (v, AZ_TYPE_STRING, (AZReference *) (s))

ARIKKEI_INLINE void
az_packed_value_transfer_reference (AZPackedValue *value, unsigned int type, AZReference *ref)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(type);
	value->v.reference = ref;
}

ARIKKEI_INLINE void
az_packed_value_transfer_string (AZPackedValue *value, AZString *str)
{
	if (value->impl && AZ_IMPL_IS_REFERENCE(value->impl) && value->v.reference) az_reference_unref ((AZReferenceClass *) value->impl, value->v.reference);
	value->impl = (AZImplementation *) az_type_get_class(AZ_TYPE_STRING);
	value->v.string = str;
}

void az_packed_value_copy_indirect (AZPackedValue *dst, const AZPackedValue *src);

ARIKKEI_INLINE void
az_packed_value_copy (AZPackedValue *dst, const AZPackedValue *src)
{
	if (dst == src) return;
	if (!src->impl || (AZ_IMPL_TYPE(src->impl) < AZ_TYPE_REFERENCE)) {
		if (dst->impl && (AZ_IMPL_TYPE(dst->impl) >= AZ_TYPE_REFERENCE)) az_packed_value_clear (dst);
		*dst = *src;
	} else {
		az_packed_value_copy_indirect (dst, src);
	}
}

unsigned int az_packed_value_can_convert (unsigned int to, const AZPackedValue *from);
unsigned int az_packed_value_convert (AZPackedValue *dst, unsigned int type, const AZPackedValue *from);

void *az_packed_value_get_instance (AZPackedValue *value);
void az_packed_value_set_from_type_value (AZPackedValue *dst, unsigned int type, const void *src);
void az_packed_value_set_from_impl_value (AZPackedValue *dst, const AZImplementation *impl, const void *src);
void az_packed_value_set_from_type_instance (AZPackedValue *dst, unsigned int type, void *inst);
void az_packed_value_set_from_impl_instance (AZPackedValue *dst, const AZImplementation *impl, void *inst);

#ifdef __cplusplus
};
#endif


#endif
