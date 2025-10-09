#define __AZ_INTERFACE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/interface.h>
#include <az/private.h>

AZInterfaceClass *az_register_interface_type (unsigned int *type, const unsigned char *name, unsigned int parent,
	unsigned int class_size, unsigned int impl_size, unsigned int inst_size, unsigned int flags,
	void (*class_init) (AZClass *),
	void (*implementation_init) (AZImplementation *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
	AZInterfaceClass *if_klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (az_type_is_a (parent, AZ_TYPE_INTERFACE), NULL);
#endif
	az_register_type (type, name, parent, class_size, inst_size, flags, class_init, instance_init, instance_finalize);
	if_klass = (AZInterfaceClass *) AZ_CLASS_FROM_TYPE(*type);
	if_klass->implementation_size = impl_size;
	if_klass->implementation_init = implementation_init;
	return if_klass;
}

static AZInterfaceClass *interface_class = NULL;

void
az_init_interface_class (void)
{
	interface_class = (AZInterfaceClass *) az_class_new_with_type (AZ_TYPE_INTERFACE, AZ_TYPE_BLOCK, sizeof (AZInterfaceClass), 0, AZ_FLAG_ABSTRACT, (const uint8_t *) "interface");
	interface_class->klass.impl.flags |= AZ_FLAG_INTERFACE;
	interface_class->implementation_size = sizeof (AZImplementation);
}
