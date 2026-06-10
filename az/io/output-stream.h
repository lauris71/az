#ifndef __AZ_OUTPUT_STREAM_H__
#define __AZ_OUTPUT_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

typedef struct _AZOutputStreamImplementation AZOutputStreamImplementation;
typedef struct _AZInterfaceClass AZOutputStreamClass;

#include <stdint.h>

#include <az/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An abstract data consumer interface
 * 
 */

struct _AZOutputStreamImplementation {
    AZImplementation impl;
    /**
     * @brief Write data to the stream
     * 
     * The returned size is always equal to the size of the data buffer, or negative error code.
     * 
     * @param impl The ouput stream implementation
     * @param inst The output stream instance
     * @param data The data buffer to write
     * @param size The size of the data buffer
     * @return The @c size or error code
     * 
     */
	int64_t (*write) (const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size);
    /**
     * @brief Close the stream
     * 
     * The meaining of closing a stream is implementation-dependent.
     * 
     * @param impl The ouput stream implementation
     * @param inst The output stream instance
     * @return AZ_OK on success, negative error code on failure
     * 
     */
    int64_t (*close) (const AZOutputStreamImplementation *impl, AZOutputStream *inst);
};

static inline int64_t
az_output_stream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
	return impl->write (impl, inst, data, size);
}

static inline int64_t
az_output_stream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
	return (impl->close) ? impl->close (impl, inst) : 0;
}

#ifdef __cplusplus
};
#endif

#endif
