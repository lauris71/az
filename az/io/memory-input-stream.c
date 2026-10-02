#define __AZ_MEMORY_INPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>
#include <string.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/memory-input-stream.h>

static void mistream_class_init (AZMemoryInputStreamClass *klass);
static int64_t mistream_read (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size);
static int64_t mistream_seek (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset);
static int64_t mistream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes);
static int mistream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst);
static int mistream_is_error (const AZInputStreamImplementation *impl, AZInputStream *inst);

unsigned int mistream_type = 0;
AZMemoryInputStreamClass *mistream_class = NULL;

unsigned int
az_buffer_input_stream_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(mistream_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!mistream_type) {
		az_register_type (&mistream_type, (const unsigned char *) "AZMemoryInputStream", AZ_TYPE_STRUCT,
			sizeof (AZMemoryInputStreamClass), sizeof (AZMemoryInputStream), AZ_FLAG_FINAL, 0, 0,
			(void (*) (AZClass *)) mistream_class_init,
			NULL, NULL);
		/* Registration nested inside another class construction is deferred - force it */
		mistream_class = (AZMemoryInputStreamClass *) az_type_get_class (mistream_type);
	}
	t = mistream_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
mistream_class_init (AZMemoryInputStreamClass *klass)
{
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_INPUT_STREAM, ARIKKEI_OFFSET (AZMemoryInputStreamClass, istream_impl), 0);
	klass->istream_impl.read = mistream_read;
	klass->istream_impl.seek = mistream_seek;
	klass->istream_impl.skip = mistream_skip;
	klass->istream_impl.is_eof = mistream_is_eof;
	klass->istream_impl.is_error = mistream_is_error;
}

static int64_t
mistream_read (const AZInputStreamImplementation *impl, AZInputStream *inst, void *data, uint64_t size)
{
	AZMemoryInputStream *mistream = (AZMemoryInputStream *) inst;
	if (mistream->pos >= mistream->size) return 0;
	if (mistream->pos + size > mistream->size) {
		size = mistream->size - mistream->pos;
	}
	memcpy (data, mistream->buffer + mistream->pos, size);
	mistream->pos += size;
	return (int64_t) size;
}

static int64_t
mistream_seek (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t offset)
{
	AZMemoryInputStream *mistream = (AZMemoryInputStream *) inst;
	if (offset > mistream->size) offset = mistream->size;
	mistream->pos = offset;
	return (int64_t) offset;
}

static int64_t
mistream_skip (const AZInputStreamImplementation *impl, AZInputStream *inst, uint64_t n_bytes)
{
	AZMemoryInputStream *mistream = (AZMemoryInputStream *) inst;
	uint64_t remaining = mistream->size - mistream->pos;
	if (n_bytes > remaining) n_bytes = remaining;
	mistream->pos += n_bytes;
	return (int64_t) n_bytes;
}

static int
mistream_is_eof (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	AZMemoryInputStream *mistream = (AZMemoryInputStream *) inst;
	return mistream->pos >= mistream->size;
}

static int
mistream_is_error (const AZInputStreamImplementation *impl, AZInputStream *inst)
{
	return 0;
}