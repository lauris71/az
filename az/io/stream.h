#ifndef __AZ_STREAM_H__
#define __AZ_STREAM_H__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

/*
 * Shared stream semantics
 *
 * Streams are blocking: read/write/skip either transfer the requested amount
 * or stop at a stream boundary or failure. They always return the transferred
 * count (0 <= n <= requested) and never an error code.
 *
 * If the returned count is less than the requested size, state() is not
 * AZ_STREAM_STATE_OK and reports the reason - either a benign boundary state
 * or a negative error code. state() may be non-OK after a fully successful
 * transfer that exactly reached a boundary.
 *
 * Operations that do not transfer data (seek, close) and frontends that can
 * fail before transferring anything (read_all allocation, print formatting)
 * return their result as a nonnegative value (or AZ_OK) and a negative
 * AZErrorCode on failure.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Stream state
 *
 * Returned by the state() methods:
 * - AZ_STREAM_STATE_OK (0) - the stream is fully operational
 * - positive - benign boundary state (not an error)
 * - negative - AZErrorCode, the operation failed
 */
enum AZStreamState {
	AZ_STREAM_STATE_OK = 0,
	/** The stream has been closed */
	AZ_STREAM_STATE_CLOSED = 1,
	/**
	 * The backing storage boundary was reached
	 *
	 * Output: the fixed capacity is exhausted (no more space to write)
	 * Input: end of data (AZ_STREAM_STATE_EOF is the readable alias)
	 */
	AZ_STREAM_STATE_OUT_OF_SPACE = 2,
	AZ_STREAM_STATE_EOF = AZ_STREAM_STATE_OUT_OF_SPACE
};

#ifdef __cplusplus
};
#endif

#endif
