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

AZBoxedValue *az_boxed_value_new(const AZClass *klass);
AZBoxedValue *az_boxed_value_new_from_inst(const AZClass *klass, void *inst);
AZBoxedValue *az_boxed_value_new_from_val(const AZClass *klass, const AZValue *val);

static inline const AZImplementation *
az_boxed_value_unbox(AZValue *dst, AZBoxedValue *boxed)
{
	az_value_copy(&boxed->klass->impl, dst, &boxed->val);
	return &boxed->klass->impl;
}

static inline void
az_boxed_value_ref (AZBoxedValue *boxed)
{
	az_reference_ref(&boxed->ref);
}

static inline void
az_boxed_value_unref (AZBoxedValue *boxed)
{
	az_reference_unref(&AZBoxedValueKlass, &boxed->ref);
}

#ifdef __cplusplus
};
#endif

#endif
