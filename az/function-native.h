#ifndef __AZ_FUNCTION_NATIVE_H__
#define __AZ_FUNCTION_NATIVE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

/*
 * Simple value type that calls a native C function by pointer
 */

#define AZ_TYPE_FUNCTION_NATIVE az_function_native_get_type ()

typedef struct _AZFunctionNativeClass AZFunctionNativeClass;
typedef struct _AZFunctionNative AZFunctionNative;

#include <az/function.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZFunctionNative {
	AZFunctionSignature *signature;
	void (*func) (void);
};

struct _AZFunctionNativeClass {
	AZClass klass;
	AZFunctionImplementation function_impl;
};

unsigned int az_function_native_get_type (void);

void az_function_native_setup (AZFunctionNative *fnat, AZFunctionSignature *sig, void (*func) (void));

#ifdef __cplusplus
};
#endif

#endif
