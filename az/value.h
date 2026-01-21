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

/**
 * @brief A convenience union of most common values
 * 
 * Allows easy access the most common value and block types in values. The maximum size of an
 * value is 16 bytes - thus complex doubles and 4-component float vectors fit into it.
 */
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
		AZObject *object;
		uint8_t data[16];
	};
};

/**
 * @brief A convenience union for value sizes up to 64 bytes
 * 
 * An wrapper around _AZValue that can store larger values - up to 64 bytes, thus
 * 4x4 float matrixes fit into it.
 */
struct _AZValue64 {
	AZValue value;
	uint8_t data[48];
};

/**
 * @brief initialize a value location
 * 
 * For value types it calls the constructor, for block types sets pointer to null.
 * 
 * @param impl the type implementation
 * @param val the value (uninitialized)
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

const AZImplementation *az_value_init_autobox(const AZImplementation *impl, AZValue *dst, unsigned int size);

/**
 * @brief clear an value location
 * 
 * Remove reference hold by the value for reference types. After the operation the val is in uninitialized state.
 * 
 * @param impl the value implementation
 * @param val the value to be cleared
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
 * @param impl the src implementation
 * @param dst the destination (uninitialized)
 * @param src the source value
 */
static inline void
az_value_transfer (const AZImplementation *impl, AZValue *dst, const AZValue *src)
{
	if (impl && az_class_value_size(AZ_CLASS_FROM_IMPL(impl))) {
		memcpy (dst, src, az_class_value_size(AZ_CLASS_FROM_IMPL(impl)));
	}
}

/**
 * @brief tranfer a value to a new location, boxing/unboxing if needed
 * 
 * If the src value does not fit into size bytes, AZBoxedValue is created at dst. If the src
 * is an AZBoxedValue but the value fits into dst, the actual value is unboxed to dst.
 * After the transfer src will be in uninitialized state.
 * 
 * @param impl the src implemntation
 * @param dst the destination (uninitialized)
 * @param src the source value
 * @param size the size of the destination value
 * @return the dst implementation (may be changed by boxing/unboxing)
 */
const AZImplementation *az_value_transfer_autobox(const AZImplementation *impl, AZValue *dst, AZValue *src, unsigned int size);

/**
 * @brief copy a value to a new location
 * 
 * Copy data from initialized src to uninitialized dst. After the operation src will reamin intact.
 * 
 * @param impl the src implemntation
 * @param dst the destination (uninitialized)
 * @param src the source value
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
 * @brief copy a value from one handle to another, boxing/unboxing if needed
 * 
 * Copy data from initialized src to uninitialized dst. After the operation src will reamin intact.
 * If the source does not fit into size bytes AZBoxedValue is created in dst. If src
 * is AZBoxedValue but the value fits into dst the actual value is copied.
 * 
 * @param impl the src implemntation
 * @param dst the destination (uninitialized)
 * @param src the source value
 * @param size the size of the destination value
 * @return the dst implementation (may be changed by boxing/unboxing)
 */
const AZImplementation *az_value_copy_autobox(const AZImplementation *impl, AZValue *dst, const AZValue *src, unsigned int size);

/**
 * @brief set value from instance
 * 
 * @param impl the type implementation
 * @param dst the destination (uninitialized)
 * @param inst the source instance
 */
void az_value_set_from_inst (const AZImplementation *impl, AZValue *dst, void *inst);

/**
 * @brief set value from instance, boxing if needed
 * 
 * If the value does not fit into size bytes, AZBoxedValue is created at dst.
 * It does not unbox automatically.
 * 
 * @param impl the type implemntation
 * @param dst the destination (uninitialized)
 * @param inst the source instance
 * @param size the size of the destination value
 * @return the dst implementation (may be changed by boxing)
 */
const AZImplementation *az_value_set_from_inst_autobox(const AZImplementation *impl, AZValue *dst, void *inst, unsigned int size);

/**
 * @brief dereference instance from value
 * 
 * Does not unbox automatically.
 * 
 * @param impl the type implementation
 * @param val the source value
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
 * @brief ereference instance from value, unboxing if needed
 * 
 * If the value is AZBoxedValue, return the actual implementation and instance inside it.
 * 
 * @param impl the type implementation
 * @param val the source value
 * @param inst the result instance
 * @return the actual implementation (may be changed by unboxing)
 */
const AZImplementation *az_value_get_inst_autobox (const AZImplementation *impl, const AZValue *val, void **inst);

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
unsigned int az_value_equals_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const AZValue *rhs);
unsigned int az_value_equals_instance (const AZImplementation *impl, const AZValue *lhs, const void *rhs);
unsigned int az_value_equals_instance_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const void *rhs);

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
