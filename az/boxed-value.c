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
boxed_value_to_string (const AZImplementation *impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZBoxedValue *boxed = (AZBoxedValue *) inst;
	AZClass *klass = AZ_CLASS_FROM_IMPL(boxed->val.impl);
	unsigned int pos;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Boxed ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, klass->name);
	if (pos < len) buf[pos] = 0;
	return pos;
}

void
az_init_boxed_value_class (void)
{
	az_boxed_value_class = (AZBoxedValueClass *) az_class_new_with_type (AZ_TYPE_BOXED_VALUE, AZ_TYPE_REFERENCE, sizeof (AZBoxedValueClass), 0, AZ_FLAG_FINAL, (const uint8_t *) "boxed value");
	az_boxed_value_class->ref_class.klass.to_string = boxed_value_to_string;
}

AZBoxedValue *
az_boxed_value_new (const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
    val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + val_size);
	az_instance_init (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_instance (&boxed->val, impl, inst);
	return boxed;
}

AZBoxedValue *
az_boxed_value_new_from_impl_value (const AZImplementation *impl, const AZValue *val)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
    val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + val_size);
	az_instance_init (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_value (&boxed->val, impl, val);
	return boxed;
}

AZBoxedValue *
az_boxed_value_new_from_impl_instance (const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	unsigned int val_size = AZ_TYPE_VALUE_SIZE(AZ_IMPL_TYPE(impl));
    val_size = (val_size > 16) ? val_size - 16 : 0;
	AZBoxedValue *boxed = (AZBoxedValue *) malloc (sizeof (AZBoxedValue) + val_size);
	az_instance_init (boxed, AZ_TYPE_BOXED_VALUE);
	boxed->val.impl = NULL;
	az_packed_value_set_from_impl_instance (&boxed->val, impl, inst);
	return boxed;
}
