#ifndef __AZ_BUFFER_OUTPUT_STREAM_H__
#define __AZ_BUFFER_OUTPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

 #define AZ_TYPE_BUFFER_OUTPUT_STREAM az_buffer_output_stream_get_type()

typedef struct _AZBufferOutputStream AZBufferOutputStream;
typedef struct _AZBufferOutputStreamClass AZBufferOutputStreamClass;

#include <stdint.h>

#include <az/io/output-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A lightweight block type writing into dynamically growing buffer
 */

struct _AZBufferOutputStream {
	uint8_t *buffer;
	uint64_t allocated;
	uint64_t pos;
};

struct _AZBufferOutputStreamClass {
	AZClass klass;
	AZOutputStreamImplementation ostream_impl;
};

unsigned int az_buffer_output_stream_get_type (void);

#ifdef __cplusplus
};
#endif

#endif