#ifndef __AZ_INPUT_STREAM_H__
#define __AZ_INPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

typedef struct _AZInputStreamImplementation AZInputStreamImplementation;
typedef struct _AZInterfaceClass AZInputStreamClass;

#include <stdint.h>
#include <stdio.h>

#include <az/interface.h>
#include <az/io/stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Data transfers (read, skip) are blocking and return the transferred count,
 * 0 <= n <= requested, never an error code. If n < requested, state() is not
 * AZ_STREAM_STATE_OK and reports the reason (see az/io/stream.h).
 * seek and read_all return >= 0 values or a negative AZErrorCode.
 */
struct _AZInputStreamImplementation {
	AZImplementation impl;
	int64_t (*read) (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size);
	int64_t (*seek) (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset);
	int64_t (*skip) (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes);
	int (*state) (const AZInputStreamImplementation *impl, AZInputStream *inst);
};

static inline int64_t
az_input_stream_read (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size)
{
	return impl->read (impl, inst, data, size);
}

static inline int64_t
az_input_stream_seek (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset)
{
	return (impl->seek) ? impl->seek (impl, inst, offset) : AZ_NOT_IMPLEMENTED;
}

int64_t az_input_stream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes);

static inline int
az_input_stream_state (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return (impl->state) ? impl->state (impl, inst) : AZ_STREAM_STATE_OK;
}

static inline int
az_input_stream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return az_input_stream_state (impl, inst) == AZ_STREAM_STATE_EOF;
}

/**
 * @brief Read the stream to the end of data
 *
 * Reaching the end of data is a success. Returns the total number of bytes
 * read (>= 0) or a negative error code.
 */
int64_t az_input_stream_read_all (const AZInputStreamImplementation *impl, AZInputStream *inst, uint8_t **data);

#ifdef __cplusplus
};
#endif

#endif
