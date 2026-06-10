#ifndef __AZ_OS_OUTPUT_STREAM_H__
#define __AZ_OS_OUTPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#define AZ_TYPE_OS_OUTPUT_STREAM az_os_output_stream_get_type()

typedef struct _AZOSOutputStream AZOSOutputStream;
typedef struct _AZOSOutputStreamClass AZOSOutputStreamClass;

#include <stdio.h>
#include <stdint.h>

#include <az/io/output-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZOSOutputStream {
	FILE *file;
};

struct _AZOSOutputStreamClass {
	AZClass klass;
	AZOutputStreamImplementation ostream_impl;
};

unsigned int az_os_output_stream_get_type (void);

#ifdef __cplusplus
};
#endif

#endif