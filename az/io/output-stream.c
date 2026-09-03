#define __AZ_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/base.h>
#include <az/types.h>
#include <az/private.h>

#include <az/io/output-stream.h>

AZ_CLASS_ALIGN AZInterfaceClass AZOutputStreamKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_OUTPUT_STREAM },
		.parent = &AZInterfaceKlass.klass,
		.name = (const uint8_t *) "output stream",
		.alignment = 3,
		.class_size = sizeof(AZInterfaceKlass),
		.instance_size = 0,
		.to_string = az_any_to_string
	},
	.implementation_size = sizeof(AZOutputStreamImplementation),
	.implementation_init = NULL
};

void
az_init_output_stream_class(void)
{
    az_class_new_with_value(&AZOutputStreamKlass.klass);
}
