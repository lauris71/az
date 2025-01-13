#ifndef __AZ_FUNCTION_H__
#define __AZ_FUNCTION_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_FUNCTION_MAX_RETURN_VALUE_SIZE 64

typedef struct _AZFunctionSignature AZFunctionSignature;
typedef struct _AZFunctionSignature32 AZFunctionSignature32;
typedef struct _AZFunctionSignatureClass AZFunctionSignatureClass;

typedef struct _AZFunctionInstance AZFunctionInstance;
typedef struct _AZFunctionImplementation AZFunctionImplementation;
typedef struct _AZFunctionClass AZFunctionClass;

#include <az/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZFunctionSignature {
	/* Return type */
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[1];
};

struct _AZFunctionSignature32 {
	/* Return type */
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[32];
};

AZFunctionSignature* az_function_signature_new (unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);
AZFunctionSignature* az_function_signature_new_any(unsigned int this_type, unsigned int ret_type, unsigned int n_args);
AZFunctionSignature *az_function_signature_new_va (unsigned int ret_type, unsigned int n_args, ...);
/* Strict compatibility check, i.e. only subclass types accepted */
unsigned int az_function_signature_is_assignable_to (const AZFunctionSignature *sig, const AZFunctionSignature *other, unsigned int test_ret_val);

void az_function_signature_delete(AZFunctionSignature* sig);

struct _AZFunctionInstance {
	AZFunctionSignature *signature;
};

struct _AZFunctionClass {
	AZInterfaceClass interface_class;
};

struct _AZFunctionImplementation {
	AZImplementation implementation;
	const AZFunctionSignature *(*signature) (const AZFunctionImplementation *impl, AZFunctionInstance *inst);
	unsigned int (*invoke) (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
};

const AZFunctionSignature *az_function_get_signature (const AZFunctionImplementation *impl, AZFunctionInstance *inst);

/* First value is this (if applicable) */
unsigned int az_function_invoke (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
unsigned int az_function_convert_args_in_place (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], AZValue *arg_vals[]);

unsigned int az_function_invoke_packed (const AZFunctionImplementation *impl, AZFunctionInstance *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);

/* Helper */
unsigned int az_function_invoke_by_type_instance (unsigned int type, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);
unsigned int az_instance_invoke_function (const AZImplementation *impl, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);

/*
 * Argument rules
 * Final types with value size <= 8 bytes - as values
 * Final types with value size > 8 bytes - as pointers
 * Objects as values
 * Non-final types as implementation, value/pointer
 *
 * No references are created
 * fixme: This should be changed
 */

unsigned int az_function_invoke_va (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);
unsigned int az_function_invoke_by_signature_va (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);
unsigned int az_function_invoke_by_value_signature_va (const AZImplementation *impl, const AZValue *val, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);

/* Library internals */
void az_init_function_classes (void);

#ifdef __cplusplus
};
#endif

#endif
