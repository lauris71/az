#define __AZ_INPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/base.h>
#include <az/types.h>
#include <az/private.h>

#include <az/io/input-stream.h>

AZInterfaceClass AZInputStreamKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_IDX_OUTPUT_STREAM},
	&AZInterfaceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "input stream",
	3, sizeof(AZInterfaceKlass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL},
	sizeof(AZInputStreamImplementation), NULL
};

void
az_init_input_stream_class(void)
{
    az_class_new_with_value(&AZInputStreamKlass.klass);
}

int64_t
az_input_stream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes)
{
	if (impl->skip) return impl->skip (impl, inst, n_bytes);
	while (n_bytes) {
		uint8_t buf[1024];
		uint64_t n = n_bytes;
		if (n > sizeof (buf)) n = sizeof (buf);
		n = az_input_stream_read (impl, inst, buf, n);
		if (n <= 0) return n;
		n_bytes -= n;
	}
	return AZ_OK;
}

#define COPY_BUFFER_SIZE 4096

int64_t
az_input_stream_read_all (const AZInputStreamImplementation *impl, AZInputStream *inst, uint8_t **data)
{
	uint64_t allocated = COPY_BUFFER_SIZE;
	uint64_t total = 0;
	uint8_t *buf = (uint8_t *) malloc (allocated);
	if (!buf) return AZ_OUT_OF_MEMORY;

	while (1) {
		if (total == allocated) {
			allocated *= 2;
			uint8_t *nbuf = (uint8_t *) realloc (buf, allocated);
			if (!nbuf) {
				free (buf);
				return AZ_OUT_OF_MEMORY;
			}
			buf = nbuf;
		}
		int64_t nread = az_input_stream_read (impl, inst, buf + total, allocated - total);
		if (nread < 0) {
			free (buf);
			return nread;
		}
		if (nread == 0) break;
		total += nread;
	}
	*data = buf;
	return (int64_t) total;
}
