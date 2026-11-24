#define __AZ_TESTS_IO_C__

#include <az/value.h>
#include <az/string.h>
#include <az/io/buffer-output-stream.h>
#include <az/io/memory-input-stream.h>
#include <az/io/memory-output-stream.h>
#include <az/io/os-output-stream.h>
#include <az/io/stream-utils.h>

#include "unity/unity.h"

void
test_io(void)
{
    az_init();
    void *inst;

    /* Buffer output stream: print/write, state is OK */
    AZBufferOutputStream *bostream = (AZBufferOutputStream *) az_instance_new(AZ_TYPE_BUFFER_OUTPUT_STREAM);
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
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OK, az_output_stream_state(impl, inst));
    TEST_ASSERT_EQUAL_INT(AZ_OK, az_output_stream_close(impl, inst));
    TEST_ASSERT(bostream->buffer != NULL);
    fprintf(stderr, "Buffer output stream: '%s'\n", (const char *) bostream->buffer);
    TEST_ASSERT_EQUAL_UINT64(37, bostream->pos);
    TEST_ASSERT_EQUAL_STRING("42 305419896 3.1415927 Hello, world!", (const char *) bostream->buffer);
    az_instance_delete(AZ_TYPE_BUFFER_OUTPUT_STREAM, bostream);

    /* Memory input stream: transfers report end of data via state() */
    AZMemoryInputStream mistream = { (const uint8_t *) "abcdef", 6, 0 };
    const AZInputStreamImplementation *in_impl = (const AZInputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_MEMORY_INPUT_STREAM), &mistream, AZ_TYPE_INPUT_STREAM, &inst);
    TEST_ASSERT_NOT_NULL(in_impl);
    uint8_t rbuf[4];
    TEST_ASSERT_EQUAL_INT64(4, az_input_stream_read(in_impl, inst, rbuf, 4));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OK, az_input_stream_state(in_impl, inst));
    TEST_ASSERT_FALSE(az_input_stream_is_eof(in_impl, inst));
    /* Short read - state() is not OK and tells why */
    TEST_ASSERT_EQUAL_INT64(2, az_input_stream_read(in_impl, inst, rbuf, 4));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_EOF, az_input_stream_state(in_impl, inst));
    TEST_ASSERT_TRUE(az_input_stream_is_eof(in_impl, inst));
    TEST_ASSERT_EQUAL_INT64(0, az_input_stream_read(in_impl, inst, rbuf, 4));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_EOF, az_input_stream_state(in_impl, inst));
    /* Seeking back reopens the stream */
    TEST_ASSERT_EQUAL_INT64(0, az_input_stream_seek(in_impl, inst, 0));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OK, az_input_stream_state(in_impl, inst));
    /* skip returns the skipped count */
    TEST_ASSERT_EQUAL_INT64(4, az_input_stream_skip(in_impl, inst, 4));
    TEST_ASSERT_EQUAL_INT64(2, az_input_stream_skip(in_impl, inst, 4));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_EOF, az_input_stream_state(in_impl, inst));
    /* Reading to the end of data is a success */
    uint8_t *all = NULL;
    TEST_ASSERT_EQUAL_INT64(0, az_input_stream_seek(in_impl, inst, 0));
    TEST_ASSERT_EQUAL_INT64(6, az_input_stream_read_all(in_impl, inst, &all));
    TEST_ASSERT_NOT_NULL(all);
    TEST_ASSERT_EQUAL_MEMORY("abcdef", all, 6);
    free(all);

    /* Memory output stream: short write reports OUT_OF_SPACE */
    uint8_t wbuf[4];
    AZMemoryOutputStream mostream = { wbuf, 4, 0 };
    const AZOutputStreamImplementation *mo_impl = (const AZOutputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_MEMORY_OUTPUT_STREAM), &mostream, AZ_TYPE_OUTPUT_STREAM, &inst);
    TEST_ASSERT_NOT_NULL(mo_impl);
    TEST_ASSERT_EQUAL_INT64(3, az_output_stream_write(mo_impl, inst, "abc", 3));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OK, az_output_stream_state(mo_impl, inst));
    TEST_ASSERT_EQUAL_INT64(1, az_output_stream_write(mo_impl, inst, "de", 2));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OUT_OF_SPACE, az_output_stream_state(mo_impl, inst));
    TEST_ASSERT_EQUAL_INT64(0, az_output_stream_write(mo_impl, inst, "f", 1));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OUT_OF_SPACE, az_output_stream_state(mo_impl, inst));

    /* OS output stream: closed is a benign state, not an error */
    FILE *file = tmpfile();
    TEST_ASSERT_NOT_NULL(file);
    AZOSOutputStream osostream = { file };
    const AZOutputStreamImplementation *os_impl = (const AZOutputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_OS_OUTPUT_STREAM), &osostream, AZ_TYPE_OUTPUT_STREAM, &inst);
    TEST_ASSERT_NOT_NULL(os_impl);
    TEST_ASSERT_EQUAL_INT64(3, az_output_stream_write(os_impl, inst, "abc", 3));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_OK, az_output_stream_state(os_impl, inst));
    TEST_ASSERT_EQUAL_INT(AZ_OK, az_output_stream_close(os_impl, inst));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_CLOSED, az_output_stream_state(os_impl, inst));
    TEST_ASSERT_EQUAL_INT64(0, az_output_stream_write(os_impl, inst, "x", 1));
    TEST_ASSERT_EQUAL_INT(AZ_STREAM_STATE_CLOSED, az_output_stream_state(os_impl, inst));
    TEST_ASSERT_EQUAL_INT(AZ_OK, az_output_stream_close(os_impl, inst));

    /* Stream copy reports the copied count */
    AZMemoryInputStream csrc = { (const uint8_t *) "abcdefgh", 8, 0 };
    uint8_t cbuf[8];
    AZMemoryOutputStream cdst = { cbuf, sizeof(cbuf), 0 };
    const AZInputStreamImplementation *c_in = (const AZInputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_MEMORY_INPUT_STREAM), &csrc, AZ_TYPE_INPUT_STREAM, &inst);
    void *o_inst;
    const AZOutputStreamImplementation *c_out = (const AZOutputStreamImplementation *) az_instance_get_interface(
        AZ_IMPL_FROM_TYPE(AZ_TYPE_MEMORY_OUTPUT_STREAM), &cdst, AZ_TYPE_OUTPUT_STREAM, &o_inst);
    TEST_ASSERT_EQUAL_INT64(3, az_stream_copy(c_in, inst, c_out, o_inst, 3));
    TEST_ASSERT_EQUAL_INT64(5, az_stream_copy_all(c_in, inst, c_out, o_inst));
    TEST_ASSERT_EQUAL_MEMORY("abcdefgh", cbuf, 8);
}
