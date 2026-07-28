#define __AZ_FUNCTION_NATIVE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdlib.h>

#include <az/function-native.h>
#include <az/extend.h>

static void function_native_class_init (AZFunctionNativeClass *klass);

/* AZFunction implementation */
const AZFunctionSignature *fnat_signature (const AZFunctionImplementation *impl, void *inst);
static unsigned int function_native_invoke (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

static unsigned int function_native_type = 0;

unsigned int
az_function_native_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(function_native_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!function_native_type) {
		az_register_type (&function_native_type, (const unsigned char *) "FunctionNative", AZ_TYPE_STRUCT, sizeof (AZFunctionNativeClass), sizeof (AZFunctionNative), AZ_FLAG_FINAL | AZ_FLAG_ZERO_MEMORY,
			1, 0,
			(void (*) (AZClass *)) function_native_class_init,
			NULL, NULL);
	}
	t = function_native_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
function_native_class_init (AZFunctionNativeClass *klass)
{
	az_class_declare_interface (&klass->klass, 0, AZ_TYPE_FUNCTION, ARIKKEI_OFFSET (AZFunctionNativeClass, function_impl), 0);
	klass->function_impl.signature = fnat_signature;
	klass->function_impl.invoke = function_native_invoke;
}

const AZFunctionSignature *
fnat_signature (const AZFunctionImplementation *impl, void *inst)
{
	AZFunctionNative *fnat = (AZFunctionNative *) inst;
	return fnat->signature;
}

static unsigned int
function_native_invoke (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	AZFunctionNative *fnat = (AZFunctionNative *) inst;
	return az_function_call_native (fnat->func, fnat->signature, ret_impl, ret_val, arg_impls, arg_vals);
}

void
az_function_native_setup (AZFunctionNative *fnat, AZFunctionSignature *sig, void (*func) (void))
{
	fnat->signature = sig;
	fnat->func = func;
}
