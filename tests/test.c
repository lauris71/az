#define __TEST_C__

#include <stdio.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/boxed-value.h>
#include <az/types.h>
#include <az/value.h>
#include <az/collections/array-list.h>

#include "unity/unity.h"

static void test_types();
static void test_boxed_value();
static void test_array_list();

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

int
main(int argc, const char *argv[])
{
    UNITY_BEGIN();
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "types")) {
            RUN_TEST(test_types);
        } else if (!strcmp(argv[i], "boxed-value")) {
            RUN_TEST(test_boxed_value);
        } else if (!strcmp(argv[i], "array-list")) {
            RUN_TEST(test_array_list);
        }
    }
    return UNITY_END();
}

typedef struct {
    unsigned int parent;
    unsigned int base : 1;
    unsigned int primitive : 1;
    unsigned int arithmetic : 1;
    unsigned int integral : 1;
} TypeDef;

static const TypeDef defs[] = {
    // None
    {0},
    // Any
    {0,                 1, 0, 0, 0},
    // Boolean
    {AZ_TYPE_ANY,       1, 1, 0, 0},
    // int8, uint8
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    // int16, uint16
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    // int32, uint32
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    // int64, uint64
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    {AZ_TYPE_ANY,       1, 1, 1, 1},
    // float, double
    {AZ_TYPE_ANY,       1, 1, 1, 0},
    {AZ_TYPE_ANY,       1, 1, 1, 0},
    // complex float, complex double
    {AZ_TYPE_ANY,       1, 1, 1, 0},
    {AZ_TYPE_ANY,       1, 1, 1, 0},
    // pointer
    {AZ_TYPE_ANY,       1, 1, 0, 0},
    // struct
	{AZ_TYPE_ANY,       1, 0, 0, 0},
	// block
	{AZ_TYPE_ANY,       1, 0, 0, 0},
    // implementation, class, interface
	{AZ_TYPE_BLOCK,     1, 0, 0, 0},
	{AZ_TYPE_IMPLEMENTATION,     1, 0, 0, 0},
	{AZ_TYPE_BLOCK,     1, 0, 0, 0},
    // field
    {AZ_TYPE_BLOCK,     1, 0, 0, 0},
    // signature, function
	{AZ_TYPE_BLOCK,     1, 0, 0, 0},
	{AZ_TYPE_INTERFACE, 1, 0, 0, 0},
	// reference
	{AZ_TYPE_BLOCK,     1, 0, 0, 0},
    // string
	{AZ_TYPE_REFERENCE, 1, 0, 0, 0},
    // boxed value
	{AZ_TYPE_REFERENCE, 1, 0, 0, 0},
    // boxed interface
	{AZ_TYPE_REFERENCE, 1, 0, 0, 0},
    // packed value
    {AZ_TYPE_BLOCK,     1, 0, 0, 0},
    // object
	{AZ_TYPE_REFERENCE, 1, 0, 0, 0}
};
#define NUM_DEFS (sizeof(defs) / sizeof(defs[0]))

static void
test_types()
{
    az_init();
    for (int i = 1; i < NUM_DEFS; i++) {
        fprintf(stderr, "%3d ", i);
        const AZClass *klass = AZ_CLASS_FROM_TYPE(i);
        TEST_ASSERT(klass);
        fprintf(stderr, "%20s", klass->name);
        TEST_ASSERT(klass->impl.type == i);
        const AZClass *parent = (defs[i].parent) ? AZ_CLASS_FROM_TYPE(defs[i].parent) : NULL;
        TEST_ASSERT(klass->parent == parent);
        TEST_ASSERT(AZ_TYPE_IS_BASE(i) == defs[i].base);
        TEST_ASSERT(AZ_TYPE_IS_PRIMITIVE(i) == defs[i].primitive);
        TEST_ASSERT(AZ_TYPE_IS_ARITHMETIC(i) == defs[i].arithmetic);
        TEST_ASSERT(AZ_TYPE_IS_ARITHMETIC(i) == ((klass->impl.flags & AZ_FLAG_ARITHMETIC) != 0));
        TEST_ASSERT(AZ_TYPE_IS_INTEGRAL(i) == defs[i].integral);
        TEST_ASSERT(AZ_TYPE_IS_INTEGRAL(i) == ((klass->impl.flags & AZ_FLAG_INTEGRAL) != 0));
        const AZImplementation *impl;
        AZValue64 val;
        TEST_ASSERT(az_instance_get_property(&klass->impl, NULL, (const uint8_t *) "isArithmetic", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_ARITHMETIC(i));
        fprintf(stderr, " arithmetic %d", val.value.boolean_v);
        TEST_ASSERT(az_instance_get_property(&klass->impl, NULL, (const uint8_t *) "isIntegral", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_INTEGRAL(i));
        fprintf(stderr, " integral %d", val.value.boolean_v);
        TEST_ASSERT(az_instance_get_property(&klass->impl, NULL, (const uint8_t *) "isSigned", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_SIGNED(i));
        fprintf(stderr, " signed %d\n", val.value.boolean_v);
    }
}

static void
test_boxed_value()
{
    az_init();
    AZValue val0, val1;
    val0.cdouble_v = (AZComplexDouble) {1.0, -1.0};
    const AZImplementation *impl = az_value_copy_autobox(&AZComplexDoubleKlass.impl, &val1, &val0, 16);
    TEST_ASSERT(impl == &AZComplexDoubleKlass.impl);
    impl = az_value_copy_autobox(impl, &val0, &val1, 8);
    TEST_ASSERT(impl == &AZBoxedValueKlass.klass.impl);
    impl = az_value_copy_autobox(impl, &val1, &val0, 8);
    TEST_ASSERT(impl == &AZBoxedValueKlass.klass.impl);
    impl = az_value_copy_autobox(impl, &val0, &val1, 16);
    TEST_ASSERT(impl == &AZComplexDoubleKlass.impl);
    TEST_ASSERT(val0.cdouble_v.r == 1.0);
    TEST_ASSERT(val0.cdouble_v.i == -1.0);
}

static void
test_array_list()
{
    AZArrayList *alist = az_array_list_new(AZ_TYPE_ANY, 8);
    for (unsigned int i = 0; i < 10; i++) {
        uint32_t inst = i;
        TEST_ASSERT(az_array_list_append(alist, &AZUint32Klass.impl, &inst));
    }
    for (unsigned int i = 0; i < 10; i++) {
        AZValue val;
        const AZImplementation *impl = az_list_get_element(&AZArrayListKlass->list_implementation, alist, i, &val, 16);
        TEST_ASSERT(impl == &AZUint32Klass.impl);
        TEST_ASSERT(val.uint32_v == i);
    }
}

