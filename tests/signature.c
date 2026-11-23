#define __AZ_SIGNATURE_TEST_C__

/*
 * Test function signature semantics (pure and varargs).
 */

#include <az/types.h>
#include <az/convert.h>
#include <az/signature.h>
#include <az/function.h>

#include "unity/unity.h"

void test_signature(void)
{
    az_init();

    /* Fixed signatures (regression) */
    const AZFunctionSignature *void_f = az_function_signature_new_va (0, AZ_TYPE_NONE, 0);
    const AZFunctionSignature *void_ii = az_function_signature_new_va (0, AZ_TYPE_NONE, 2, AZ_TYPE_INT32, AZ_TYPE_INT32);
    const AZFunctionSignature *void_ia = az_function_signature_new_va (0, AZ_TYPE_NONE, 2, AZ_TYPE_INT32, AZ_TYPE_ANY);
    const AZFunctionSignature *void_ai = az_function_signature_new_va (0, AZ_TYPE_NONE, 2, AZ_TYPE_ANY, AZ_TYPE_INT32);
    const AZFunctionSignature *ret_ii = az_function_signature_new_va (0, AZ_TYPE_INT32, 2, AZ_TYPE_INT32, AZ_TYPE_INT32);
    /* Pure */
    const AZFunctionSignature *pure_ii = az_function_signature_new_va (AZ_SIGNATURE_FLAG_PURE, AZ_TYPE_NONE, 2, AZ_TYPE_INT32, AZ_TYPE_INT32);
    /* Varargs (last declared argument is the tail type) */
    const AZFunctionSignature *va_only_i = az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 1, AZ_TYPE_INT32);
    const AZFunctionSignature *va_only_a = az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 1, AZ_TYPE_ANY);
    const AZFunctionSignature *va_ii = az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 2, AZ_TYPE_INT32, AZ_TYPE_INT32);
    const AZFunctionSignature *va_ia = az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 2, AZ_TYPE_INT32, AZ_TYPE_ANY);
    const AZFunctionSignature *va_ai = az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 2, AZ_TYPE_ANY, AZ_TYPE_INT32);
    const AZFunctionSignature *pure_va_i = az_function_signature_new_va (AZ_SIGNATURE_FLAG_PURE | AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 1, AZ_TYPE_INT32);

    TEST_ASSERT_NOT_NULL (void_f);
    TEST_ASSERT_NOT_NULL (void_ii);
    TEST_ASSERT_NOT_NULL (va_only_i);

    /* A varargs signature needs the tail type (prints an expected fail warning) */
    TEST_ASSERT_NULL (az_function_signature_new_va (AZ_SIGNATURE_FLAG_VA_ARGS, AZ_TYPE_NONE, 0));

    /* Flag queries */
    TEST_ASSERT_FALSE (AZ_SIGNATURE_IS_PURE (void_ii));
    TEST_ASSERT_TRUE (AZ_SIGNATURE_IS_PURE (pure_ii));
    TEST_ASSERT_FALSE (AZ_SIGNATURE_IS_VA_ARGS (void_ii));
    TEST_ASSERT_TRUE (AZ_SIGNATURE_IS_VA_ARGS (va_ii));
    TEST_ASSERT_EQUAL_UINT (2, AZ_SIGNATURE_N_FIXED_ARGS (void_ii));
    TEST_ASSERT_EQUAL_UINT (1, AZ_SIGNATURE_N_FIXED_ARGS (va_ii));
    TEST_ASSERT_EQUAL_UINT (0, AZ_SIGNATURE_N_FIXED_ARGS (va_only_i));
    TEST_ASSERT_EQUAL_UINT (AZ_TYPE_INT32, AZ_SIGNATURE_VA_ARG_TYPE (va_ii));

    /* Fixed assignability (regression) */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (void_ii, void_ii, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (void_f, void_f, 1));
    /* Arguments are contravariant: impl (int32, any) serves slot (int32, int32) */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (void_ia, void_ii, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, void_ia, 1));
    /* Argument counts have to match */
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, void_f, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_f, void_ii, 1));
    /* Return value is covariant - a returning function serves a void slot */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (ret_ii, void_ii, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, ret_ii, 1));
    /* Return value is ignored when not tested */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (void_ii, ret_ii, 0));

    /* Pure: the implementation has to be at least as pure as the slot */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (pure_ii, pure_ii, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (pure_ii, void_ii, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, pure_ii, 1));

    /* A fixed implementation cannot serve a varargs slot */
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, va_ii, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (void_ii, va_only_i, 1));
    /* A varargs implementation can serve a fixed slot with enough arguments */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_ii, void_ii, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_only_i, void_ii, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_ia, void_ii, 1));
    /* Extra slot arguments have to fit the varargs tail */
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_ii, void_ia, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_only_i, void_ia, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_only_a, void_ia, 1));
    /* Varargs tail is contravariant */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_only_i, va_only_i, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_only_a, va_only_i, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_only_i, va_only_a, 1));
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (va_ai, va_ii, 1));
    /* The implementation cannot require more fixed arguments than the slot passes */
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_ii, va_only_i, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_ia, va_only_i, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_ii, va_only_a, 1));
    /* Pure applies to varargs signatures too */
    TEST_ASSERT_TRUE (az_function_signature_is_assignable_to (pure_va_i, va_only_i, 1));
    TEST_ASSERT_FALSE (az_function_signature_is_assignable_to (va_only_i, pure_va_i, 1));

    /* Fixed callability (regression) */
    const unsigned int args_ii[] = { AZ_TYPE_INT32, AZ_TYPE_INT32 };
    const unsigned int args_iii[] = { AZ_TYPE_INT32, AZ_TYPE_INT32, AZ_TYPE_INT32 };
    const unsigned int args_is[] = { AZ_TYPE_INT32, AZ_TYPE_STRING };
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (void_ii, AZ_TYPE_NONE, 2, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (void_ii, AZ_TYPE_NONE, 1, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (void_ii, AZ_TYPE_NONE, 3, args_iii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (void_ii, AZ_TYPE_NONE, 2, args_is));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (void_ii, AZ_TYPE_INT32, 2, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (ret_ii, AZ_TYPE_INT32, 2, args_ii));

    /* Varargs callability: the tail accepts 0 or more arguments */
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_only_i, AZ_TYPE_NONE, 0, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_only_i, AZ_TYPE_NONE, 1, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_only_i, AZ_TYPE_NONE, 2, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (va_only_i, AZ_TYPE_NONE, 2, args_is));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_ii, AZ_TYPE_NONE, 1, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_ii, AZ_TYPE_NONE, 2, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CONVERT_SAME, az_function_signature_is_callable_as (va_ii, AZ_TYPE_NONE, 3, args_iii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (va_ii, AZ_TYPE_NONE, 0, args_ii));
    TEST_ASSERT_EQUAL_UINT (AZ_CANNOT_CONVERT, az_function_signature_is_callable_as (va_ii, AZ_TYPE_NONE, 2, args_is));
}
