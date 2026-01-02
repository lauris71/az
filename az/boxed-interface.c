#define __AZ_BOXED_INTERFACE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-utils.h>

#include <az/boxed-interface.h>
#include <az/instance.h>
#include <az/class.h>
#include <az/object.h>
#include <az/private.h>
#include <az/serialization.h>

static unsigned int
serialize_boxed_interface (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx) {
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_serialize(boxed->impl, boxed->inst, d, dlen, ctx);
}

static unsigned int
boxed_interface_to_string (const AZImplementation *impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	AZClass *klass = AZ_CLASS_FROM_IMPL(boxed->val.impl);
	AZClass *iface_class = AZ_CLASS_FROM_IMPL(boxed->impl);
	unsigned int pos;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Boxed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, iface_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " in ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, klass->name);
	if (pos < len) buf[pos] = 0;
	return pos;
}

AZBoxedInterfaceClass AZBoxedInterfaceKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BOXED_INTERFACE},
	&AZReferenceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "boxed interface",
	7, sizeof(AZBoxedInterfaceClass), 0,
	NULL,
	NULL, NULL,
	serialize_boxed_interface, NULL, boxed_interface_to_string,
	NULL, NULL},
	NULL, NULL
};

void
az_init_boxed_interface_class (void)
{
	az_class_new_with_value(&AZBoxedInterfaceKlass.klass);
}

AZBoxedInterface *
az_boxed_interface_new (const AZImplementation *impl, void *inst, const AZImplementation *if_impl, void *if_inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	arikkei_return_val_if_fail (if_impl != NULL, NULL);
	arikkei_return_val_if_fail (!AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl)), NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
	AZBoxedInterface *boxed;
	if (val_size > 16) {
		boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size - 16);
	} else {
		boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface));
	}
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_INTERFACE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_instance (&boxed->val, impl, inst);
	boxed->impl = if_impl;
	boxed->inst = if_inst;
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_value (const AZImplementation *impl, const AZValue *val, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
	AZBoxedInterface *boxed;
	if (val_size > 16) {
		boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size - 16);
	} else {
		boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface));
	}
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_INTERFACE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_value (&boxed->val, impl, val);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, val), type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_instance (const AZImplementation *impl, void *inst, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
	AZBoxedInterface *boxed;
	if (val_size > 16) {
		boxed = ( AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size - 16);
	} else {
		boxed = ( AZBoxedInterface *) malloc (sizeof (AZBoxedInterface));
	}
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_INTERFACE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_instance (&boxed->val, impl, inst);
	boxed->impl = az_instance_get_interface (impl, inst, type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_object (AZObject *obj, unsigned int type)
{
	const AZImplementation *if_impl;
	void *if_inst;
	if_impl = az_object_get_interface (obj, type, &if_inst);
	return az_boxed_interface_new ((const AZImplementation *) obj->klass, obj, if_impl, if_inst);
}
