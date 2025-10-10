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

typedef struct _AZBoxedValueClass AZBoxedValueClass;

#define AZ_BOXED_VALUE_CLASS az_boxed_value_class
#define AZ_BOXED_VALUE_IMPL ((AZImplementation *) az_boxed_value_class)

#ifdef __cplusplus
extern "C" {
#endif

struct _AZBoxedValue {
	AZReference ref;
    uint32_t filler_1;
	const AZImplementation *impl;
	AZValue val;
};

struct _AZBoxedValueClass {
	AZReferenceClass ref_class;
};

#ifndef __AZ_BOXED_VALUE_C__
extern AZBoxedValueClass *az_boxed_value_class;
#else
AZBoxedValueClass *az_boxed_value_class = NULL;
#endif

AZBoxedValue *az_boxed_value_new (const AZImplementation *impl, void *inst);
AZBoxedValue *az_boxed_value_new_from_impl_value (const AZImplementation *impl, const AZValue *val);

ARIKKEI_INLINE void
az_boxed_value_ref (AZBoxedValue *boxed)
{
	az_reference_ref(&boxed->ref);
}

ARIKKEI_INLINE void
az_boxed_value_unref (AZBoxedValue *boxed)
{
	az_reference_unref(&az_boxed_value_class->ref_class, &boxed->ref);
}

static inline const AZImplementation *
az_value_set_autobox(AZValue *dst, const AZImplementation *impl, void *inst, unsigned int value_size)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > value_size)) {
		dst->block = az_boxed_value_new(impl, inst);
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
	if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > value_size)) {
		dst->block = az_boxed_value_new_from_impl_value(impl, src);
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
		impl = boxed->impl;
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
		impl = boxed->impl;
		az_value_set_from_impl_value(val, impl, &boxed->val);
		az_boxed_value_unref(boxed);
	}
	return impl;
}

#ifdef AZ_INTERNAL
/* Library internal */
void az_init_boxed_value_class (void);
#endif

#ifdef __cplusplus
};
#endif

#endif
