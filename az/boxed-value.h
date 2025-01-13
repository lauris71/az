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

#ifdef __cplusplus
extern "C" {
#endif

struct _AZBoxedValue {
	AZReference ref;
    uint32_t filler_1;
    uint64_t filler_2;
	AZPackedValue val;
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
AZBoxedValue *az_boxed_value_new_from_impl_instance (const AZImplementation *impl,void *inst);

ARIKKEI_INLINE void
az_boxed_value_ref (AZBoxedValue *boxed)
{
	az_reference_ref (&boxed->ref);
}

ARIKKEI_INLINE void
az_boxed_value_unref (AZBoxedValue *boxed)
{
	az_reference_unref (&az_boxed_value_class->ref_class, &boxed->ref);
}

#ifdef AZ_INTERNAL
/* Library internal */
void az_init_boxed_value_class (void);
#endif

#ifdef __cplusplus
};
#endif

#endif
