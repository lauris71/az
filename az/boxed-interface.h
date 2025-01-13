#ifndef __AZ_BOXED_INTERFACE_H__
#define __AZ_BOXED_INTERFACE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2021
*/

#include <stdint.h>

#include <az/reference.h>
#include <az/packed-value.h>

typedef struct _AZBoxedInterfaceClass AZBoxedInterfaceClass;

#ifdef __cplusplus
extern "C" {
#endif

struct _AZBoxedInterface {
	AZReference reference;
	const AZImplementation *impl;
	void *inst;
	uint64_t filler;
	AZPackedValue val;
};

struct _AZBoxedInterfaceClass {
	AZReferenceClass reference_class;
};

#ifndef __AZ_BOXED_INTERFACE_C__
extern AZBoxedInterfaceClass *az_boxed_interface_class;
#else
AZBoxedInterfaceClass *az_boxed_interface_class = NULL;
#endif

AZBoxedInterface *az_boxed_interface_new (const AZImplementation *impl, void *inst, const AZImplementation *if_impl, void *if_inst);
AZBoxedInterface *az_boxed_interface_new_from_impl_value (const AZImplementation *impl, const AZValue *val, unsigned int type);
AZBoxedInterface *az_boxed_interface_new_from_impl_instance (const AZImplementation *impl,void *inst, unsigned int type);
AZBoxedInterface *az_boxed_interface_new_from_object (AZObject *obj, unsigned int type);

ARIKKEI_INLINE void
az_boxed_interface_ref (AZBoxedInterface *astr)
{
	az_reference_ref (&astr->reference);
}

ARIKKEI_INLINE void
az_boxed_interface_unref (AZBoxedInterface *astr)
{
	az_reference_unref (&az_boxed_interface_class->reference_class, &astr->reference);
}

#ifdef AZ_INTERNAL
/* Library internal */
void az_init_boxed_interface_class (void);
#endif

#ifdef __cplusplus
};
#endif

#endif
