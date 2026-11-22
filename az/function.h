#ifndef __AZ_FUNCTION_H__
#define __AZ_FUNCTION_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_FUNCTION_MAX_RETURN_VALUE_SIZE 64

typedef struct _AZFunctionImplementation AZFunctionImplementation;

#include <az/interface.h>
#include <az/signature.h>
#include <az/value-store.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup function Function interface
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
	 * @brief Invoke function using value store
	 * 
	 * The arguments have to match signature (is_a), i.e.
	 * - nulls typed
	 * - arithmetic values exact
	 * - interfaces resolved
	 * Boxed values and boxed interfaces are allowed
	 * 
	 * @param impl The function implementation
	 * @param inst The instance data
	 * @param n_args Number of arguments
	 * @param args Array of argument elements
	 * @param data Data buffer of arguments
	 * @param data_end Pointer to data end position
	 * @param ret_impl The return implementation pointer
	 * @param ret_val Pointer to return value
	 * @param ret_size Size of return value
	 * @param ctx The context
	 * @return Exception code or AZ_OK
	 */
	int (*invoke) (const AZFunctionImplementation *impl, void *inst, unsigned int n_args,
		AZStoreElement args[], AZValue data[],
		const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx);
};

const AZFunctionSignature *az_function_get_signature (const AZFunctionImplementation *impl, void *inst);

/**
 * @brief Check whether function with given signature is assignable to another
 * 
 * Requires strict compatibility for both arguments and return value - only subclasses accepted, no implicit conversions
 * 
 * @param sig The function signature
 * @param other The other function signature
 * @param test_ret_val Whether to test return value also
 * @return 1 if assignable, 0 otherwise
 */
unsigned int az_function_signature_is_assignable_to(const AZFunctionSignature *sig, const AZFunctionSignature *other, unsigned int test_ret_val);
/**
 * @brief Check whether a function with the given signature is callable with the given return type and argument types
 * 
 * @param sig The function signature
 * @param ret_type The return type (NONE to ignore return value)
 * @param n_args The number of arguments
 * @param arg_types Array of argument types
 * @return The required conversion type
 */
unsigned int az_function_signature_is_callable_as(const AZFunctionSignature *sig, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[]);

/* Returns error/exception code */
int az_function_invoke (const AZFunctionImplementation *impl, void *inst, unsigned int n_args, AZStoreElement args[], AZValue data[], const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx);

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
 * @param impl The function implementation
 * @param inst The instance data
 * @param sig The function signature
 * @param ctx The context
 * @param ret_impl The return implementation pointer (NULL to ignore return)
 * @param ret_val The return value buffer
 * @param ... Variadic arguments for the function
 * @return unsigned int 
 */
unsigned int az_function_invoke_by_signature_va (const AZFunctionImplementation *impl, void *inst, const AZFunctionSignature *sig, AZContext *ctx, const AZImplementation **ret_impl, AZValue64 *ret_val, ...);

/** @ingroup function
 * @brief Call a native C function with az arguments
 *
 * Marshals the arguments according to the native calling rules (see above) and calls
 * the native function. The native function has to be non-variadic.
 *
 * Currently implemented for ARM64 (AAPCS64), x86-64 System V (AMD64) and
 * x86-64 Windows (Microsoft x64 calling convention), returns
 * AZ_NOT_IMPLEMENTED on other architectures.
 *
 * @param func the native function pointer
 * @param sig the function signature
 * @param n_args the number of arguments
 * @param args the argument store elements
 * @param data the argument values
 * @param ret_impl the returned implementation (may be NULL)
 * @param ret_val the returned value (may be NULL if the return value is not needed)
 * @param ret_size the size of the return value buffer
 * @param ctx the context
 * @return AZ_OK or error/exception code
 */
int az_function_invoke_native(void (*func) (void), const AZFunctionSignature *sig, unsigned int n_args, AZStoreElement args[], AZValue data[], const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx);

#ifdef __cplusplus
};
#endif

#endif
