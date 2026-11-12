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

extern AZClass AZFunctionSignatureKlass;

/*
 * Convenience macros for function signature
 */
#define AZ_SIGNATURE_SIZE(sig) (sizeof(AZFunctionSignature) - 4 + ((sig)->n_args) * 4)
#define AZ_SIGNATURE_N_ARGS(sig) ((sig)->n_args)
/* NB: arg_types is declared as arg_types[1] but holds up to 255 entries (struct hack).
 * Accesses have to go through a void pointer cast so the compiler cannot use the
 * declared array bound to optimize them away (gcc does this at -O2/-O3). */
#define AZ_SIGNATURE_ARG_TYPES(sig) ((uint32_t *) (void *) (sig)->arg_types)
#define AZ_SIGNATURE_ARG_TYPE(sig,i) (((const uint32_t *) (const void *) (sig)->arg_types)[i])
#define AZ_SIGNATURE_RET_TYPE(sig) ((sig)->ret_type)

struct _AZFunctionSignature {
	uint32_t sig_type;
	uint32_t head_type;
	uint8_t n_args;
	/* Must be 0 */
	uint8_t _reserved;
	uint16_t sig_flags;
	uint32_t ret_type;
	uint32_t arg_types[1];
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
