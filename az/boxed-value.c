#define __AZ_BOXED_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2025
*/

#include <arikkei/arikkei-strlib.h>

#include <az/boxed-value.h>
#include <az/private.h>

static unsigned int
serialize_boxed_value (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx) {
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	return az_instance_serialize(&boxed->klass->impl, az_instance_from_value(&boxed->klass->impl, &boxed->val), d, dlen, ctx);
}

static unsigned int
boxed_value_to_string (const AZImplementation *impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	unsigned int pos;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Boxed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, boxed->klass->name);
	if (pos < len) buf[pos] = 0;
	return pos;
}

AZBoxedValueClass AZBoxedValueKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BOXED_VALUE},
	&AZReferenceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "boxed value",
	7, sizeof(AZBoxedValueClass), 0,
	NULL,
	NULL, NULL,
	serialize_boxed_value, NULL, boxed_value_to_string,
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
	az_instance_init (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_value_set_from_impl_instance (&boxed->val, &klass->impl, inst);
	return boxed;
}

AZBoxedValue *
az_boxed_value_new_from_val (const AZClass *klass, const AZValue *val)
{
	arikkei_return_val_if_fail (klass != NULL, NULL);
	arikkei_return_val_if_fail (AZ_CLASS_IS_VALUE(klass), NULL);
	unsigned int ext_size = (klass->instance_size > 16) ? klass->instance_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + ext_size);
	az_instance_init (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->klass = klass;
	az_value_set_from_impl_value (&boxed->val, &klass->impl, val);
	return boxed;
}
