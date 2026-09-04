#define __AZ_BOXED_INTERFACE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/allocator.h>
#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-threads.h>
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
	AZClass *klass = AZ_CLASS_FROM_IMPL(boxed->val_impl);
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

/* The box answers interface and property queries on behalf of the boxed content */
static const AZImplementation *
boxed_interface_delegate_get_interface (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	/* Type-level queries (inst == NULL) cannot see through the box: the content is per-instance */
	if (!inst) return NULL;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_get_interface (boxed->impl, boxed->inst, if_type, if_inst);
}

static int
boxed_interface_delegate_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	if (!inst) return -1;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_class_lookup_property (AZ_CLASS_FROM_IMPL (boxed->impl), boxed->impl, boxed->inst, key, def_class, sub_impl, sub_inst);
}

static int
boxed_interface_delegate_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	if (!inst) return -1;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_class_lookup_function (AZ_CLASS_FROM_IMPL (boxed->impl), boxed->impl, boxed->inst, key, sig, def_class, sub_impl, sub_inst);
}

static unsigned int
boxed_interface_delegate_foreach_property (const AZClass *klass, const AZImplementation *impl, void *inst, AZPropertyForeachFunc cb, void *data)
{
	if (!inst) return 1;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_foreach_property (boxed->impl, boxed->inst, cb, data);
}

static unsigned int
boxed_interface_delegate_foreach_interface (const AZClass *klass, const AZImplementation *impl, void *inst, AZInterfaceForeachFunc cb, void *data)
{
	if (!inst) return 1;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_foreach_interface (boxed->impl, boxed->inst, cb, data);
}

static const AZClassDelegate az_boxed_interface_delegate = {
	boxed_interface_delegate_get_interface,
	boxed_interface_delegate_lookup_property,
	boxed_interface_delegate_lookup_function,
	boxed_interface_delegate_foreach_property,
	boxed_interface_delegate_foreach_interface
};

/*
 * Pool allocator
 *
 * The vast majority of boxed interfaces sit on block containers (impl+inst are
 * plain pointers) or on value containers of at most 16 bytes, so the box is
 * exactly sizeof(AZBoxedInterface) == 48 bytes and is served by the pool.
 * Bigger value containers need the allocation tail and fall back to malloc.
 * The two cases are told apart at free time from the container's value size
 * (val_impl survives finalization), so no per-instance tag is needed.
 */
#define AZ_BOXED_INTERFACE_POOL_SIZE 64
static ArikkeiPool boxed_iface_pool;
static mtx_t boxed_iface_pool_mutex;

static void *
boxed_iface_alloc (const AZImplementation *impl)
{
	unsigned int content_size = az_class_value_size (AZ_CLASS_FROM_IMPL (impl));
	unsigned int val_size = (content_size > 16) ? content_size - 16 : 0;
	if (val_size) return malloc (sizeof (AZBoxedInterface) + val_size);
	mtx_lock (&boxed_iface_pool_mutex);
	void *p = arikkei_pool_alloc (&boxed_iface_pool);
	mtx_unlock (&boxed_iface_pool_mutex);
	return p;
}

static void *
boxed_iface_allocate (AZClass *klass)
{
	/* Only ever serves the fixed-size (48 byte) box: the class is variable-sized
	 * (instance_size == 0), so az_instance_new/array never reach this */
	mtx_lock (&boxed_iface_pool_mutex);
	void *p = arikkei_pool_alloc (&boxed_iface_pool);
	mtx_unlock (&boxed_iface_pool_mutex);
	return p;
}

static void
boxed_iface_free (AZClass *klass, void *location)
{
	AZBoxedInterface *boxed = (AZBoxedInterface *) location;
	unsigned int content_size = (boxed->val_impl) ? az_class_value_size (AZ_CLASS_FROM_IMPL (boxed->val_impl)) : 0;
	unsigned int val_size = (content_size > 16) ? content_size - 16 : 0;
	if (val_size) {
		free (boxed);
		return;
	}
	mtx_lock (&boxed_iface_pool_mutex);
	arikkei_pool_free (&boxed_iface_pool, boxed);
	mtx_unlock (&boxed_iface_pool_mutex);
}

static AZInstanceAllocator boxed_iface_allocator = {
	boxed_iface_allocate,
	NULL,	/* allocate_array */
	boxed_iface_free,
	NULL	/* free_array */
};

/* Releases the packed container value (which owns the interface view's lifecycle) */
static void
boxed_interface_finalize (const AZImplementation *impl, void *inst)
{
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	az_value_clear (boxed->val_impl, &boxed->val);
}

AZ_CLASS_ALIGN AZBoxedInterfaceClass AZBoxedInterfaceKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BOXED_INTERFACE },
		.parent = &AZReferenceKlass.klass,
		.name = (const uint8_t *) "boxed interface",
		.alignment = 7,
		.class_size = sizeof(AZBoxedInterfaceClass),
		.instance_size = 0,
		.instance_finalize = boxed_interface_finalize,
		.serialize = serialize_boxed_interface,
		.to_string = boxed_interface_to_string
	},
	.drop = NULL,
	.dispose = NULL
};

void
az_init_boxed_interface_class (void)
{
	arikkei_pool_setup (&boxed_iface_pool, sizeof (AZBoxedInterface), AZ_BOXED_INTERFACE_POOL_SIZE);
	mtx_init (&boxed_iface_pool_mutex, mtx_plain);
	AZBoxedInterfaceKlass.klass.delegate_idx = az_class_register_delegate (&az_boxed_interface_delegate);
	AZBoxedInterfaceKlass.klass.allocator_idx = az_class_register_allocator (&boxed_iface_allocator);
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
	/* The implementation may live in another (immortal) class - e.g. delegated through
	 * a reference-of - so only the instance side is checked: its lifetime is bound to
	 * the container, which the box owns. */
	/* instance_size is 0 for variable-sized types - the containment is not checkable there */
	arikkei_return_val_if_fail (!klass->instance_size || (((const char *) if_inst >= (const char *) inst) && ((const char *) if_inst < (const char *) inst + klass->instance_size)), NULL);
#endif
	AZBoxedInterface *boxed = (AZBoxedInterface *) boxed_iface_alloc (impl);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val_impl = impl;
	az_value_set_from_inst (impl, &boxed->val, inst);
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
	AZBoxedInterface *boxed = (AZBoxedInterface *) boxed_iface_alloc (impl);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val_impl = impl;
	az_value_set_from_inst (impl, &boxed->val, az_value_get_inst(impl, val));
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val), type, &boxed->inst);
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
	AZBoxedInterface *boxed = (AZBoxedInterface *) boxed_iface_alloc (impl);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val_impl = impl;
	az_value_set_from_inst (impl, &boxed->val, az_value_get_inst(impl, val));
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val), type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_instance (const AZImplementation *impl, void *inst, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	/* The containing value cannot be an interface nor a boxed interface (lifecycle guard) */
	arikkei_return_val_if_fail (!AZ_TYPE_IS_INTERFACE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (AZ_IMPL_TYPE(impl) != AZ_TYPE_BOXED_INTERFACE, NULL);
	AZBoxedInterface *boxed = (AZBoxedInterface *) boxed_iface_alloc (impl);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val_impl = impl;
	az_value_set_from_inst (impl, &boxed->val, inst);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val), type, &boxed->inst);
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
