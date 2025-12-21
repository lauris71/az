#define __AZ_FIELD_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <assert.h>
#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/string.h>
#include <az/field.h>
#include <az/packed-value.h>
#include <az/private.h>

AZClass AZFieldKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_FIELD},
	&AZBlockClass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "field",
	7, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, NULL,
	NULL, NULL
};

void
az_init_field_class (void)
{
	az_class_new_with_value(&AZFieldKlass);
	//field_class = az_class_new_with_type (AZ_TYPE_FIELD, AZ_TYPE_BLOCK, sizeof (AZClass), sizeof (AZField), AZ_FLAG_FINAL, (const uint8_t *) "field");
}

void
az_field_setup (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (prop != NULL);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail (!impl || (az_type_is_assignable_to(AZ_IMPL_TYPE(impl), type)));
	arikkei_return_if_fail (!offset || !impl);
#endif
	prop->key = az_string_new (key);
	prop->type = type;
	prop->is_reference = az_type_is_a (type, AZ_TYPE_REFERENCE);
	prop->is_interface = az_type_is_a (type, AZ_TYPE_INTERFACE);
	prop->is_function = az_type_is_a (type, AZ_TYPE_FUNCTION);
	prop->is_final = is_final;
	prop->spec = spec;
	prop->read = read;
	prop->write = write;
	if (offset) {
		prop->offset = offset;
	} else if (impl) {
		unsigned int size = sizeof(AZPackedValue);
		AZClass *val_class = AZ_CLASS_FROM_IMPL(impl);
		if (az_class_value_size(val_class) > 16) {
			size += (az_class_value_size(val_class) - 16);
		}
		prop->value = (AZPackedValue *) malloc(size);
		prop->value->impl = impl;
		az_value_set_from_inst (impl, &prop->value->v, inst);
	} else {
		prop->value = NULL;
	}
}

void az_field_setup_function (AZField *field, const unsigned char *key, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, const AZFunctionSignature *sig,
	const AZImplementation *impl, void *inst)
{
	az_field_setup (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, 0, impl, inst);
	field->signature = sig;
}

void az_field_setup_function_packed (AZField *field, const unsigned char *key, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, unsigned int offset)
{
	az_field_setup (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, offset, NULL, NULL);
	field->signature = sig;
}
