#ifndef __AZ_STREAM_UTILS_H__
#define __AZ_STREAM_UTILS_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdint.h>

#include <az/io/input-stream.h>
#include <az/io/output-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

int64_t az_stream_copy (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst, uint64_t size);
int64_t az_stream_copy_all (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst);

#ifdef __cplusplus
};
#endif

#endif