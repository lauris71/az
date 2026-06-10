#define __AZ_BUFFER_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/buffer-output-stream.h>

static void bostream_class_init (AZBufferOutputStreamClass *klass);
static int64_t bostream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size);
static int64_t bostream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst);

unsigned int bostream_type = 0;
AZBufferOutputStreamClass *bostream_class = NULL;

unsigned int
az_buffer_output_stream_get_type (void)
{
	if (bostream_type) return bostream_type;
	AZ_TYPES_LOCK();
	/* Just in case someone else registered the type while we were waiting for the lock */
	if (!bostream_type) {
		bostream_class = (AZBufferOutputStreamClass *) az_register_type(&bostream_type, (const unsigned char *) "AZBufferOutputStream", AZ_TYPE_STRUCT,
            sizeof (AZBufferOutputStreamClass), sizeof (AZBufferOutputStream), AZ_FLAG_FINAL, 0, 0,
            (void (*) (AZClass *)) bostream_class_init,
            NULL, NULL);
	}
	AZ_TYPES_UNLOCK();
	return bostream_type;
}

static void
bostream_class_init (AZBufferOutputStreamClass *klass)
{
    az_class_declare_interface((AZClass *) klass, 0, AZ_TYPE_OUTPUT_STREAM, ARIKKEI_OFFSET(AZBufferOutputStreamClass, ostream_impl), 0);
    klass->ostream_impl.write = bostream_write;
    klass->ostream_impl.close = bostream_close;
}

static int64_t
bostream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
    AZBufferOutputStream *bostream = (AZBufferOutputStream *) inst;
    if (bostream->pos + size > bostream->size) {
        size = bostream->size - bostream->pos;
    }
    memcpy(bostream->buffer + bostream->pos, data, size);
    bostream->pos += size;
    return size;
}

static int64_t
bostream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst)

{
    return AZ_OK;
}
