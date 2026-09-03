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
#include <az/boxed-value.h>
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
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Boxed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, iface_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " in ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, klass->name);
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

AZ_CLASS_ALIGN AZBoxedInterfaceClass AZBoxedInterfaceKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BOXED_INTERFACE },
		.parent = &AZReferenceKlass.klass,
		.name = (const uint8_t *) "boxed interface",
		.alignment = 7,
		.class_size = sizeof(AZBoxedInterfaceClass),
		.instance_size = 0,
		.serialize = serialize_boxed_interface,
		.to_string = boxed_interface_to_string
	},
	.drop = NULL,
	.dispose = NULL
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
	/* The containing value cannot be an interface (a view cannot own a lifecycle) nor a
	 * boxed interface (would nest/defeat the lifecycle guard) */
	arikkei_return_val_if_fail (!AZ_TYPE_IS_INTERFACE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (AZ_IMPL_TYPE(impl) != AZ_TYPE_BOXED_INTERFACE, NULL);
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	/* The interface view has to be inside the containing class/instance */
	arikkei_return_val_if_fail (((const char *) if_impl >= (const char *) impl) && ((const char *) if_impl < (const char *) impl + klass->class_size), NULL);
	/* instance_size is 0 for variable-sized types - the containment is not checkable there */
	arikkei_return_val_if_fail (!klass->instance_size || (((const char *) if_inst >= (const char *) inst) && ((const char *) if_inst < (const char *) inst + klass->instance_size)), NULL);
#endif
	unsigned int val_size = az_class_value_size(AZ_CLASS_FROM_IMPL(impl));
	val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedInterface *boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val.impl = NULL;
	az_packed_value_set (&boxed->val, impl, inst);
	boxed->impl = if_impl;
	boxed->inst = if_inst;
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_value (const AZImplementation *impl, const AZValue *val, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	/* The containing value cannot be an interface nor a boxed interface (lifecycle guard) */
	arikkei_return_val_if_fail (!AZ_TYPE_IS_INTERFACE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (AZ_IMPL_TYPE(impl) != AZ_TYPE_BOXED_INTERFACE, NULL);
	unsigned int val_size = az_class_value_size(AZ_CLASS_FROM_IMPL(impl));
	val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedInterface *boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val.impl = NULL;
	az_packed_value_set_from_val (&boxed->val, impl, val);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val.v), type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_value_autobox (const AZImplementation *impl, const AZValue *val, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (impl == &AZBoxedValueKlass.klass.impl) {
		AZBoxedValue *boxed = (AZBoxedValue *) val->reference;
		impl = &((AZBoxedValue *) val->reference)->klass->impl;
		val = &((AZBoxedValue *) val->reference)->val;
	}
	/* The containing value cannot be an interface nor a boxed interface (lifecycle guard) */
	arikkei_return_val_if_fail (!AZ_TYPE_IS_INTERFACE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (AZ_IMPL_TYPE(impl) != AZ_TYPE_BOXED_INTERFACE, NULL);
	unsigned int val_size = az_class_value_size(AZ_CLASS_FROM_IMPL(impl));
	val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedInterface *boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val.impl = NULL;
	az_packed_value_set_from_val (&boxed->val, impl, val);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val.v), type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_instance (const AZImplementation *impl, void *inst, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	/* The containing value cannot be an interface nor a boxed interface (lifecycle guard) */
	arikkei_return_val_if_fail (!AZ_TYPE_IS_INTERFACE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (AZ_IMPL_TYPE(impl) != AZ_TYPE_BOXED_INTERFACE, NULL);
	unsigned int val_size = az_class_value_size(AZ_CLASS_FROM_IMPL(impl));
	val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedInterface *boxed = (AZBoxedInterface *) malloc (sizeof (AZBoxedInterface) + val_size);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val.impl = NULL;
	az_packed_value_set (&boxed->val, impl, inst);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val.v), type, &boxed->inst);
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
