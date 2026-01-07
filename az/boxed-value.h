#ifndef __AZ_BOXED_VALUE_H__
#define __AZ_BOXED_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2025
*/

#include <stdint.h>

#include <az/boxed-interface.h>
#include <az/reference.h>
#include <az/packed-value.h>

typedef struct _AZReferenceClass AZBoxedValueClass;

#define AZ_BOXED_VALUE_CLASS (&AZBoxedValueKlass)
#define AZ_BOXED_VALUE_IMPL ((AZImplementation *) &AZBoxedValueKlass)

#ifdef __cplusplus
extern "C" {
#endif

struct _AZBoxedValue {
	AZReference ref;
    uint32_t filler_1;
	const AZClass *klass;
	AZValue val;
};

extern AZBoxedValueClass AZBoxedValueKlass;

AZBoxedValue *az_boxed_value_new (const AZClass *klass, void *inst);
AZBoxedValue *az_boxed_value_new_from_val (const AZClass *klass, const AZValue *val);

static inline const AZImplementation *
az_boxed_value_unbox(AZValue *dst, AZBoxedValue *boxed)
{
	az_value_copy(&boxed->klass->impl, dst, &boxed->val);
	return &boxed->klass->impl;
}

ARIKKEI_INLINE void
az_boxed_value_ref (AZBoxedValue *boxed)
{
	az_reference_ref(&boxed->ref);
}

ARIKKEI_INLINE void
az_boxed_value_unref (AZBoxedValue *boxed)
{
	az_reference_unref(&AZBoxedValueKlass, &boxed->ref);
}

/**
 * @brief tranfer a value from one handle to another, boxing/unboxing if needed
 * 
 * If the source does not fit into size bytes AZBoxedValue is created in dst. If src
 * is AZBoxedValue but the value fits into dst the actual value is copied.
 * After the transfer src will be in uninitialized state.
 * 
 * @param impl the src implemntation
 * @param dst the destination value
 * @param src the source value
 * @param size the size of destination value
 * @return the dst implementation (may be changed by boxing/unboxing)
 */
static inline const AZImplementation *
az_value_transfer_autobox(const AZImplementation *impl, AZValue *dst, AZValue *src, unsigned int size)
{
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			// Value type that does not fit into dst, box
			dst->block = az_boxed_value_new_from_val(klass, src);
			impl = AZ_BOXED_VALUE_IMPL;
			az_value_clear(impl, src);
		} else if ((klass == (AZClass *) &AZBoxedValueKlass) && (((AZBoxedValue *) src->block)->klass->instance_size <= size)) {
			// Boxed value that fits into dst, unbox
			AZBoxedValue *boxed = (AZBoxedValue *) src->block;
			impl = &boxed->klass->impl;
			az_value_copy(impl, dst, &boxed->val);
			az_value_clear(impl, src);
		} else {
			az_value_transfer(impl, dst, src);
		}
	}
	return impl;
}

/**
 * @brief copy a value from one handle to another, boxing/unboxing if needed
 * 
 * If the source does not fit into size bytes AZBoxedValue is created in dst. If src
 * is AZBoxedValue but the value fits into dst the actual value is copied.
 * 
 * @param impl the src implemntation
 * @param dst the destination value
 * @param src the source value
 * @param size the size of destination value
 * @return the dst implementation (may be changed by boxing/unboxing)
 */
static inline const AZImplementation *
az_value_copy_autobox(const AZImplementation *impl, AZValue *dst, const AZValue *src, unsigned int size)
{
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (impl && AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			// Value type that does not fit into dst, box
			dst->block = az_boxed_value_new_from_val(klass, src);
			impl = AZ_BOXED_VALUE_IMPL;
		} else if ((klass == (AZClass *) &AZBoxedValueKlass) && (((AZBoxedValue *) src->block)->klass->instance_size <= size)) {
			// Boxed value that fits into dst, unbox
			AZBoxedValue *boxed = (AZBoxedValue *) src->block;
			impl = &boxed->klass->impl;
			az_value_copy(impl, dst, &boxed->val);
		} else {
			az_value_copy(impl, dst, src);
		}
	}
	return impl;
}

/**
 * @brief set value from instance, boxing if needed
 * 
 * If the value does not fit into size bytes AZBoxedValue is created in dst.
 * It does not unbox automatically.
 * 
 * @param impl the type implemntation
 * @param dst the destination value
 * @param inst the source instance
 * @param size the size of destination value
 * @return the dst implementation (may be changed by boxing)
 */
static inline const AZImplementation *
az_value_set_from_inst_autobox(const AZImplementation *impl, AZValue *dst, void *inst, unsigned int size)
{
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			dst->block = az_boxed_value_new(klass, inst);
			impl = AZ_BOXED_VALUE_IMPL;
		} else {
			az_value_set_from_inst(impl, dst, inst);
		}
	}
	return impl;
}

/**
 * @brief get the instance from value, unboxing if needed
 * 
 * If the value is AZBoxedValue, return the actual implementation and instance inside it.
 * 
 * @param impl the type implementation
 * @param val the source value
 * @param inst the destination instance
 * @return the actual implementation (may be changed by unboxing)
 */
static inline const AZImplementation *
az_value_get_inst_autobox (const AZImplementation *impl, const AZValue *val, void **inst)
{
	if (impl == (AZImplementation *) &AZBoxedValueKlass) {
		*inst = &((AZBoxedValue *) val->block)->val;
		impl = &((AZBoxedValue *) val->block)->klass->impl;
	} else if (impl && (AZ_IMPL_IS_BLOCK(impl))) {
		*inst = val->block;
	} else {
		*inst = (void *) val;
	}
	return impl;
}

unsigned int az_value_equals_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const AZValue *rhs);
unsigned int az_value_equals_instance_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const void *rhs);

#ifdef __cplusplus
};
#endif

#endif
