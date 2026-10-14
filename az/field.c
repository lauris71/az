#define __AZ_FIELD_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <assert.h>

#include <arikkei/arikkei-utils.h>
#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/string.h>
#include <az/field.h>
#include <az/packed-value.h>
#include <az/private.h>
#include <az/extend.h>

void az_field_setup_value (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, unsigned int offset)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (prop != NULL);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail ((read == AZ_FIELD_READ_VALUE) || (read == AZ_FIELD_READ_INSTANCE) || (read == AZ_FIELD_READ_PACKED));
	arikkei_return_if_fail ((write == (read << 3)) || (write == AZ_FIELD_WRITE_NONE) || (write == AZ_FIELD_WRITE_METHOD));
#endif
	prop->key = az_string_new (key);
	prop->type = type;
	if (is_final) prop->flags |= AZ_FIELD_FLAG_FINAL;
	prop->flags |= spec;
	prop->flags |= read;
	prop->flags |= write;
	prop->offset = offset;
	/* Masked (bit-field) values default to plain access */
	prop->value_type_idx = 0;
	prop->shift = 0;
	prop->mask_width = 0;
	prop->bits = 0;
}

void az_field_setup_stored (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (prop != NULL);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail (!impl || (az_type_is_assignable_to(AZ_IMPL_TYPE(impl), type)));
	arikkei_return_if_fail (read == AZ_FIELD_READ_STORED_STATIC);
	arikkei_return_if_fail (write == AZ_FIELD_WRITE_NONE);
#endif
	prop->key = az_string_new (key);
	prop->type = type;
	if (is_final) prop->flags |= AZ_FIELD_FLAG_FINAL;
	prop->flags |= spec;
	prop->flags |= read;
	prop->flags |= write;
	prop->value.impl = NULL;
	if (impl) {
		prop->value.impl = az_value_set_from_inst_autobox(impl, &prop->value.v, AZ_VALUE_MAX_SIZE, inst);
	}
}

void az_field_setup_method (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (prop != NULL);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail (read == AZ_FIELD_READ_METHOD);
	arikkei_return_if_fail ((write == AZ_FIELD_WRITE_NONE) || (write == AZ_FIELD_WRITE_METHOD));
#endif
	prop->key = az_string_new (key);
	prop->type = type;
	if (is_final) prop->flags |= AZ_FIELD_FLAG_FINAL;
	prop->flags |= spec;
	prop->flags |= read;
	prop->flags |= write;
}

void az_field_setup_function (AZField *field, const unsigned char *key, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, const AZFunctionSignature *sig,
	const AZImplementation *impl, void *inst)
{
	if ((read == AZ_FIELD_READ_VALUE) || (read == AZ_FIELD_READ_PACKED)) {
		// fixme: Cannot happen?
		az_field_setup_value (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, 0);
	} else if (read == AZ_FIELD_READ_METHOD) {
		// fixme: Cannot happen
		az_field_setup_method (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write);
	} else {
		az_field_setup_stored (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, impl, inst);
	}
	field->signature = sig;
}

void az_field_setup_function_packed (AZField *field, const unsigned char *key, unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, unsigned int offset)
{
	if ((read == AZ_FIELD_READ_VALUE) || (read == AZ_FIELD_READ_PACKED)) {
		az_field_setup_value (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, offset);
	} else if (read == AZ_FIELD_READ_METHOD) {
		// fixme: Cannot happen
		az_field_setup_method (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write);
	} else {
		// fixme: Cannot happen
		az_field_setup_stored (field, key, AZ_TYPE_FUNCTION, is_final, spec, read, write, NULL, NULL);
	}
	field->signature = sig;
}

enum {
	FIELD_PROP_NAME,
	FIELD_NUM_PROPS
};

static unsigned int
field_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int d_len)
{
	AZField *field = (AZField *) inst;
	if (!d) d_len = 0;
	unsigned int pos = arikkei_strncpy (d, d_len, (const unsigned char *) "AZField [");
	pos += arikkei_strncpy (d + pos, (d_len > pos) ? d_len - pos : 0, field->key->str);
	pos += arikkei_strncpy (d + pos, (d_len > pos) ? d_len - pos : 0, (const unsigned char *) ": ");
	AZClass* klass = AZ_CLASS_FROM_TYPE(field->type);
	pos += arikkei_strncpy (d + pos, (d_len > pos) ? d_len - pos : 0, klass->name);
	pos += arikkei_strncpy (d + pos, (d_len > pos) ? d_len - pos : 0, (const unsigned char *) "]");
	return pos;
}

AZ_CLASS_ALIGN AZClass AZFieldKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_FIELD },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "field",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = sizeof(AZField),
	.n_props_self = FIELD_NUM_PROPS,
	.to_string = field_to_string
};

void
az_init_field_class (void)
{
	az_class_new_with_value(&AZFieldKlass);
}

void
az_construct_field_class (void)
{
	az_class_define_property_value (&AZFieldKlass, FIELD_PROP_NAME, (const uint8_t *) "name", AZ_TYPE_STRING, 1, AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET(AZField, key));
}