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

#ifdef __cplusplus
extern "C" {
#endif

struct _AZInputStreamImplementation {
	AZImplementation impl;
	int64_t (*read) (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size);
	int64_t (*seek) (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset);
	int64_t (*skip) (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes);
	int (*is_eof) (const AZInputStreamImplementation *impl, AZInputStream *inst);
	int (*is_error) (const AZInputStreamImplementation *impl, AZInputStream *inst);
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
az_input_stream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return (impl->is_eof) ? impl->is_eof (impl, inst) : 0;
}

static inline int
az_input_stream_is_error (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return (impl->is_error) ? impl->is_error (impl, inst) : 0;
}

int64_t az_input_stream_read_all (const AZInputStreamImplementation *impl, AZInputStream *inst, uint8_t **data);

#ifdef __cplusplus
};
#endif

#endif