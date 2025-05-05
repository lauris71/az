#define __AZ_FIELD_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <assert.h>
#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/string.h>
#include <az/field.h>
#include <az/packed-value.h>
#include <az/private.h>

static AZClass *field_class = NULL;

void
az_init_field_class (void)
{
	field_class = az_class_new_with_type (AZ_TYPE_FIELD, AZ_TYPE_BLOCK, sizeof (AZClass), sizeof (AZField), AZ_FLAG_FINAL, (const uint8_t *) "field");
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
	arikkei_return_if_fail (!impl || (az_type_is_assignable_to (impl->type, type)));
#endif
	prop->key = az_string_new (key);
	prop->type = type;
	prop->offset = offset;
	prop->is_reference = az_type_is_a (type, AZ_TYPE_REFERENCE);
	prop->is_interface = az_type_is_a (type, AZ_TYPE_INTERFACE);
	prop->is_function = az_type_is_a (type, AZ_TYPE_FUNCTION);
	prop->is_final = is_final;
	prop->spec = spec;
	prop->read = read;
	prop->write = write;
	if (impl) {
		prop->val.impl = impl;
		az_set_value_from_instance (impl, &prop->val.v.value, inst);
	}
}

void az_field_setup_function (AZField *field, const unsigned char *key, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig,
	const AZImplementation *impl, void *inst)
{
	az_field_setup (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, offset, impl, inst);
	field->signature = sig;
}
