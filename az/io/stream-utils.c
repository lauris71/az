#define __AZ_STREAM_UTILS_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>
#include <string.h>

#include <az/io/stream-utils.h>

#define COPY_BUFFER_SIZE 4096

int64_t
az_stream_copy (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst, uint64_t size)
{
	uint8_t buf[COPY_BUFFER_SIZE];
	uint64_t remaining = size;

	while (remaining) {
		uint64_t chunk = (remaining < COPY_BUFFER_SIZE) ? remaining : COPY_BUFFER_SIZE;
		int64_t nread = az_input_stream_read (in_impl, in_inst, buf, chunk);
		if (nread <= 0) return nread;
		int64_t nwritten = az_output_stream_write (out_impl, out_inst, buf, (uint64_t) nread);
		if (nwritten < nread) return nwritten;
		remaining -= (uint64_t) nwritten;
	}
	return (int64_t) size;
}

int64_t
az_stream_copy_all (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst)
{
	uint8_t buf[COPY_BUFFER_SIZE];
	uint64_t total = 0;

	while (1) {
		int64_t nread = az_input_stream_read (in_impl, in_inst, buf, COPY_BUFFER_SIZE);
		if (nread < 0) return nread;
		if (nread == 0) break;
		int64_t nwritten = az_output_stream_write (out_impl, out_inst, buf, (uint64_t) nread);
		if (nwritten < 0) return nwritten;
		if (nwritten < nread) return AZ_IO_ERROR;
		total += nwritten;
	}
	return (int64_t) total;
}