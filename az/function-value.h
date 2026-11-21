#ifndef __AZ_FUNCTION_VALUE_H__
#define __AZ_FUNCTION_VALUE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

/*
 * Simple value type that calls function by stored pointer
 */

#define AZ_TYPE_FUNCTION_VALUE az_function_value_get_type ()

typedef struct _AZFunctionValueClass AZFunctionValueClass;
typedef struct _AZFunctionValue AZFunctionValue;

#include <az/function.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZFunctionValue {
	const AZFunctionSignature *signature;
	/* Returns exception code or OK */
	int (*invoke) (unsigned int n_args, AZStoreElement args[], AZValue data[], const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx);
};

struct _AZFunctionValueClass {
	AZClass klass;
	AZFunctionImplementation function_impl;
};

unsigned int az_function_value_get_type (void);

void az_function_value_setup (AZFunctionValue *fval, const AZFunctionSignature *sig,
	int (*invoke) (unsigned int n_args, AZStoreElement args[], AZValue data[],
		const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx));

#ifdef __cplusplus
};
#endif

#endif
