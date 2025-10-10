#ifndef __AZ_VALUE_H__
#define __AZ_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdint.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/complex.h>
#include <az/interface.h>
#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AZ_PACKED_VALUE_MAX_SIZE 16

struct _AZValue {
	union {
		uint32_t boolean_v;
		int8_t int8_v;
		uint8_t uint8_v;
		int16_t int16_v;
		uint16_t uint16_v;
		int32_t int32_v;
		uint32_t uint32_v;
		int64_t int64_v;
		uint64_t uint64_v;
		float float_v;
		double double_v;
		AZComplexFloat cfloat_v;
		AZComplexDouble cdouble_v;
		void *pointer_v;
		void *block;
		AZInterfaceValue iface;
		AZReference *reference;
		AZString *string;
		uint8_t data[16];
	};
};

struct _AZValue64 {
	AZValue value;
	uint8_t data[48];
};

#ifdef AZ_SAFETY_CHECKS
void az_value_init (const AZImplementation *impl, AZValue *val);
void az_clear_value (const AZImplementation *impl, AZValue *val);
void az_transfer_value (const AZImplementation *impl, AZValue *dst, const AZValue *src);
void *az_instance_from_value (const AZImplementation *impl, const AZValue *value);
#else
ARIKKEI_INLINE void
az_value_init (const AZImplementation *impl, AZValue *val)
{
	AZClass *klass = az_type_get_class (impl->type);
	if (klass->flags & AZ_FLAG_VALUE) {
		az_instance_init(val, impl->type);
	} else if (klass->flags & AZ_FLAG_BLOCK) {
		val->block = NULL;
	}
}

ARIKKEI_INLINE void
az_clear_value (const AZImplementation *impl, AZValue *val)
{
	if (impl && (AZ_TYPE_IS_REFERENCE(impl->type))) {
		if (val->reference) az_reference_unref (( AZReferenceClass *) impl, val->reference);
	}
}

ARIKKEI_ININE void
az_transfer_value (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	if (impl && az_classes[impl->type]->value_size) memcpy (dst, src, az_classes[impl->type]->value_size);
}

ARIKKEI_INLINE void *
az_instance_from_value (const AZImplementation *impl, const AZValue *value)
{
	if (impl && (AZ_FLAG_BLOCK(impl->type))) {
		return value->block;
	} else {
		return (void *) value;
	}
}
#endif

unsigned int az_value_equals (const AZImplementation *impl, const AZValue *lhs, const AZValue *rhs);
unsigned int az_value_equals_instance (const AZImplementation *impl, const AZValue *lhs, const void *rhs);

void az_copy_value (const AZImplementation *impl, AZValue *dst, const AZValue *src);
void az_set_value_from_instance (const AZImplementation *impl, AZValue *dst, void *inst);
#define az_value_set_from_impl_value(dst, impl, src) az_copy_value(impl, dst, src)
#define az_value_set_from_impl_instance(dst, impl, src) az_set_value_from_instance(impl, dst, src)

/* Transfer reference instance to destination */

ARIKKEI_INLINE void
az_value_transfer_reference (AZValue *dst_val, AZReference *inst)
{
	dst_val->reference = inst;
}

/* Copy reference instance to destination */

ARIKKEI_INLINE void
az_value_set_reference (AZValue *dst_val, AZReference *inst)
{
	dst_val->reference = inst;
	if (inst) az_reference_ref (inst);
}

unsigned int az_value_convert_auto (const AZImplementation **dst_impl, AZValue *dst_val, const AZImplementation **src_impl, const AZValue *src_val, unsigned int to_type);
unsigned int az_value_convert_in_place (const AZImplementation **impl, AZValue *val, unsigned int to_type);

AZImplementation *az_value_box (AZValue *dst_val, const AZImplementation *src_impl, const AZValue *src_val);
AZImplementation *az_value_debox (AZValue *dst_val, const AZImplementation *src_impl, const AZValue *src_val);

#ifdef __cplusplus
};
#endif


#endif
