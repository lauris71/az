#ifndef __AZ_BOXED_INTERFACE_H__
#define __AZ_BOXED_INTERFACE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2021
*/

#include <stdint.h>

#include <az/reference.h>
#include <az/value.h>

typedef struct _AZReferenceClass AZBoxedInterfaceClass;

#define AZ_BOXED_INTERFACE_CLASS (&AZBoxedInterfaceKlass)
#define AZ_BOXED_INTERFACE_IMPL ((AZImplementation *) &AZBoxedInterfaceKlass)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A reference type that holds an interface together with the containing instance
 *
 * These are used to enforce the lifecycle of interface values. Plain interfaces will become invalid as soon as
 * the containing instance is destroyed. Thus storing these in containers is prone to use-after-free bugs.
 *
 * The content value may extend past the end of the struct (the allocation has
 * room for it; the box owns a copy of value containers and a reference to
 * reference containers).
 */
struct _AZBoxedInterface {
	AZReference reference;
	uint32_t _reserved;
	/* The interface view */
	const AZImplementation *impl;
	void *inst;
	/* The packed container value (owns the lifecycle); the value bytes may use
	 * the allocation tail past the struct end if the container is > 16 bytes */
	const AZImplementation *val_impl;
	AZValue val;
};

extern AZBoxedInterfaceClass AZBoxedInterfaceKlass;

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
