#define __AZ_OS_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/extend.h>
#include <az/types.h>

#include <az/io/os-output-stream.h>

static void osostream_class_init (AZOSOutputStreamClass *klass);
static int64_t osostream_write (const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size);
static int64_t osostream_close (const AZOutputStreamImplementation *impl, AZOutputStream *inst);

unsigned int osostream_type = 0;
AZOSOutputStreamClass *osostream_class = NULL;

unsigned int
az_os_output_stream_get_type (void)
{
	if (osostream_type) return osostream_type;
	AZ_TYPES_LOCK();
	if (!osostream_type) {
		osostream_class = (AZOSOutputStreamClass *) az_register_type (&osostream_type, (const unsigned char *) "AZOSOutputStream", AZ_TYPE_STRUCT,
			sizeof (AZOSOutputStreamClass), sizeof (AZOSOutputStream), AZ_FLAG_FINAL, 0, 0,
			(void (*) (AZClass *)) osostream_class_init,
			NULL, NULL);
	}
	AZ_TYPES_UNLOCK();
	return osostream_type;
}

static void
osostream_class_init (AZOSOutputStreamClass *klass)
{
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_OUTPUT_STREAM, ARIKKEI_OFFSET (AZOSOutputStreamClass, ostream_impl), 0);
	klass->ostream_impl.write = osostream_write;
	klass->ostream_impl.close = osostream_close;
}

static int64_t
osostream_write (const AZOutputStreamImplementation *impl, AZOutputStream *inst, const void *data, uint64_t size)
{
	AZOSOutputStream *osostream = (AZOSOutputStream *) inst;
	size_t written = fwrite (data, 1, size, osostream->file);
	if (written < size && ferror (osostream->file)) return -1;
	return (int64_t) written;
}

static int64_t
osostream_close (const AZOutputStreamImplementation *impl, AZOutputStream *inst)
{
	AZOSOutputStream *osostream = (AZOSOutputStream *) inst;
	return (int64_t) fclose (osostream->file);
}