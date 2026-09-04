#define __AZ_BOXED_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2025
*/

#define DEBUG_BOXED_VALUE

#include <arikkei/arikkei-strlib.h>

#include <az/boxed-value.h>
#include <az/instance.h>
#include <az/private.h>

static unsigned int
serialize_boxed_value (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx) {
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_serialize(&boxed->klass->impl, &boxed->val, d, dlen, ctx);
}

static unsigned int
boxed_value_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen)
{
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_to_string(&boxed->klass->impl, &boxed->val, d, dlen);
}

#ifdef DEBUG_BOXED_VALUE
static void
boxed_value_init (AZBoxedValueClass *klass, AZBoxedValue *boxed)
{
	fprintf (stderr, "boxed value init\n");
}

static void
boxed_value_finalize (AZBoxedValueClass *klass, AZBoxedValue *boxed)
{
	fprintf (stderr, "boxed value finalize\n");
}
#endif

AZ_CLASS_ALIGN AZBoxedValueClass AZBoxedValueKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BOXED_VALUE },
		.parent = &AZReferenceKlass.klass,
		.name = (const uint8_t *) "boxed value",
		.alignment = 7,
		.class_size = sizeof(AZBoxedValueClass),
		.instance_size = 0,
		/* instance_init, instance_finalize */
#ifdef DEBUG_BOXED_VALUE
		.instance_init = (void (*) (const AZImplementation *, void *)) boxed_value_init,
		.instance_finalize = (void (*) (const AZImplementation *, void *)) boxed_value_finalize,
#endif
		.serialize = serialize_boxed_value,
		.to_string = boxed_value_to_string
	},
	.drop = NULL,
	.dispose = NULL
};

/* The box answers interface and property queries on behalf of the boxed content */
static const AZImplementation *
boxed_value_delegate_get_interface (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	/* Type-level queries (inst == NULL) cannot see through the box: the content is per-instance */
	if (!inst) return NULL;
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_get_interface (&boxed->klass->impl, &boxed->val, if_type, if_inst);
}

static int
boxed_value_delegate_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	if (!inst) return -1;
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_class_lookup_property (boxed->klass, &boxed->klass->impl, &boxed->val, key, def_class, sub_impl, sub_inst);
}

static int
boxed_value_delegate_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst)
{
	if (!inst) return -1;
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_class_lookup_function (boxed->klass, &boxed->klass->impl, &boxed->val, key, sig, def_class, sub_impl, sub_inst);
}

static unsigned int
boxed_value_delegate_foreach_property (const AZClass *klass, const AZImplementation *impl, void *inst, AZPropertyForeachFunc cb, void *data)
{
	if (!inst) return 1;
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_foreach_property (&boxed->klass->impl, &boxed->val, cb, data);
}

static unsigned int
boxed_value_delegate_foreach_interface (const AZClass *klass, const AZImplementation *impl, void *inst, AZInterfaceForeachFunc cb, void *data)
{
	if (!inst) return 1;
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_foreach_interface (&boxed->klass->impl, &boxed->val, cb, data);
}

static const AZClassDelegate az_boxed_value_delegate = {
	boxed_value_delegate_get_interface,
	boxed_value_delegate_lookup_property,
	boxed_value_delegate_lookup_function,
	boxed_value_delegate_foreach_property,
	boxed_value_delegate_foreach_interface
};

void
az_init_boxed_value_class (void)
{
	AZBoxedValueKlass.klass.delegate_idx = az_class_register_delegate (&az_boxed_value_delegate);
	az_class_new_with_value(&AZBoxedValueKlass.klass);
}

AZBoxedValue *
az_boxed_value_new(const AZClass *klass)
{
	arikkei_return_val_if_fail (klass != NULL, NULL);
	arikkei_return_val_if_fail (AZ_CLASS_IS_VALUE(klass), NULL);
	/* Boxed values only hold big values (fundamental types are never boxed) */
	arikkei_return_val_if_fail (klass->instance_size > AZ_VALUE_MAX_SIZE, NULL);
	unsigned int ext_size = (klass->instance_size > 16) ? klass->instance_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + ext_size);
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_instance_init(&klass->impl, &boxed->val);
	return boxed;
}

AZBoxedValue *
az_boxed_value_new_from_inst (const AZClass *klass, void *inst)
{
	arikkei_return_val_if_fail (klass != NULL, NULL);
	arikkei_return_val_if_fail (AZ_CLASS_IS_VALUE(klass), NULL);
	/* Boxed values only hold big values (fundamental types are never boxed) */
	arikkei_return_val_if_fail (klass->instance_size > AZ_VALUE_MAX_SIZE, NULL);
	unsigned int ext_size = (klass->instance_size > 16) ? klass->instance_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + ext_size);
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_value_set_from_inst (&klass->impl, &boxed->val, inst);
	return boxed;
}

AZBoxedValue *
az_boxed_value_new_from_val (const AZClass *klass, const AZValue *val)
{
	arikkei_return_val_if_fail (klass != NULL, NULL);
	arikkei_return_val_if_fail (AZ_CLASS_IS_VALUE(klass), NULL);
	/* Boxed values only hold big values (fundamental types are never boxed) */
	arikkei_return_val_if_fail (klass->instance_size > AZ_VALUE_MAX_SIZE, NULL);
	unsigned int ext_size = (klass->instance_size > 16) ? klass->instance_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + ext_size);
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_value_copy (&klass->impl, &boxed->val, val);
	return boxed;
}

const AZImplementation *
az_value_init_autobox(const AZImplementation *impl, AZValue *dst, unsigned int size)
{
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (AZ_CLASS_IS_VALUE(klass)) {
			if (klass->instance_size > size) {
				/* Value type that does not fit into dst, box */
				dst->block = az_boxed_value_new_from_val(klass, NULL);
				impl = AZ_BOXED_VALUE_IMPL;
			} else {
				az_instance_init(impl, dst);
			}
		} else {
			dst->block = NULL;
		}
	} else {
		/* Untyped null */
		az_value_set_null (dst);
	}
	return impl;
}

const AZImplementation *
az_value_transfer_autobox(const AZImplementation *impl, AZValue *dst, AZValue *src, unsigned int size)
{
	if (dst == src) {
		/* In-place size normalization: the result obeys the size argument
		 * (all structs <= size are in plain form, bigger ones are boxed) */
		if (impl) {
			AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
			if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
				/* Big value does not fit - box it in place */
				dst->block = az_boxed_value_new_from_val (klass, src);
				return AZ_BOXED_VALUE_IMPL;
			}
			if ((klass == (AZClass *) &AZBoxedValueKlass) && (((AZBoxedValue *) src->block)->klass->instance_size <= size)) {
				/* Boxed content that fits - unbox to plain form in place */
				AZBoxedValue *boxed = (AZBoxedValue *) src->block;
				const AZImplementation *content_impl = &boxed->klass->impl;
				az_value_copy (content_impl, dst, &boxed->val);
				az_boxed_value_unref (boxed);
				return content_impl;
			}
		}
		/* Self-transfer is otherwise a no-op */
		return impl;
	}
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			// Value type that does not fit into dst, box
			dst->block = az_boxed_value_new_from_val(klass, src);
			/* Clear the source value with its own implementation */
			az_value_clear(&klass->impl, src);
			impl = AZ_BOXED_VALUE_IMPL;
		} else if ((klass == (AZClass *) &AZBoxedValueKlass) && (((AZBoxedValue *) src->block)->klass->instance_size <= size)) {
			// Boxed value that fits into dst, unbox
			AZBoxedValue *boxed = (AZBoxedValue *) src->block;
			impl = &boxed->klass->impl;
			az_value_copy(impl, dst, &boxed->val);
			/* Release the box (the content was moved out) */
			az_boxed_value_unref (boxed);
		} else {
			az_value_transfer(impl, dst, src);
		}
	} else {
		/* Untyped null */
		az_value_set_null (dst);
	}
	return impl;
}

const AZImplementation *
az_value_copy_autobox(const AZImplementation *impl, AZValue *dst, const AZValue *src, unsigned int size)
{
	/* Self-copy is a caller error (a copy must leave src intact) - warn and no-op */
	arikkei_return_val_if_fail (dst != src, impl);
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (impl && AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			// Value type that does not fit into dst, box
			dst->block = az_boxed_value_new_from_val(klass, src);
			impl = AZ_BOXED_VALUE_IMPL;
		} else if ((klass == (AZClass *) &AZBoxedValueKlass) && (((AZBoxedValue *) src->block)->klass->instance_size <= size)) {
			// Boxed value that fits into dst, unbox
			AZBoxedValue *boxed = (AZBoxedValue *) src->block;
			impl = &boxed->klass->impl;
			az_value_copy(impl, dst, &boxed->val);
		} else {
			az_value_copy(impl, dst, src);
		}
	} else {
		/* Untyped null */
		az_value_set_null (dst);
	}
	return impl;
}

const AZImplementation *
az_value_set_from_inst_autobox(const AZImplementation *impl, AZValue *dst, unsigned int size, void *inst)
{
	if (impl) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
		if (AZ_CLASS_IS_VALUE(klass) && (klass->instance_size > size)) {
			dst->block = az_boxed_value_new_from_inst(klass, inst);
			impl = AZ_BOXED_VALUE_IMPL;
		} else {
			az_value_set_from_inst(impl, dst, inst);
		}
	} else {
		/* Untyped null */
		az_value_set_null (dst);
	}
	return impl;
}

const AZImplementation *
az_value_get_inst_autobox (const AZImplementation *impl, const AZValue *val, void **inst)
{
	if (impl == (AZImplementation *) &AZBoxedValueKlass) {
		*inst = &((AZBoxedValue *) val->block)->val;
		impl = &((AZBoxedValue *) val->block)->klass->impl;
	} else if (impl && (AZ_IMPL_IS_BLOCK(impl))) {
		*inst = val->block;
	} else {
		*inst = (void *) val;
	}
	return impl;
}

unsigned int
az_value_equals_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const AZValue *rhs)
{
	if (lhs_impl == &AZBoxedValueKlass.klass.impl) {
		AZBoxedValue *boxed = (AZBoxedValue *) lhs->block;
		lhs_impl = &boxed->klass->impl;
		lhs = &boxed->val;
	}
	if (rhs_impl == &AZBoxedValueKlass.klass.impl) {
		AZBoxedValue *boxed = (AZBoxedValue *) rhs->block;
		rhs_impl = &boxed->klass->impl;
		rhs = &boxed->val;
	}
	if (lhs_impl != rhs_impl) return 0;
	if (AZ_IMPL_IS_BLOCK(lhs_impl)) {
		return lhs->block == rhs->block;
	} else if (AZ_IMPL_IS_VALUE(lhs_impl)) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(lhs_impl);
		if (klass->instance_size) return !memcmp (lhs, rhs, klass->instance_size);
	}
	return 0;
}

unsigned int
az_value_equals_instance_autobox (const AZImplementation *lhs_impl, const AZValue *lhs, const AZImplementation *rhs_impl, const void *rhs)
{
	if (lhs_impl == &AZBoxedValueKlass.klass.impl) {
		AZBoxedValue *boxed = (AZBoxedValue *) lhs->block;
		lhs_impl = &boxed->klass->impl;
		lhs = &boxed->val;
	}
	if (lhs_impl != rhs_impl) return 0;
	if (AZ_IMPL_IS_BLOCK(lhs_impl)) {
		return lhs->block == rhs;
	} else if (AZ_IMPL_IS_VALUE(lhs_impl)) {
		AZClass *klass = AZ_CLASS_FROM_IMPL(lhs_impl);
		if (klass->instance_size) return !memcmp (lhs, rhs, klass->instance_size);
	}
	return 0;
}
