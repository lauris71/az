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
	AZFunctionInstance function_inst;
	unsigned int (*invoke) (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
};

struct _AZFunctionValueClass {
	AZClass klass;
	AZFunctionImplementation function_impl;
};

unsigned int az_function_value_get_type (void);

void az_function_value_setup (AZFunctionValue *fval, AZFunctionSignature *sig,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *));

#ifdef __cplusplus
};
#endif

#endif
