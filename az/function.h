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

typedef struct _AZFunctionImplementation AZFunctionImplementation;

#include <az/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup function Function interface
 *  A callable interface
 */

extern AZClass AZFunctionSignatureKlass;
extern AZInterfaceClass AZFunctionKlass;

struct _AZFunctionSignature {
	/* Return type */
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[1];
};

struct _AZFunctionSignature32 {
	union {
		struct {
			uint32_t ret_type;
			uint32_t n_args;
			uint32_t arg_types[32];
		};
		AZFunctionSignature signature;
	};
};

AZFunctionSignature* az_function_signature_new (unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);
AZFunctionSignature* az_function_signature_new_any(unsigned int this_type, unsigned int ret_type, unsigned int n_args);
AZFunctionSignature *az_function_signature_new_va (unsigned int ret_type, unsigned int n_args, ...);
/* Strict compatibility check, i.e. only subclass types accepted */
unsigned int az_function_signature_is_assignable_to (const AZFunctionSignature *sig, const AZFunctionSignature *other, unsigned int test_ret_val);

void az_function_signature_delete(AZFunctionSignature* sig);

/** @ingroup function
 * @brief Polymorphic implementation of all function types
 * 
 */

struct _AZFunctionImplementation {
	AZImplementation implementation;
	/**
	 * @brief Get function signature
	 * 
	 */
	const AZFunctionSignature *(*signature) (const AZFunctionImplementation *impl, void *inst);
	/**
	 * @brief Invoke function
	 * 
	 */
	unsigned int (*invoke) (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
};

const AZFunctionSignature *az_function_get_signature (const AZFunctionImplementation *impl, void *inst);

/** @ingroup function
 *  First value is this (if applicable)
 */
unsigned int az_function_invoke (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
unsigned int az_function_convert_args_in_place (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], AZValue *arg_vals[]);

/** @ingroup function
 * @brief Invoke function using packed arguments
 * 
 * @param impl 
 * @param inst 
 * @param this_val 
 * @param ret_val 
 * @param args 
 * @param check_types 
 * @return unsigned int 
 */
unsigned int az_function_invoke_packed (const AZFunctionImplementation *impl, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);

/* Helper */
unsigned int az_function_invoke_by_type_instance (unsigned int type, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);
unsigned int az_instance_invoke_function (const AZImplementation *impl, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types);

/*
 * Native <-> az function calling rules (determined by signature)
 *
 * Arguments:
 * Objects - [pointer]
 * Primitive types - [value]
 * Final types - [pointer]
 * Non-final types - [impl, pointer]
 *
 * Return values:
 * Primitives - by value
 * Final blocks and objects - by pointer
 * Final values - void, the first (hidden) argument is a pointer to the return storage
 * Non-final values/blocks - implementation pointer, the first (hidden) argument is a
 * pointer to the return storage (struct storage for values, void * location for blocks)
 *
 * No references are created
 */

/** @ingroup function
 * @brief Invoke function interface
 * 
 * @param impl 
 * @param inst 
 * @param ret_impl 
 * @param ret_val 
 * @param ... 
 * @return unsigned int 
 */
unsigned int az_function_invoke_va (const AZFunctionImplementation *impl, void *inst, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);
unsigned int az_function_invoke_by_signature_va (const AZFunctionImplementation *impl, void *inst, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);
unsigned int az_function_invoke_by_value_signature_va (const AZImplementation *impl, const AZValue *val, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);

/** @ingroup function
 * @brief Call a native C function with az arguments
 *
 * Marshals the arguments according to the native calling rules (see above) and calls
 * the native function. The native function has to be non-variadic.
 *
 * Currently implemented for ARM64 (AAPCS64), x86-64 System V (AMD64) and
 * x86-64 Windows (Microsoft x64 calling convention), returns 0 on other
 * architectures.
 *
 * @param func the native function pointer
 * @param sig the function signature
 * @param ret_impl the returned implementation (may be NULL)
 * @param ret_val the returned value (may be NULL if the return value is not needed)
 * @param arg_impls argument implementations
 * @param arg_vals argument values
 * @return 1 on success, 0 on error
 */
unsigned int az_function_call_native (void (*func) (void), const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation *arg_impls[], const AZValue *arg_vals[]);

#ifdef __cplusplus
};
#endif

#endif
