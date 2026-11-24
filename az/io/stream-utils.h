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

/**
 * @brief Copy data between streams
 *
 * Returns the number of bytes copied (>= 0, less than @c size if a stream
 * boundary was reached - state() of the streams tells which and why) or a
 * negative error code.
 */
int64_t az_stream_copy (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst, uint64_t size);
/**
 * @brief Copy data between streams until end of input
 *
 * Returns the number of bytes copied (>= 0) or a negative error code.
 */
int64_t az_stream_copy_all (const AZInputStreamImplementation *in_impl, AZInputStream *in_inst, const AZOutputStreamImplementation *out_impl, AZOutputStream *out_inst);

#ifdef __cplusplus
};
#endif

#endif