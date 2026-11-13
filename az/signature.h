#ifndef __AZ_SIGNATURE_H__
#define __AZ_SIGNATURE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

extern AZClass AZSignatureKlass;

enum AZSignatureFlags {
	/**
	 * @brief Function is pure
	 * 
	 */
	AZ_SIGNATURE_FLAG_PURE = 1,
	/**
	 * @brief Function accepts variable number of arguments
	 * 
	 * The last argument type is the type of variadic arguments (0 or more)
	 */
	AZ_SIGNATURE_FLAG_VA_ARGS = 2
};

#define AZ_SIGNATURE_SIZE(sig) (sizeof(AZFunctionSignature) + ((sig)->n_params) * 4)

/*
 * Convenience macros for function signature
 */
#define AZ_SIGNATURE_N_ARGS(sig) ((sig)->n_params - 1)
#define AZ_SIGNATURE_ARG_TYPES(sig) (&(sig)->params[1])
#define AZ_SIGNATURE_ARG_TYPE(sig,i) ((sig)->params[i + 1])
#define AZ_SIGNATURE_RET_TYPE(sig) ((sig)->params[0])

struct _AZSignature {
	uint32_t sig_type;
	uint32_t head_type;
	uint8_t n_params;
	/* Must be 0 */
	uint8_t _reserved;
	uint16_t sig_flags;
	uint32_t params[];
};

const AZFunctionSignature* az_function_signature_new(unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);
const AZFunctionSignature *az_function_signature_new_va(unsigned int ret_type, unsigned int n_args, ...);
const AZFunctionSignature* az_function_signature_new_with_this(unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);

#define az_function_signature_delete(sig)

#ifdef __cplusplus
};
#endif

#endif
