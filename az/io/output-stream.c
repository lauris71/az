#define __AZ_OUTPUT_STREAM_C__

/*
 * A run-time type library
 *
 * Copyright (C) Lauris Kaplinski 2026
 */

#include <stdlib.h>

#include <az/base.h>
#include <az/types.h>
#include <az/private.h>
#include <az/instance.h>
#include <az/extend.h>
#include <az/boxed-interface.h>
#include <az/function.h>

#include <az/io/output-stream.h>

enum {
	FUNC_WRITE,
	FUNC_PRINT,
	FUNC_PRINTLN,
	OSTREAM_NUM_PROPS
};


AZ_CLASS_ALIGN AZInterfaceClass AZOutputStreamKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_OUTPUT_STREAM },
		.parent = &AZInterfaceKlass.klass,
		.name = (const uint8_t *) "output stream",
		.alignment = 3,
		.class_size = sizeof(AZInterfaceKlass),
		.instance_size = 0,
		.n_props_self = OSTREAM_NUM_PROPS,
		.to_string = az_any_to_string
	},
	.implementation_size = sizeof(AZOutputStreamImplementation),
	.implementation_init = NULL
};

static int64_t
ostream_write(const AZOutputStreamImplementation *os_impl, AZOutputStream *os_inst, const AZImplementation *data_impl, void *data_inst)
{
	if ((void *) os_impl == &AZBoxedInterfaceKlass.klass.impl) {
		az_boxed_interface_unbox((const AZImplementation **) &os_impl, (void **) &os_inst);
	}
	int64_t ret = az_output_stream_write_inst(os_impl, os_inst, data_impl, data_inst);
	return ret;
}

static int64_t
ostream_print_native(const AZOutputStreamImplementation *os_impl, AZOutputStream *os_inst, const AZImplementation *data_impl, void *data_inst)
{
	if ((void *) os_impl == &AZBoxedInterfaceKlass.klass.impl) {
		az_boxed_interface_unbox((const AZImplementation **) &os_impl, (void **) &os_inst);
	}
	int64_t ret = az_output_stream_print_inst(os_impl, os_inst, data_impl, data_inst);
	return ret;
}

static int
ostream_print(unsigned int n_args, AZStoreElement *args, AZValue *vals, const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx)
{
	if (n_args < 1) return AZ_SIGNATURE_MISMATCH;
	AZOutputStreamImplementation *os_impl = (AZOutputStreamImplementation *) args[0].impl;
	AZOutputStream *os_inst = (AZOutputStream *) az_value_store_instance_at(args, 0, vals);
	if ((void *) os_impl == &AZBoxedInterfaceKlass.klass.impl) {
		az_boxed_interface_unbox((const AZImplementation **) &os_impl, (void **) &os_inst);
	}
	int64_t total = 0;
	for (unsigned int i = 1; i < n_args; i++) {
		int64_t ret = az_output_stream_print_inst(os_impl, os_inst, args[i].impl, az_value_store_instance_at(args, i, vals));
		if (ret < 0) return ret;
		total += ret;
	}
	if (ret_impl) {
		*ret_impl = &AZInt64Klass.impl;
		ret_val->int64_v = total;
	}
	return AZ_OK;
}

static int
ostream_println(unsigned int n_args, AZStoreElement *args, AZValue *vals, const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx)
{
	if (n_args < 1) return AZ_SIGNATURE_MISMATCH;
	AZOutputStreamImplementation *os_impl = (AZOutputStreamImplementation *) args[0].impl;
	AZOutputStream *os_inst = (AZOutputStream *) az_value_store_instance_at(args, 0, vals);
	if ((void *) os_impl == &AZBoxedInterfaceKlass.klass.impl) {
		az_boxed_interface_unbox((const AZImplementation **) &os_impl, (void **) &os_inst);
	}
	int64_t total = 0;
	for (unsigned int i = 1; i < n_args; i++) {
		int64_t ret = az_output_stream_print_inst(os_impl, os_inst, args[i].impl, az_value_store_instance_at(args, i, vals));
		if (ret < 0) return ret;
		total += ret;
	}
	int64_t ret = az_output_stream_write (os_impl, os_inst, "\n", 1);
	if (ret < 0) return ret;
	total += ret;
	if (ret_impl) {
		*ret_impl = &AZInt64Klass.impl;
		ret_val->int64_v = total;
	}
	return AZ_OK;
}

void
az_init_output_stream_class()
{
    az_class_new_with_value(&AZOutputStreamKlass.klass);
	az_class_define_method_native_va((AZClass *) &AZOutputStreamKlass, FUNC_WRITE, (const uint8_t *) "write", (void (*) ()) ostream_write, AZ_TYPE_INT64, 1, AZ_TYPE_ANY);
	az_class_define_method((AZClass *) &AZOutputStreamKlass, FUNC_PRINT, (const uint8_t *) "print",
		az_function_signature_new_va(AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_INT64, 2, AZ_TYPE_OUTPUT_STREAM, AZ_TYPE_ANY), ostream_print);
	az_class_define_method((AZClass *) &AZOutputStreamKlass, FUNC_PRINTLN, (const uint8_t *) "printLn",
		az_function_signature_new_va(AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_INT64, 2, AZ_TYPE_OUTPUT_STREAM, AZ_TYPE_ANY), ostream_println);
}

int64_t
az_output_stream_write_inst(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const AZImplementation *data_impl, void *data_inst)
{
    if (!data_impl || !data_inst) return 0;
	uint8_t d[1024];
	unsigned int dlen = az_instance_serialize (data_impl, data_inst, d, sizeof(d), NULL);
	if (dlen > sizeof(d)) {
		unsigned char *buf = (unsigned char *) malloc(dlen);
		if (!buf) return -1;
		dlen = az_instance_serialize (data_impl, data_inst, buf, dlen, NULL);
		int64_t ret = az_output_stream_write(impl, inst, buf, dlen);
		free(buf);
		return ret;
	}
	return az_output_stream_write(impl, inst, d, dlen);
}

int64_t
az_output_stream_print_inst(const AZOutputStreamImplementation *impl, AZOutputStream *inst, const AZImplementation *data_impl, void *data_inst)
{
	if (!data_impl || !data_inst) return 0;
	unsigned char d[1024];
	unsigned int dlen = az_instance_to_string (data_impl, data_inst, d, sizeof(d));
	if (dlen > sizeof(d)) {
		unsigned char *buf = (unsigned char *) malloc(dlen);
		if (!buf) return -1;
		dlen = az_instance_to_string (data_impl, data_inst, buf, dlen);
		int64_t ret = az_output_stream_write(impl, inst, buf, dlen);
		free(buf);
		return ret;
	}
	return az_output_stream_write (impl, inst, d, dlen);
}
