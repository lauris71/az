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

#include <az/io/output-stream.h>

static unsigned int ostream_call_write(const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
static unsigned int ostream_call_print(const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

enum {
	FUNC_WRITE,
	FUNC_PRINT,
	NUM_PROPS
};


AZ_CLASS_ALIGN AZInterfaceClass AZOutputStreamKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_OUTPUT_STREAM },
		.parent = &AZInterfaceKlass.klass,
		.name = (const uint8_t *) "output stream",
		.alignment = 3,
		.class_size = sizeof(AZInterfaceKlass),
		.instance_size = 0,
		.to_string = az_any_to_string
	},
	.implementation_size = sizeof(AZOutputStreamImplementation),
	.implementation_init = NULL
};

void
az_init_output_stream_class()
{
    az_class_new_with_value(&AZOutputStreamKlass.klass);
}

void
az_post_init_output_stream_class()
{
	az_class_set_num_properties (&AZOutputStreamKlass.klass, NUM_PROPS);
	az_class_define_method_va((AZClass *) &AZOutputStreamKlass, FUNC_WRITE, (const uint8_t *) "write", ostream_call_write, AZ_TYPE_INT64, 1, AZ_TYPE_ANY);
	az_class_define_method_va((AZClass *) &AZOutputStreamKlass, FUNC_PRINT, (const uint8_t *) "print", ostream_call_print, AZ_TYPE_INT64, 1, AZ_TYPE_ANY);
}

static unsigned int
ostream_call_write(const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	if (!arg_impls[1]) return 0;
	void *data_inst;
	const AZImplementation *data_impl = az_value_get_inst_autobox (arg_impls[1], arg_vals[1], &data_inst);
	*ret_impl = &AZInt64Klass.impl;
	ret_val->value.int64_v = az_output_stream_write_inst ((const AZOutputStreamImplementation *) arg_impls[0], (AZOutputStream *) arg_vals[0]->block, data_impl, data_inst);
	return 1;
}

static unsigned int
ostream_call_print(const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	if (!arg_impls[1]) return 0;
	void *data_inst;
	const AZImplementation *data_impl = az_value_get_inst_autobox (arg_impls[1], arg_vals[1], &data_inst);
	*ret_impl = &AZInt64Klass.impl;
	ret_val->value.int64_v = az_output_stream_print_inst ((const AZOutputStreamImplementation *) arg_impls[0], (AZOutputStream *) arg_vals[0]->block, data_impl, data_inst);
	return 1;
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
