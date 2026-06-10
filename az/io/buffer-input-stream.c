#define __AZ_BUFFER_INPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>
#include <string.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/buffer-input-stream.h>

static void bistream_class_init (AZBufferInputStreamClass *klass);
static int64_t bistream_read (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size);
static int64_t bistream_seek (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset);
static int64_t bistream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes);
static int bistream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst);
static int bistream_is_error (const AZInputStreamImplementation *impl, AZInputStream *inst);

unsigned int bistream_type = 0;
AZBufferInputStreamClass *bistream_class = NULL;

unsigned int
az_buffer_input_stream_get_type (void)
{
	if (bistream_type) return bistream_type;
	AZ_TYPES_LOCK();
	/* Just in case someone else registered the type while we were waiting for the lock */
	if (!bistream_type) {
		bistream_class = (AZBufferInputStreamClass *) az_register_type (&bistream_type, (const unsigned char *) "AZBufferInputStream", AZ_TYPE_STRUCT,
			sizeof (AZBufferInputStreamClass), sizeof (AZBufferInputStream), AZ_FLAG_FINAL, 0, 0,
			(void (*) (AZClass *)) bistream_class_init,
			NULL, NULL);
	}
	AZ_TYPES_UNLOCK();
	return bistream_type;
}

static void
bistream_class_init (AZBufferInputStreamClass *klass)
{
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_INPUT_STREAM, ARIKKEI_OFFSET (AZBufferInputStreamClass, istream_impl), 0);
	klass->istream_impl.read = bistream_read;
	klass->istream_impl.seek = bistream_seek;
	klass->istream_impl.skip = bistream_skip;
	klass->istream_impl.is_eof = bistream_is_eof;
	klass->istream_impl.is_error = bistream_is_error;
}

static int64_t
bistream_read (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size)
{
	AZBufferInputStream *bistream = (AZBufferInputStream *) inst;
	if (bistream->pos >= bistream->size) return 0;
	if (bistream->pos + size > bistream->size) {
		size = bistream->size - bistream->pos;
	}
	memcpy (data, bistream->buffer + bistream->pos, size);
	bistream->pos += size;
	return (int64_t) size;
}

static int64_t
bistream_seek (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset)
{
	AZBufferInputStream *bistream = (AZBufferInputStream *) inst;
	if (offset > bistream->size) offset = bistream->size;
	bistream->pos = offset;
	return (int64_t) offset;
}

static int64_t
bistream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes)
{
	AZBufferInputStream *bistream = (AZBufferInputStream *) inst;
	uint64_t remaining = bistream->size - bistream->pos;
	if (n_bytes > remaining) n_bytes = remaining;
	bistream->pos += n_bytes;
	return (int64_t) n_bytes;
}

static int
bistream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	AZBufferInputStream *bistream = (AZBufferInputStream *) inst;
	return bistream->pos >= bistream->size;
}

static int
bistream_is_error (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return 0;
}