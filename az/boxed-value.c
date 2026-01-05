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

AZBoxedValueClass AZBoxedValueKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BOXED_VALUE},
	&AZReferenceKlass.klass,
	0, 0, 0, 0,
	/* ifaces / ifaces_self, ifaces_all */
	{0},
	/* props_self */
	NULL,
	(const uint8_t *) "boxed value",
	7, sizeof(AZBoxedValueClass), 0,
	NULL,
	/* instance_init, instance_finalize */
#ifdef DEBUG_BOXED_VALUE
	(void (*) (const AZImplementation *, void *)) boxed_value_init, (void (*) (const AZImplementation *, void *)) boxed_value_finalize,
#else
	NULL, NULL,
#endif
	serialize_boxed_value, NULL, boxed_value_to_string,
	/* get_property, set_property */
	NULL, NULL},
	NULL, NULL
};

void
az_init_boxed_value_class (void)
{
	az_class_new_with_value(&AZBoxedValueKlass.klass);
}

AZBoxedValue *
az_boxed_value_new (const AZClass *klass, void *inst)
{
	arikkei_return_val_if_fail (klass != NULL, NULL);
	arikkei_return_val_if_fail (AZ_CLASS_IS_VALUE(klass), NULL);
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
	unsigned int ext_size = (klass->instance_size > 16) ? klass->instance_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + ext_size);
	az_instance_init_by_type (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_value_copy (&klass->impl, &boxed->val, val);
	return boxed;
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
