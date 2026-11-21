#define __AZ_FUNCTION_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

#include <stdlib.h>

#include <az/function-value.h>
#include <az/extend.h>

const AZFunctionSignature *
fval_signature (const AZFunctionImplementation *impl, void *inst)
{
	AZFunctionValue *fval = (AZFunctionValue *) inst;
	return fval->signature;
}

static int
function_value_invoke (const AZFunctionImplementation *impl, void *inst, unsigned int n_args, AZStoreElement args[], AZValue data[],
		const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx)
{
	AZFunctionValue *fval = (AZFunctionValue *) inst;
	unsigned int result = fval->invoke(n_args, args, data, ret_impl, ret_val, ret_size, ctx);
	return result;
}

static void
function_value_class_init (AZFunctionValueClass *klass)
{
	az_class_declare_interface (&klass->klass, 0, AZ_TYPE_FUNCTION, offsetof(AZFunctionValueClass, function_impl), 0);
	klass->function_impl.signature = fval_signature;
	klass->function_impl.invoke = function_value_invoke;
}

static unsigned int function_value_type = 0;

unsigned int
az_function_value_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(function_value_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!function_value_type) {
		az_register_type (&function_value_type, (const unsigned char *) "FunctionValue", AZ_TYPE_STRUCT, sizeof(AZFunctionValueClass), sizeof(AZFunctionValue), AZ_FLAG_FINAL,
			1, 0,
			(void (*) (AZClass *)) function_value_class_init,
			NULL, NULL);
	}
	t = function_value_type;
	AZ_TYPES_UNLOCK();
	return t;
}

void
az_function_value_setup (AZFunctionValue *fval, const AZFunctionSignature *sig,
	int (*invoke) (unsigned int n_args, AZStoreElement args[], AZValue data[],
	const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx))
{
	fval->signature = sig;
	fval->invoke = invoke;
}

