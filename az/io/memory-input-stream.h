#ifndef __AZ_MEMORY_INPUT_STREAM_H__
#define __AZ_MEMORY_INPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

 #define AZ_TYPE_MEMORY_INPUT_STREAM az_memory_input_stream_get_type()

typedef struct _AZMemoryInputStream AZMemoryInputStream;
typedef struct _AZMemoryInputStreamClass AZMemoryInputStreamClass;

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

struct _AZMemoryInputStream {
	const uint8_t *buffer;
	uint64_t size;
	uint64_t pos;
};

struct _AZMemoryInputStreamClass {
	AZClass klass;
	AZInputStreamImplementation istream_impl;
};

unsigned int az_memory_input_stream_get_type (void);

#ifdef __cplusplus
};
#endif

#endif