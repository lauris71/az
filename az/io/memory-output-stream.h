#ifndef __AZ_MEMORY_OUTPUT_STREAM_H__
#define __AZ_MEMORY_OUTPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

 #define AZ_TYPE_MEMORY_OUTPUT_STREAM az_memory_output_stream_get_type()

typedef struct _AZMemoryOutputStream AZMemoryOutputStream;
typedef struct _AZMemoryOutputStreamClass AZMemoryOutputStreamClass;

#include <stdint.h>

#include <az/io/output-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A lightweight value type writing into dynamically growing buffer
 * 
 * As it is a value type, the copies are not synchronized
 */

struct _AZMemoryOutputStream {
	uint8_t *buffer;
	uint64_t allocated;
	uint64_t pos;
};

struct _AZMemoryOutputStreamClass {
	AZClass klass;
	AZOutputStreamImplementation ostream_impl;
};

unsigned int az_memory_output_stream_get_type (void);

#ifdef __cplusplus
};
#endif

#endif