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

AZInterfaceClass AZOutputStreamKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_IDX_OUTPUT_STREAM},
	&AZInterfaceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "output stream",
	3, sizeof(AZInterfaceKlass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL},
	sizeof(AZOutputStreamImplementation), NULL
};

void
az_init_output_stream_class(void)
{
    az_class_new_with_value(&AZOutputStreamKlass.klass);
}
