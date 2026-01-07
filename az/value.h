#ifndef __AZ_VALUE_H__
#define __AZ_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdint.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/complex.h>
#include <az/interface.h>
#include <az/instance.h>
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
		AZReference *reference;
		AZString *string;
		uint8_t data[16];
	};
};

struct _AZValue64 {
	AZValue value;
	uint8_t data[48];
};

/**
 * @brief initialize a value location
 * 
 * For value types it copies the default value, for block types set pointer to null.
 * 
 * @param impl the type implementation
 * @param val pointer to the value to be initialized
 */
static inline void
az_value_init (const AZImplementation *impl, AZValue *val)
{
	if (AZ_IMPL_IS_VALUE(impl)) {
		az_instance_init_by_type(val, AZ_IMPL_TYPE(impl));
	} else if (AZ_IMPL_IS_BLOCK(impl)) {
		val->block = NULL;
	}
}

/**
 * @brief clear an value location
 * 
 * Remove reference hold by the value for reference types. After the operation the val is uninitialized.
 * 
 * @param impl the tye implementation
 * @param val pointer to the value to be cleared
 */
static inline void
az_value_clear (const AZImplementation *impl, AZValue *val)
{
	if (impl && (AZ_TYPE_IS_REFERENCE(impl->type))) {
		if (val->reference) az_reference_unref ((AZReferenceClass *) impl, val->reference);
	}
}

/**
 * @brief transfer a value to a new location
 * 
 * Move data from initialized src to uninitialized dst. After the operation src will be uninitialized.
 * 
 * @param impl the type implementation
 * @param dst pointer to the destination (uninitialized)
 * @param src pointer to the source (initialized)
 */
static inline void
az_value_transfer (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	if (impl && az_class_value_size(AZ_CLASS_FROM_IMPL(impl))) {
		memcpy (dst, src, az_class_value_size(AZ_CLASS_FROM_IMPL(impl)));
	}
}

/**
 * @brief copy a value to a new unitialized location
 * 
 * @param impl the type implementation
 * @param dst pointer to the source location
 * @param src pointer to the destination location
 */
static inline void
az_value_copy (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	if (impl) {
		if (az_class_value_size(AZ_CLASS_FROM_IMPL(impl))) {
			memcpy (dst, src, az_class_value_size(AZ_CLASS_FROM_IMPL(impl)));
		}
		if (AZ_IMPL_IS_REFERENCE(impl)) {
			if (src->reference) az_reference_ref (src->reference);
		}
	}
}

/**
 * @brief set value from instance
 * 
 * @param impl the type implementation
 * @param dst pointer to the value
 * @param inst pointer to the instance
 */
void az_value_set_from_inst (const AZImplementation *impl, AZValue *dst, void *inst);

/**
 * @brief dereference instance from value
 * 
 * Does not unbox automatically.
 * 
 * @param impl the type implementation
 * @param val pointer to a value
 * @return pointer to the instance
 */
static inline void *
az_value_get_inst (const AZImplementation *impl, const AZValue *val)
{
	if (impl && (AZ_IMPL_IS_BLOCK(impl))) {
		return val->block;
	} else {
		return (void *) val;
	}
}

/**
 * @brief creates a value from serialized data
 * 
 * @param impl the type implementation
 * @param val pointer to uninitialized target value
 * @param s source buffer
 * @param slen  source buffer length
 * @param ctx the execution context
 * @return the number of bytes consumed (0 on error)
 */
unsigned int az_value_deserialize (const AZImplementation *impl, AZValue *val, const unsigned char *s, unsigned int slen, AZContext *ctx);

/**
 * @brief compares two values for equality
 * 
 * Value equality is defined as bitwise equality of value data - i.e. equality of structs
 * or block pointers
 * 
 * @param impl the type implementation
 * @param lhs left hand value
 * @param rhs right hand value
 * @return 1 if equal, 0 if not
 */
unsigned int az_value_equals (const AZImplementation *impl, const AZValue *lhs, const AZValue *rhs);
unsigned int az_value_equals_instance (const AZImplementation *impl, const AZValue *lhs, const void *rhs);

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

void *az_value_new_array(const AZImplementation *impl, unsigned int length);
void az_value_delete_array(const AZImplementation *impl, void *data, unsigned int length);

unsigned int az_value_convert_auto (const AZImplementation **dst_impl, AZValue *dst_val, const AZImplementation **src_impl, const AZValue *src_val, unsigned int to_type);
unsigned int az_value_convert_in_place (const AZImplementation **impl, AZValue *val, unsigned int to_type);

#ifdef __cplusplus
};
#endif


#endif
