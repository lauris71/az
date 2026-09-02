#define __CALL_NATIVE_TEST_C__

/*
 * Test calling native C functions from AZ.
 */

#include <string.h>

#include <az/types.h>
#include <az/complex.h>
#include <az/function.h>
#include <az/function-native.h>
#include <az/function-value.h>
#include <az/value.h>
#include <az/string.h>

#include "unity/unity.h"

/*
 * az_function_call_native (ARM64)
 */

static int32_t native_add_i32 (int32_t a, int32_t b)
{
    return a + b;
}

static float native_add_f (float a, float b)
{
    return a + b;
}

static double native_add_d (double a, double b)
{
    return a + b;
}

static int64_t native_mixed (int32_t a, double b, int64_t c, float d)
{
    return a + (int64_t) b + c + (int64_t) d;
}

static AZComplexFloat native_add_cf (AZComplexFloat a, AZComplexFloat b)
{
    AZComplexFloat r;
    r.r = a.r + b.r;
    r.i = a.i + b.i;
    return r;
}

static AZComplexDouble native_add_cd (AZComplexDouble a, AZComplexDouble b)
{
    AZComplexDouble r;
    r.r = a.r + b.r;
    r.i = a.i + b.i;
    return r;
}

/* More than 8 general purpose arguments (stack spill) */
static int64_t native_sum10 (int64_t a, int64_t b, int64_t c, int64_t d, int64_t e, int64_t f, int64_t g, int64_t h, int64_t i, int64_t j)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h + 9 * i + 10 * j;
}

/* More than 8 FP arguments (stack spill) */
static double native_sum10d (double a, double b, double c, double d, double e, double f, double g, double h, double i, double j)
{
    return a + b + c + d + e + f + g + h + i + j;
}

/* HFA does not fit into the remaining FP registers and goes to the stack */
static double native_7d_cf (double a, double b, double c, double d, double e, double f, double g, AZComplexFloat h)
{
    return a + b + c + d + e + f + g + h.r + h.i;
}

static void *native_id_ptr (void *p)
{
    return p;
}

static const AZImplementation *nf_arg_impl;
static const void *nf_arg_inst;

static int64_t native_nonfinal_arg (const AZImplementation *impl, const void *inst)
{
    return (impl == nf_arg_impl && inst == nf_arg_inst) ? 7 : 0;
}

static void *nf_ret_ptr;

static const AZImplementation *native_ret_nonfinal_block (void **ret)
{
    *ret = nf_ret_ptr;
    return AZ_IMPL_FROM_TYPE (AZ_TYPE_STRING);
}

static const AZImplementation *native_ret_any_value (void *ret)
{
    *(int32_t *) ret = 4242;
    return AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32);
}

static void native_make_fval (void *ret)
{
    AZFunctionValue *fv = (AZFunctionValue *) ret;
    fv->signature = (AZFunctionSignature *) (uintptr_t) 0xdeadbeef;
    fv->invoke = NULL;
}

static uint64_t native_fval_sig (const AZFunctionValue *fv)
{
    return (uint64_t) (uintptr_t) fv->signature;
}

static unsigned int
call_native (void (*func) (void), unsigned int ret_type, unsigned int n_args, const unsigned int *arg_types, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation **arg_impls, const AZValue **arg_vals)
{
    AZFunctionSignature32 sig32;
    sig32.ret_type = ret_type;
    sig32.n_args = n_args;
    if (n_args) memcpy (sig32.arg_types, arg_types, n_args * sizeof (unsigned int));
    return az_function_call_native (func, &sig32.signature, ret_impl, ret_val, arg_impls, arg_vals);
}

void
test_call_native()
{
    const AZImplementation *ret_impl;
    AZValue64 ret_val;

    az_init();

    /* int32 (int32, int32) */
    {
        unsigned int types[2] = {AZ_TYPE_INT32, AZ_TYPE_INT32};
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32), AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        a.int32_v = 17;
        b.int32_v = 25;
        ret_impl = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_add_i32, AZ_TYPE_INT32, 2, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32));
        TEST_ASSERT_EQUAL_INT32 (42, ret_val.value.int32_v);
    }
    /* float (float, float) */
    {
        unsigned int types[2] = {AZ_TYPE_FLOAT, AZ_TYPE_FLOAT};
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_FLOAT), AZ_IMPL_FROM_TYPE (AZ_TYPE_FLOAT)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        a.float_v = 1.5f;
        b.float_v = 2.25f;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_add_f, AZ_TYPE_FLOAT, 2, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_FLOAT));
        TEST_ASSERT_EQUAL_FLOAT (3.75f, ret_val.value.float_v);
    }
    /* double (double, double) */
    {
        unsigned int types[2] = {AZ_TYPE_DOUBLE, AZ_TYPE_DOUBLE};
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE), AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        a.double_v = 1.5;
        b.double_v = 2.25;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_add_d, AZ_TYPE_DOUBLE, 2, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE));
        TEST_ASSERT_EQUAL_DOUBLE (3.75, ret_val.value.double_v);
    }
    /* int64 (int32, double, int64, float) - mixed GPR/FPR allocation */
    {
        unsigned int types[4] = {AZ_TYPE_INT32, AZ_TYPE_DOUBLE, AZ_TYPE_INT64, AZ_TYPE_FLOAT};
        const AZImplementation *impls[4] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32), AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE), AZ_IMPL_FROM_TYPE (AZ_TYPE_INT64), AZ_IMPL_FROM_TYPE (AZ_TYPE_FLOAT)};
        AZValue a, b, c, d;
        const AZValue *vals[4] = {&a, &b, &c, &d};
        a.int32_v = 1;
        b.double_v = 2.7;
        c.int64_v = 3;
        d.float_v = 4.9f;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_mixed, AZ_TYPE_INT64, 4, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_INT64 (10, ret_val.value.int64_v);
    }
    /* complex float (complex float, complex float) - HFA arguments and return */
    {
        unsigned int types[2] = {AZ_TYPE_COMPLEX_FLOAT, AZ_TYPE_COMPLEX_FLOAT};
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_FLOAT), AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_FLOAT)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        a.cfloat_v.r = 1.0f;
        a.cfloat_v.i = 2.0f;
        b.cfloat_v.r = 3.0f;
        b.cfloat_v.i = 4.0f;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_add_cf, AZ_TYPE_COMPLEX_FLOAT, 2, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_FLOAT));
        TEST_ASSERT_EQUAL_FLOAT (4.0f, ret_val.value.cfloat_v.r);
        TEST_ASSERT_EQUAL_FLOAT (6.0f, ret_val.value.cfloat_v.i);
    }
    /* complex double (complex double, complex double) - HFA arguments and return */
    {
        unsigned int types[2] = {AZ_TYPE_COMPLEX_DOUBLE, AZ_TYPE_COMPLEX_DOUBLE};
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_DOUBLE), AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_DOUBLE)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        a.cdouble_v.r = 1.5;
        a.cdouble_v.i = 2.5;
        b.cdouble_v.r = 3.25;
        b.cdouble_v.i = 4.75;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_add_cd, AZ_TYPE_COMPLEX_DOUBLE, 2, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_DOUBLE));
        TEST_ASSERT_EQUAL_DOUBLE (4.75, ret_val.value.cdouble_v.r);
        TEST_ASSERT_EQUAL_DOUBLE (7.25, ret_val.value.cdouble_v.i);
    }
    /* int64 (10 x int64) - GPR stack spill */
    {
        unsigned int types[10];
        const AZImplementation *impls[10];
        AZValue v[10];
        const AZValue *vals[10];
        for (int k = 0; k < 10; k++) {
            types[k] = AZ_TYPE_INT64;
            impls[k] = AZ_IMPL_FROM_TYPE (AZ_TYPE_INT64);
            v[k].int64_v = k + 1;
            vals[k] = &v[k];
        }
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_sum10, AZ_TYPE_INT64, 10, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_INT64 (385, ret_val.value.int64_v);
    }
    /* double (10 x double) - FPR stack spill */
    {
        unsigned int types[10];
        const AZImplementation *impls[10];
        AZValue v[10];
        const AZValue *vals[10];
        for (int k = 0; k < 10; k++) {
            types[k] = AZ_TYPE_DOUBLE;
            impls[k] = AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE);
            v[k].double_v = k + 1;
            vals[k] = &v[k];
        }
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_sum10d, AZ_TYPE_DOUBLE, 10, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_DOUBLE (55.0, ret_val.value.double_v);
    }
    /* double (7 x double, complex float) - HFA passed on the stack */
    {
        unsigned int types[8];
        const AZImplementation *impls[8];
        AZValue v[8];
        const AZValue *vals[8];
        for (int k = 0; k < 7; k++) {
            types[k] = AZ_TYPE_DOUBLE;
            impls[k] = AZ_IMPL_FROM_TYPE (AZ_TYPE_DOUBLE);
            v[k].double_v = k + 1;
            vals[k] = &v[k];
        }
        types[7] = AZ_TYPE_COMPLEX_FLOAT;
        impls[7] = AZ_IMPL_FROM_TYPE (AZ_TYPE_COMPLEX_FLOAT);
        v[7].cfloat_v.r = 8.0f;
        v[7].cfloat_v.i = 9.0f;
        vals[7] = &v[7];
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_7d_cf, AZ_TYPE_DOUBLE, 8, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_DOUBLE (45.0, ret_val.value.double_v);
    }
    /* string (string) - final block argument, returned by pointer */
    {
        AZString *str = az_string_new ((const unsigned char *) "hello");
        unsigned int types[1] = {AZ_TYPE_STRING};
        const AZImplementation *impls[1] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_STRING)};
        AZValue a;
        const AZValue *vals[1] = {&a};
        a.block = str;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_id_ptr, AZ_TYPE_STRING, 1, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_STRING));
        TEST_ASSERT (ret_val.value.block == str);
        az_string_unref (str);
    }
    /* int64 (block) - non-final argument [impl, pointer] */
    {
        AZString *str = az_string_new ((const unsigned char *) "hello");
        unsigned int types[1] = {AZ_TYPE_BLOCK};
        const AZImplementation *impls[1] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_STRING)};
        AZValue a;
        const AZValue *vals[1] = {&a};
        a.block = str;
        nf_arg_impl = impls[0];
        nf_arg_inst = str;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_nonfinal_arg, AZ_TYPE_INT64, 1, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_INT64 (7, ret_val.value.int64_v);
        az_string_unref (str);
    }
    /* block () - non-final block return, impl returned, block written through hidden argument */
    {
        AZString *str = az_string_new ((const unsigned char *) "hello");
        nf_ret_ptr = str;
        ret_impl = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_ret_nonfinal_block, AZ_TYPE_BLOCK, 0, NULL, &ret_impl, &ret_val, NULL, NULL));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_STRING));
        TEST_ASSERT (ret_val.value.block == str);
        az_string_unref (str);
    }
    /* any () - non-final value return, impl returned, struct written to hidden storage */
    {
        ret_impl = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_ret_any_value, AZ_TYPE_ANY, 0, NULL, &ret_impl, &ret_val, NULL, NULL));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32));
        TEST_ASSERT_EQUAL_INT32 (4242, ret_val.value.int32_v);
    }
    /* FunctionValue () - final value return, void, struct written to hidden storage */
    {
        unsigned int fv_type = az_function_value_get_type();
        ret_impl = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_make_fval, fv_type, 0, NULL, &ret_impl, &ret_val, NULL, NULL));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (fv_type));
        TEST_ASSERT (((AZFunctionValue *) &ret_val.value)->signature == (AZFunctionSignature *) (uintptr_t) 0xdeadbeef);
        TEST_ASSERT (((AZFunctionValue *) &ret_val.value)->invoke == NULL);
    }
    /* uint64 (FunctionValue) - final value argument, passed by pointer */
    {
        unsigned int fv_type = az_function_value_get_type();
        unsigned int types[1] = {fv_type};
        const AZImplementation *impls[1] = {AZ_IMPL_FROM_TYPE (fv_type)};
        AZValue a;
        const AZValue *vals[1] = {&a};
        AZFunctionValue *fv = (AZFunctionValue *) &a;
        fv->signature = (AZFunctionSignature *) (uintptr_t) 0xdeadbeef;
        fv->invoke = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        TEST_ASSERT (call_native ((void (*) (void)) native_fval_sig, AZ_TYPE_UINT64, 1, types, &ret_impl, &ret_val, impls, vals));
        TEST_ASSERT_EQUAL_UINT64 (0xdeadbeef, ret_val.value.uint64_v);
    }
    /* AZFunctionNative - native function invoked through the AZFunction interface */
    {
        unsigned int fn_type = az_function_native_get_type();
        unsigned int arg_types[2] = {AZ_TYPE_INT32, AZ_TYPE_INT32};
        AZFunctionSignature *sig = az_function_signature_new (0, AZ_TYPE_INT32, 2, arg_types);
        AZFunctionNative fnat;
        const AZImplementation *f_impl;
        void *f_inst;
        const AZImplementation *impls[2] = {AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32), AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32)};
        AZValue a, b;
        const AZValue *vals[2] = {&a, &b};
        az_function_native_setup (&fnat, sig, (void (*) (void)) native_add_i32);
        a.int32_v = 17;
        b.int32_v = 25;
        ret_impl = NULL;
        memset (&ret_val, 0, sizeof (AZValue64));
        f_impl = az_instance_get_interface_from_type (fn_type, &fnat, AZ_TYPE_FUNCTION, &f_inst);
        TEST_ASSERT (f_impl != NULL);
        TEST_ASSERT (az_function_invoke ((const AZFunctionImplementation *) f_impl, f_inst, impls, vals, &ret_impl, &ret_val, NULL));
        TEST_ASSERT (ret_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_INT32));
        TEST_ASSERT_EQUAL_INT32 (42, ret_val.value.int32_v);
        az_function_signature_delete (sig);
    }
}
