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
#include <az/io/stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An abstract data consumer interface
 *
 * Data transfers (write) are blocking and return the transferred count,
 * 0 <= n <= requested, never an error code. If n < requested, state() is not
 * AZ_STREAM_STATE_OK and reports the reason (see az/io/stream.h).
 * close returns AZ_OK or a negative AZErrorCode.
 */

struct _AZOutputStreamImplementation {
    AZImplementation impl;
    /**
     * @brief Write data to the stream
     *
     * The returned count is always between 0 and @c size. If it is less than
     * @c size, state() reports the reason (AZ_STREAM_STATE_OUT_OF_SPACE,
     * AZ_STREAM_STATE_CLOSED or a negative error code).
     *
     * @param impl The ouput stream implementation
     * @param inst The output stream instance
     * @param data The data buffer to write
     * @param size The size of the data buffer
     * @return The number of bytes written (0 <= n <= @c size)
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
    int (*close) (const AZOutputStreamImplementation *impl, AZOutputStream *inst);
    /**
     * @brief Get the stream state
     *
     * @return AZ_STREAM_STATE_OK, a positive benign AZStreamState or a negative AZErrorCode
     */
    int (*state) (const AZOutputStreamImplementation *impl, AZOutputStream *inst);
};

static inline int64_t
az_output_stream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
	return impl->write (impl, inst, data, size);
}

static inline int
az_output_stream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
	return (impl->close) ? impl->close (impl, inst) : AZ_OK;
}

static inline int
az_output_stream_state (const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
	return (impl->state) ? impl->state (impl, inst) : AZ_STREAM_STATE_OK;
}

/**
 * @brief Serialize the instance and write it to the stream
 *
 * Returns the number of bytes written (>= 0, may be short - state() tells why)
 * or a negative error code if the data could not even be prepared.
 */
int64_t az_output_stream_write_inst(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const AZImplementation *data_impl, void *data_inst);
/**
 * @brief Convert the instance to string and write it to the stream
 *
 * Returns the number of bytes written (>= 0, may be short - state() tells why)
 * or a negative error code if the data could not even be prepared.
 */
int64_t az_output_stream_print_inst(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const AZImplementation *data_impl, void *data_inst);

#ifdef __cplusplus
};
#endif

#endif
