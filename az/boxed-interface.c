#define __AZ_BOXED_INTERFACE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stddef.h>

#include <arikkei/allocator.h>
#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-threads.h>
#include <arikkei/arikkei-utils.h>

#include <az/boxed-interface.h>
#include <az/boxed-value.h>
#include <az/instance.h>
#include <az/extend.h>
#include <az/object.h>
#include <az/private.h>
#include <az/serialization.h>

static int64_t
serialize_boxed_interface (const AZImplementation *impl, void *inst, uint8_t *d, uint64_t dlen) {
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_serialize(boxed->impl, boxed->inst, d, dlen);
}

static int64_t
boxed_interface_to_string (const AZImplementation *impl, void *inst, uint8_t *buf, uint64_t len)
{
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	AZClass *klass = AZ_CLASS_FROM_IMPL(boxed->val_impl);
	AZClass *iface_class = AZ_CLASS_FROM_IMPL(boxed->impl);
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, (const uint8_t *) "Boxed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, iface_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const uint8_t *) " in ");
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
	if (!inst) return AZ_NULL_VALUE;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_class_lookup_property (AZ_CLASS_FROM_IMPL (boxed->impl), boxed->impl, boxed->inst, key, def_class, sub_impl, sub_inst);
}

static int
boxed_interface_delegate_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	if (!inst) return AZ_NULL_VALUE;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_class_lookup_function (AZ_CLASS_FROM_IMPL (boxed->impl), boxed->impl, boxed->inst, key, sig, def_class, sub_impl, sub_inst);
}

static int
boxed_interface_delegate_foreach_property (const AZClass *klass, const AZImplementation *impl, void *inst, AZPropertyForeachFunc cb, void *data)
{
	if (!inst) return AZ_OK;
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	return az_instance_foreach_property (boxed->impl, boxed->inst, cb, data);
}

static int
boxed_interface_delegate_foreach_interface (const AZClass *klass, const AZImplementation *impl, void *inst, AZInterfaceForeachFunc cb, void *data)
{
	if (!inst) return AZ_OK;
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
 * The vast majority of boxed interfaces sit on reference containers (impl+inst are
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
boxed_iface_allocate (AZClass *klass, unsigned int size)
{
	/* The pool/malloc decision lives here: the common 48-byte box is pooled,
	 * bigger (tailed) boxes go to the generic allocator */
	if (size > sizeof (AZBoxedInterface)) return malloc (size);
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
	.allocate = boxed_iface_allocate,
	.free = boxed_iface_free
};

/* Releases the packed container value (which owns the interface view's lifecycle).
 * Note: val_impl is deliberately left intact - the allocator's free uses it to
 * recompute the allocation size class (pool vs malloc) after finalization. */
static void
boxed_interface_finalize (const AZImplementation *impl, void *inst)
{
	AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
	az_value_clear (boxed->val_impl, &boxed->val);
}

AZ_CLASS_ALIGN AZBoxedInterfaceClass AZBoxedInterfaceKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_VARIABLE_SIZE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BOXED_INTERFACE },
		.parent = &AZReferenceKlass.klass,
		.name = (const uint8_t *) "boxed interface",
		.alignment = 7,
		.class_size = sizeof(AZBoxedInterfaceClass),
		/* The minimum size; value containers > 16 bytes use an allocation tail */
		.instance_size = sizeof(AZBoxedInterface),
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
	AZBoxedInterfaceKlass.klass.delegate_idx = az_register_delegate (&az_boxed_interface_delegate);
	AZBoxedInterfaceKlass.klass.allocator_idx = az_register_allocator (&boxed_iface_allocator);
	az_class_new_with_value(&AZBoxedInterfaceKlass.klass);
}

static AZBoxedInterface *
boxed_iface_container_new(const AZImplementation *cont_impl)
{
	unsigned int content_size = az_class_value_size(AZ_CLASS_FROM_IMPL(cont_impl));
	unsigned int val_size = (content_size > 16) ? content_size - 16 : 0;
	AZBoxedInterface *boxed = (AZBoxedInterface *) az_instance_new_sized (AZ_TYPE_BOXED_INTERFACE, sizeof (AZBoxedInterface) + val_size);
	az_instance_init(&AZBoxedInterfaceKlass.klass.impl, boxed);
	boxed->val_impl = cont_impl;
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new (const AZImplementation *impl, void *inst, const AZImplementation *if_impl, void *if_inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	arikkei_return_val_if_fail (if_impl != NULL, NULL);
	/* Law B: the box owns the container - only value (copied) and reference (reffed)
	 * containers qualify. Plain blocks are caller-managed and stay naked; interfaces
	 * are views (cannot own a lifecycle) and boxes are transparent (no nesting) */
	arikkei_return_val_if_fail (AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl)) || AZ_TYPE_IS_REFERENCE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (!AZ_TYPE_IS_BOXED(AZ_IMPL_TYPE(impl)), NULL);
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	/* The implementation may live in another (immortal) class - e.g. delegated through
	 * a reference-of - so only the instance side is checked: its lifetime is bound to
	 * the container, which the box owns. */
	/* instance_size is 0 for variable-sized types - the containment is not checkable there */
	arikkei_return_val_if_fail (!klass->instance_size || (((const char *) if_inst >= (const char *) inst) && ((const char *) if_inst < (const char *) inst + klass->instance_size)), NULL);
#endif
	AZBoxedInterface *boxed = boxed_iface_container_new(impl);
	az_value_set_from_inst (impl, &boxed->val, inst);
	boxed->impl = if_impl;
	if (AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl))) {
		/* The given view points into the source value; relocate it to the box's copy */
		ptrdiff_t delta = (const char *) if_inst - (const char *) inst;
		boxed->inst = (char *) &boxed->val + delta;
	} else {
		boxed->inst = if_inst;
	}
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_value (const AZImplementation *impl, const AZValue *val, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	/* Law B: only value and reference containers own a lifecycle */
	arikkei_return_val_if_fail (AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl)) || AZ_TYPE_IS_REFERENCE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (!AZ_TYPE_IS_BOXED(AZ_IMPL_TYPE(impl)), NULL);
	void *sub_inst;
	const AZImplementation *sub_impl = az_instance_get_interface (impl, az_value_get_inst(impl, val), type, &sub_inst);
	arikkei_return_val_if_fail(sub_impl != NULL, NULL);
	AZBoxedInterface *boxed = boxed_iface_container_new(impl);
	az_value_copy(impl, &boxed->val, val);
	boxed->impl = az_instance_get_interface (impl, az_value_get_inst(impl, &boxed->val), type, &boxed->inst);
	return boxed;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_value_autobox (const AZImplementation *impl, const AZValue *val, unsigned int type)
{
	/* Single decision table lives in az_instance_get_interface_autobox */
	void *if_inst;
	const AZImplementation *if_impl = az_instance_get_interface_autobox (impl, az_value_get_inst (impl, val), type, &if_inst);
	if (!if_impl) return NULL;
	/* Not boxable (plain block container or naked interface) - no lifecycle to own */
	if (AZ_IMPL_TYPE(if_impl) != AZ_TYPE_BOXED_INTERFACE) return NULL;
	return (AZBoxedInterface *) if_inst;
}

AZBoxedInterface *
az_boxed_interface_new_from_impl_instance (const AZImplementation *impl, void *inst, unsigned int type)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	/* Law B: only value and reference containers own a lifecycle */
	arikkei_return_val_if_fail (AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl)) || AZ_TYPE_IS_REFERENCE(AZ_IMPL_TYPE(impl)), NULL);
	arikkei_return_val_if_fail (!AZ_TYPE_IS_BOXED(AZ_IMPL_TYPE(impl)), NULL);
	void *sub_inst;
	const AZImplementation *sub_impl = az_instance_get_interface (impl, inst, type, &sub_inst);
	arikkei_return_val_if_fail(sub_impl != NULL, NULL);
	AZBoxedInterface *boxed = boxed_iface_container_new(impl);
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

const AZImplementation *
az_instance_get_interface_autobox(const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	if (!AZ_IMPL_IS_CLASS(impl)) {
		/* Naked interface, no boxing possible, return naked (sub)interface */
		return az_instance_get_interface(impl, inst, if_type, if_inst);
	}
	if (impl == &AZBoxedInterfaceKlass.klass.impl) {
		/* Boxed interface, get naked (sub)interface of contained instance */
		void *sub_inst;
		const AZImplementation *sub_impl = az_instance_get_interface(impl, inst, if_type, &sub_inst);
		if (!sub_impl) return NULL;
		/* Interface is actually there */
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		if ((sub_impl == boxed->impl) && (sub_inst == boxed->inst)) {
			/* The same as boxed, return the same box */
			az_boxed_interface_ref(boxed);
			*if_inst = inst;
			return &AZBoxedInterfaceKlass.klass.impl;
		} else {
			/* Sub-interface */
			if (AZ_IMPL_IS_VALUE(boxed->val_impl)) {
				/* Container is value, sub_inst points to existing instance, need pointer magic */
				AZBoxedInterface *new_box = boxed_iface_container_new(boxed->val_impl);
				az_value_copy(boxed->val_impl, &new_box->val, &boxed->val);
				new_box->impl = sub_impl;
				ptrdiff_t delta = (char *) sub_inst - (char *) &boxed->val;
				new_box->inst = (char *) &new_box->val + delta;
				*if_inst = new_box;
				return &AZBoxedInterfaceKlass.klass.impl;
			} else {
				/* Container is reference (shared), sub_inst is valid */
				AZBoxedInterface *new_box = boxed_iface_container_new(boxed->val_impl);
				az_value_copy(boxed->val_impl, &new_box->val, &boxed->val);
				new_box->impl = sub_impl;
				new_box->inst = sub_inst;
				*if_inst = new_box;
				return &AZBoxedInterfaceKlass.klass.impl;
			}
		}
	}
	if (impl == &AZBoxedValueKlass.klass.impl) {
		/* Boxed value, unbox first */
		AZBoxedValue *boxed_val = (AZBoxedValue *) inst;
		impl = &boxed_val->klass->impl;
		inst = &boxed_val->val;
	}
	if (!AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(impl)) && !AZ_TYPE_IS_REFERENCE(AZ_IMPL_TYPE(impl))) {
		/* Plain block - the lifetime is caller-managed (like a naked interface),
		 * so a box would promise ownership it cannot deliver (Law B) */
		return az_instance_get_interface(impl, inst, if_type, if_inst);
	}
	AZBoxedInterface *boxed = az_boxed_interface_new_from_impl_instance(impl, inst, if_type);
	if (!boxed) return NULL;
	*if_inst = boxed;
	return &AZBoxedInterfaceKlass.klass.impl;
}

const AZImplementation *
az_value_set_interface_autobox (const AZImplementation *impl, AZValue *dst, void *inst, unsigned int type)
{
	void *if_inst;
	const AZImplementation *if_impl = az_instance_get_interface_autobox (impl, inst, type, &if_inst);
	if (!if_impl) return NULL;
	if (AZ_IMPL_TYPE(if_impl) == AZ_TYPE_BOXED_INTERFACE) {
		/* Box owned - transfer the reference into the value */
		az_value_transfer_reference (dst, &((AZBoxedInterface *) if_inst)->reference);
	} else {
		/* Naked (plain block container or naked interface) - store the borrowed view */
		az_value_set_from_inst (if_impl, dst, if_inst);
	}
	return if_impl;
}