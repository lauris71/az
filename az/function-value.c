#define __AZ_FUNCTION_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

#include <stdlib.h>

#include <az/function-value.h>

static void function_value_class_init (AZFunctionValueClass *klass);

/* AZFunction implementation */
static unsigned int function_value_invoke (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

static unsigned int function_value_type = 0;

unsigned int
az_function_value_get_type (void)
{
	if (!function_value_type) {
		az_register_type (&function_value_type, (const unsigned char *) "FunctionValue", AZ_TYPE_STRUCT, sizeof (AZFunctionValueClass), sizeof (AZFunctionValue), AZ_FLAG_FINAL | AZ_CLASS_ZERO_MEMORY,
			(void (*) (AZClass *)) function_value_class_init,
			NULL, NULL);
	}
	return function_value_type;
}

static void
function_value_class_init (AZFunctionValueClass *klass)
{
	az_class_set_num_interfaces (&klass->klass, 1);
	az_class_declare_interface (&klass->klass, 0, AZ_TYPE_FUNCTION, ARIKKEI_OFFSET (AZFunctionValueClass, function_impl), ARIKKEI_OFFSET (AZFunctionValue, function_inst));
	klass->function_impl.invoke = function_value_invoke;
}

static unsigned int
function_value_invoke (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	AZFunctionValue *fval = (AZFunctionValue *) inst;
	unsigned int result = fval->invoke (arg_impls, arg_vals, ret_impl, ret_val, ctx);
	return result;
}

void
az_function_value_setup (AZFunctionValue *fval, AZFunctionSignature *sig,
unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *))
{
	fval->function_inst.signature = sig;
	fval->invoke = invoke;
}
