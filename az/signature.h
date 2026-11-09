#ifndef __AZ_SIGNATURE_H__
#define __AZ_SIGNATURE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

typedef struct _AZFunctionSignature32 AZFunctionSignature32;

#include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

extern AZClass AZFunctionSignatureKlass;

#define AZ_SIGNATURE_N_ARG_TYPES(sig) ((sig)->n_args)

struct _AZFunctionSignature {
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[];
};

struct _AZFunctionSignature32 {
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[32];
};

AZFunctionSignature* az_function_signature_new(unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);
AZFunctionSignature *az_function_signature_new_va(unsigned int ret_type, unsigned int n_args, ...);
AZFunctionSignature* az_function_signature_new_with_this(unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);
AZFunctionSignature* az_function_signature_new_with_this_any(unsigned int this_type, unsigned int ret_type, unsigned int n_args);
/* Strict compatibility check, i.e. only subclass types accepted */
unsigned int az_function_signature_is_assignable_to (const AZFunctionSignature *sig, const AZFunctionSignature *other, unsigned int test_ret_val);

#define az_function_signature_delete(sig)

#ifdef __cplusplus
};
#endif

#endif
