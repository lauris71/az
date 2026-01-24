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

typedef struct _AZReferenceClass AZBoxedInterfaceClass;

#define AZ_BOXED_INTERFACE_CLASS (&AZBoxedInterfaceKlass)
#define AZ_BOXED_INTERFACE_IMPL ((AZImplementation *) &AZBoxedInterfaceKlass)

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

extern AZBoxedInterfaceClass AZBoxedInterfaceKlass;

AZBoxedInterface *az_boxed_interface_new (const AZImplementation *impl, void *inst, const AZImplementation *if_impl, void *if_inst);
AZBoxedInterface *az_boxed_interface_new_from_impl_value (const AZImplementation *impl, AZValue *val, unsigned int type);
AZBoxedInterface *az_boxed_interface_new_from_impl_value_autobox (const AZImplementation *impl, AZValue *val, unsigned int type);
AZBoxedInterface *az_boxed_interface_new_from_impl_instance (const AZImplementation *impl,void *inst, unsigned int type);
AZBoxedInterface *az_boxed_interface_new_from_object (AZObject *obj, unsigned int type);

ARIKKEI_INLINE void
az_boxed_interface_ref(AZBoxedInterface *astr)
{
	az_reference_ref (&astr->reference);
}

ARIKKEI_INLINE void
az_boxed_interface_unref(AZBoxedInterface *astr)
{
	az_reference_unref (&AZBoxedInterfaceKlass, &astr->reference);
}

static inline const AZImplementation *
az_value_set_interface_autobox(const AZImplementation *impl, AZValue *dst, void *inst, unsigned int type)
{
	AZBoxedInterface *boxed = az_boxed_interface_new_from_impl_instance(impl, inst, type);
	az_value_transfer_reference(dst, &boxed->reference);
	return AZ_BOXED_INTERFACE_IMPL;
}

ARIKKEI_INLINE void
az_boxed_interface_unbox(const AZImplementation **impl, void **inst) {
	AZBoxedInterface *boxed = (AZBoxedInterface *) *inst;
	*impl = boxed->impl;
	*inst = boxed->inst;
}

#ifdef __cplusplus
};
#endif

#endif
