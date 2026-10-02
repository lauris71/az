#define __AZ_BUFFER_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>
#include <string.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/buffer-output-stream.h>

#define MIN_ALLOC 256

static void bostream_class_init (AZBufferOutputStreamClass *klass);
static void bostream_finalize (AZBufferOutputStreamClass *klass, AZBufferOutputStream *bostream);
static int64_t bostream_write (const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size);
static int64_t bostream_close (const AZOutputStreamImplementation *impl, AZOutputStream *inst);

unsigned int bostream_type = 0;
AZBufferOutputStreamClass *bostream_class = NULL;

unsigned int
az_buffer_output_stream_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(bostream_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!bostream_type) {
		az_register_type (&bostream_type, (const unsigned char *) "AZBufferOutputStream", AZ_TYPE_BLOCK,
			sizeof (AZBufferOutputStreamClass), sizeof (AZBufferOutputStream), AZ_FLAG_FINAL | AZ_FLAG_ZERO_MEMORY, 1, 0,
			(void (*) (AZClass *)) bostream_class_init,
			(void (*) (const AZImplementation *, void *)) bostream_finalize,
			NULL);
		/* Registration nested inside another class construction is deferred - force it */
		bostream_class = (AZBufferOutputStreamClass *) az_type_get_class (bostream_type);
	}
	t = bostream_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
bostream_class_init (AZBufferOutputStreamClass *klass)
{
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_OUTPUT_STREAM, ARIKKEI_OFFSET (AZBufferOutputStreamClass, ostream_impl), 0);
	klass->ostream_impl.write = bostream_write;
	klass->ostream_impl.close = bostream_close;
}

static void
bostream_finalize (AZBufferOutputStreamClass *klass, AZBufferOutputStream *bostream)
{
	if (bostream->buffer) free (bostream->buffer);
}

static int64_t
bostream_write (const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
	AZBufferOutputStream *bostream = (AZBufferOutputStream *) inst;
	uint64_t needed = bostream->pos + size;
	if (needed > bostream->allocated) {
		uint64_t alloc = bostream->allocated;
		if (alloc < MIN_ALLOC) alloc = MIN_ALLOC;
		while (alloc < needed) alloc *= 2;
		uint8_t *nbuf = (uint8_t *) realloc (bostream->buffer, alloc);
		if (!nbuf) return AZ_OUT_OF_MEMORY;
		bostream->buffer = nbuf;
		bostream->allocated = alloc;
	}
	memcpy (bostream->buffer + bostream->pos, data, size);
	bostream->pos += size;
	return (int64_t) size;
}

static int64_t
bostream_close (const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
	AZBufferOutputStream *bostream = (AZBufferOutputStream *) inst;
	bostream->buffer = (uint8_t *) realloc (bostream->buffer, bostream->pos);
	if (!bostream->buffer) return AZ_OUT_OF_MEMORY;
	bostream->allocated = bostream->pos;
	return AZ_OK;
}
