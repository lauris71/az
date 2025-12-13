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
az_value_set_autobox(AZValue *dst, const AZImplementation *impl, void *inst, unsigned int value_size)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > value_size)) {
		dst->block = az_boxed_value_new(klass, inst);
		return AZ_BOXED_VALUE_IMPL;
	} else {
		az_value_set_from_impl_instance(dst, impl, inst);
		return impl;
	}
}

static inline const AZImplementation *
az_value_copy_autobox(AZValue *dst, const AZImplementation *impl, const AZValue *src, unsigned int value_size)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	if (impl && AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > value_size)) {
		dst->block = az_boxed_value_new_from_val(klass, src);
		return AZ_BOXED_VALUE_IMPL;
	} else {
		az_value_set_from_impl_value(dst, impl, src);
		return impl;
	}
}

static inline const AZImplementation *
az_value_copy_autounbox(AZValue *dst, const AZImplementation *impl, const AZValue *src)
{
	if (impl == AZ_BOXED_VALUE_IMPL) {
		AZBoxedValue *boxed = (AZBoxedValue *) src->block;
		impl = &boxed->klass->impl;
		az_value_set_from_impl_value(dst, impl, &boxed->val);
	} else {
		az_value_set_from_impl_value(dst, impl, src);
	}
	return impl;
}

static inline const AZImplementation *
az_value_autounbox(const AZImplementation *impl, AZValue *val)
{
	if (impl == AZ_BOXED_VALUE_IMPL) {
		AZBoxedValue *boxed = (AZBoxedValue *) val->block;
		impl = &boxed->klass->impl;
		az_value_set_from_impl_value(val, impl, &boxed->val);
		az_boxed_value_unref(boxed);
	}
	return impl;
}

static inline unsigned int
az_value_is_boxed(const AZImplementation *impl, const AZValue *value)
{
	return AZ_IMPL_IS_BOXED_VALUE(impl);
}

static inline unsigned int
az_value_get_unboxed_size(const AZImplementation *impl, const AZValue *value)
{
	if (AZ_IMPL_IS_BOXED_VALUE(impl)) {
		return ((AZBoxedValue *) value)->klass->instance_size;
	} else {
		return az_class_value_size(AZ_CLASS_FROM_IMPL(impl));
	}
}

#ifdef __cplusplus
};
#endif

#endif
