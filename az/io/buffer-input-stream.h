#ifndef __AZ_BUFFER_INPUT_STREAM_H__
#define __AZ_BUFFER_INPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

 #define AZ_TYPE_BUFFER_INPUT_STREAM az_buffer_input_stream_get_type()

typedef struct _AZBufferInputStream AZBufferInputStream;
typedef struct _AZBufferInputStreamClass AZBufferInputStreamClass;

#include <stdint.h>

#include <az/io/input-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A lightweight value type reading from fixed buffer
 * 
 * As it is a value type, the copies are not synchronized
 * 
 */

struct _AZBufferInputStream {
	const uint8_t *buffer;
	uint64_t size;
	uint64_t pos;
};

struct _AZBufferInputStreamClass {
	AZClass klass;
	AZInputStreamImplementation istream_impl;
};

unsigned int az_buffer_input_stream_get_type (void);

#ifdef __cplusplus
};
#endif

#endif