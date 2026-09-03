#define __AZ_BASE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/private.h>

void
az_init (void)
{
	static unsigned int initialized = 0;
	if (initialized) return;
	initialized = 1;
	az_globals_init();
	az_init_primitive_classes();
	az_init_base_classes();

	az_init_interface_class();
	az_init_field_class();
	az_init_function_classes();
	az_init_reference_class();
	az_init_string_class();
	az_init_boxed_value_class();
	az_init_boxed_interface_class();
	az_init_packed_value_class();
	az_init_object_class();
	az_init_output_stream_class();
	az_init_input_stream_class();

	az_post_init_primitive_classes();
	az_post_init_base_classes();
}

static unsigned int
impl_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZImplementation *inst_impl = (AZImplementation *) inst;
	AZClass *inst_class = AZ_CLASS_FROM_IMPL(inst_impl);
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, inst_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " implementation");
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

static unsigned int
class_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZClass *inst_class = (AZClass *) inst;
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, inst_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " class");
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

AZ_CLASS_ALIGN AZClass AZStructKlass = {
	.impl = { .flags = AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_STRUCT },
	.parent = &AZAnyKlass,
	.name = (const uint8_t *) "struct",
	.alignment = 3,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = az_any_to_string
};

AZ_CLASS_ALIGN AZClass AZBlockKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BLOCK },
	.parent = &AZAnyKlass,
	.name = (const uint8_t *) "block",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = az_any_to_string
};

AZ_CLASS_ALIGN AZClass AZImplKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_IMPLEMENTATION },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "implementation",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = sizeof(AZImplementation),
	.to_string = impl_to_string
};

AZ_CLASS_ALIGN AZClass AZClassKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_CLASS },
	.parent = &AZImplKlass,
	.name = (const uint8_t *) "class",
	/* Class is variable size */
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = class_to_string
};

void
az_init_base_classes (void)
{
	az_class_new_with_value(&AZStructKlass);
	az_class_new_with_value(&AZBlockKlass);
	az_class_new_with_value(&AZImplKlass);
	az_class_new_with_value(&AZClassKlass);
}

void
az_post_init_base_classes (void)
{
    az_class_post_init(&AZStructKlass);
    az_class_post_init(&AZBlockKlass);
    az_impl_class_post_init();
    az_class_post_init(&AZImplKlass);
    az_class_post_init(&AZClassKlass);
}
