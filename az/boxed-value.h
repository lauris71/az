#ifndef __AZ_BOXED_VALUE_H__
#define __AZ_BOXED_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2025
*/

#include <stdint.h>

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

static inline const AZImplementation *
az_value_transfer_autobox(const AZImplementation *impl, AZValue *dst, const AZValue *src, unsigned int size)
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
			az_boxed_value_unref(boxed);
		} else {
			az_value_transfer(impl, dst, src);
		}
	}
	return impl;
}

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

#ifdef __cplusplus
};
#endif

#endif
