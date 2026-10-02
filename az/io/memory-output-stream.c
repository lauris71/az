#define __AZ_MEMORY_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/memory-output-stream.h>

static void mostream_class_init (AZMemoryOutputStreamClass *klass);
static int64_t mostream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size);
static int64_t mostream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst);

unsigned int mostream_type = 0;
AZMemoryOutputStreamClass *mostream_class = NULL;

unsigned int
az_memory_output_stream_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(mostream_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!mostream_type) {
		mostream_class = (AZMemoryOutputStreamClass *) az_register_type(&mostream_type, (const unsigned char *) "AZMemoryOutputStream", AZ_TYPE_STRUCT,
            sizeof (AZMemoryOutputStreamClass), sizeof (AZMemoryOutputStream), AZ_FLAG_FINAL, 0, 0,
            (void (*) (AZClass *)) mostream_class_init,
            NULL, NULL);
	}
	t = mostream_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
mostream_class_init (AZMemoryOutputStreamClass *klass)
{
    az_class_declare_interface((AZClass *) klass, 0, AZ_TYPE_OUTPUT_STREAM, ARIKKEI_OFFSET(AZMemoryOutputStreamClass, ostream_impl), 0);
    klass->ostream_impl.write = mostream_write;
    klass->ostream_impl.close = mostream_close;
}

static int64_t
mostream_write(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
    AZMemoryOutputStream *mostream = (AZMemoryOutputStream *) inst;
    if (mostream->pos + size > mostream->size) {
        size = mostream->size - mostream->pos;
    }
    memcpy(mostream->buffer + mostream->pos, data, size);
    mostream->pos += size;
    return size;
}

static int64_t
mostream_close(const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
    return AZ_OK;
}
