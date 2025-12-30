#define __AZ_INTERFACE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/interface.h>
#include <az/private.h>
#include <az/extend.h>

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

AZInterfaceClass AZInterfaceKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INTERFACE},
	&AZBlockKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "interface",
	3, sizeof(AZInterfaceClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL},
	sizeof(AZImplementation), NULL
};

void
az_init_interface_class (void)
{
	az_class_new_with_value(&AZInterfaceKlass.klass);
}

static void
implementation_init_recursive (AZInterfaceClass *ifclass, AZImplementation *impl)
{
	/* Init superimplementations */
	if (AZ_CLASS_IS_INTERFACE(ifclass->klass.parent)) {
		implementation_init_recursive ((AZInterfaceClass *) ifclass->klass.parent, impl);
	}
	/* Init subimplementations */
	const AZIFEntry *ifentry = az_class_iface_self(&ifclass->klass, 0);
	for (uint16_t i = 0; i < ifclass->klass.n_ifaces_self; i++) {
		AZInterfaceClass *sub_class = (AZInterfaceClass *) AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		sub_impl->type = ifentry->type;
		implementation_init_recursive (sub_class, sub_impl);
		ifentry += 1;
	}
	/* Implementation itself */
	if (ifclass->implementation_init) ifclass->implementation_init (impl);
}

void
az_implementation_init_by_type (AZImplementation *impl, unsigned int type)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_types);
	arikkei_return_if_fail (AZ_TYPE_IS_INTERFACE(type));
#endif
	AZInterfaceClass *ifclass = (AZInterfaceClass *) AZ_CLASS_FROM_TYPE(type);
	impl->klass = &ifclass->klass;
	implementation_init_recursive (ifclass, impl);
}
