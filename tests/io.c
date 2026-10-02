#define __AZ_TESTS_IO_C__

#include <az/value.h>
#include <az/string.h>
#include <az/io/buffer-output-stream.h>

#include "unity/unity.h"

void
test_io(void)
{
    az_init();
    AZBufferOutputStream *bostream = (AZBufferOutputStream *) az_instance_new(AZ_TYPE_BUFFER_OUTPUT_STREAM);
    void *inst;
    const AZOutputStreamImplementation *impl = (const AZOutputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_BUFFER_OUTPUT_STREAM), bostream, AZ_TYPE_OUTPUT_STREAM, &inst);
    AZValue val;
    uint8_t u8 = 42;
    az_output_stream_print_inst(impl, inst, AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT8), &u8);
    az_output_stream_write(impl, inst, (const uint8_t *) " ", 1);
    uint32_t u32 = 0x12345678;
    az_output_stream_print_inst(impl, inst, AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT32), &u32);
    az_output_stream_write(impl, inst, (const uint8_t *) " ", 1);
    double d = 3.141592653589793238;
    az_output_stream_print_inst(impl, inst, AZ_IMPL_FROM_TYPE(AZ_TYPE_DOUBLE), &d);
    AZString *str = az_string_new((const uint8_t *) "Hello, world!");
    az_output_stream_write(impl, inst, (const uint8_t *) " ", 1);
    az_output_stream_print_inst(impl, inst, AZ_IMPL_FROM_TYPE(AZ_TYPE_STRING), str);
    az_string_unref(str);
    az_output_stream_write(impl, inst, (const uint8_t *) "", 1);
    az_output_stream_close(impl, inst);
    TEST_ASSERT(bostream->buffer != NULL);
    fprintf(stderr, "Buffer output stream: '%s'\n", (const char *) bostream->buffer);
    TEST_ASSERT_EQUAL_UINT64(37, bostream->pos);
    TEST_ASSERT_EQUAL_STRING("42 305419896 3.1415927 Hello, world!", (const char *) bostream->buffer);
    az_instance_delete(AZ_TYPE_BUFFER_OUTPUT_STREAM, bostream);
}