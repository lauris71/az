#define __TEST_C__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/boxed-value.h>
#include <az/extend.h>
#include <az/packed-value.h>
#include <az/reference-of.h>
#include <az/string.h>
#include <az/types.h>
#include <az/value.h>
#include <az/collections/array-list.h>
#include <az/collections/array.h>
#include <az/classes/array-object.h>
#include <az/collections/hash-map.h>
#include <az/collections/hash-set.h>

#include "unity/unity.h"

static void test_types();
static void test_to_string();
static void test_boxed_value();
static void test_array_list();
static void test_array();

void test_hash_map(void);
void test_hash_set(void);

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
        } else if (!strcmp(argv[i], "to-string")) {
            RUN_TEST(test_to_string);
        } else if (!strcmp(argv[i], "boxed-value")) {
            RUN_TEST(test_boxed_value);
        } else if (!strcmp(argv[i], "array-list")) {
            RUN_TEST(test_array_list);
        } else if (!strcmp(argv[i], "array")) {
            RUN_TEST(test_array);
        } else if (!strcmp(argv[i], "hash-map")) {
            RUN_TEST(test_hash_map);
        } else if (!strcmp(argv[i], "hash-set")) {
            RUN_TEST(test_hash_set);
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
        TEST_ASSERT(AZ_TYPE_INDEX(klass->impl.type) == i);
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
        TEST_ASSERT(az_instance_get_property_by_key(&klass->impl, NULL, (const uint8_t *) "isArithmetic", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_ARITHMETIC(i));
        fprintf(stderr, " arithmetic %d", val.value.boolean_v);
        TEST_ASSERT(az_instance_get_property_by_key(&klass->impl, NULL, (const uint8_t *) "isIntegral", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_INTEGRAL(i));
        fprintf(stderr, " integral %d", val.value.boolean_v);
        TEST_ASSERT(az_instance_get_property_by_key(&klass->impl, NULL, (const uint8_t *) "isSigned", &impl, &val));
        TEST_ASSERT(val.value.boolean_v == AZ_TYPE_IS_SIGNED(AZ_TYPE_FROM_INDEX(i)));
        fprintf(stderr, " signed %d\n", val.value.boolean_v);
    }
}

/*
 * Verify the to_string contract:
 * - NULL destination is accepted (nothing written, required length returned)
 * - at most dlen bytes are written
 * - the required string length (excluding the terminating 0) is returned
 * - the string is terminated with 0 only if there is extra room
 */
static void
check_to_string(const AZImplementation *impl, void *inst, const char *expected)
{
    unsigned int expected_len = (unsigned int) strlen(expected);
    uint8_t buf[256];
    /* NULL destination - returns the required length, writes nothing (any dlen) */
    TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, NULL, 0));
    TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, NULL, 128));
    /* Big buffer - full string, terminated, nothing past the terminator */
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING(expected, (const char *) buf);
    TEST_ASSERT_EQUAL_UINT8(0xAA, buf[expected_len + 1]);
    /* Exact length - full string, no terminator, nothing past the buffer end */
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, buf, expected_len));
    TEST_ASSERT_EQUAL_MEMORY(expected, buf, expected_len);
    TEST_ASSERT_EQUAL_UINT8(0xAA, buf[expected_len]);
    /* Exactly room for the terminator */
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, buf, expected_len + 1));
    TEST_ASSERT_EQUAL_STRING(expected, (const char *) buf);
    TEST_ASSERT_EQUAL_UINT8(0xAA, buf[expected_len + 1]);
    /* Truncated - required length returned, only dlen bytes written, no terminator */
    if (expected_len > 0) {
        memset(buf, 0xAA, sizeof(buf));
        TEST_ASSERT_EQUAL_UINT(expected_len, az_instance_to_string(impl, inst, buf, expected_len - 1));
        TEST_ASSERT_EQUAL_MEMORY(expected, buf, expected_len - 1);
        TEST_ASSERT_EQUAL_UINT8(0xAA, buf[expected_len - 1]);
    }
}

static void
test_to_string()
{
    az_init();
    unsigned int b = 1;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_BOOLEAN), &b, "True");
    b = 0;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_BOOLEAN), &b, "False");
    int8_t i8 = -128;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_INT8), &i8, "-128");
    uint8_t u8 = 255;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT8), &u8, "255");
    int32_t i32 = -123456;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_INT32), &i32, "-123456");
    uint32_t u32 = 123456;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT32), &u32, "123456");
    int64_t i64 = INT64_MIN;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_INT64), &i64, "-9223372036854775808");
    uint64_t u64 = UINT64_MAX;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT64), &u64, "18446744073709551615");
    float f = 1.5f;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_FLOAT), &f, "1.5000");
    double dbl = 1.5;
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_DOUBLE), &dbl, "1.5000000");
    AZComplexFloat cf = {1.5f, -2.5f};
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_COMPLEX_FLOAT), &cf, "1.5000-2.5000i");
    AZComplexDouble cd = {1.2, 3.4};
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_COMPLEX_DOUBLE), &cd, "1.2000000+3.4000000i");
    AZString *str = az_string_new((const unsigned char *) "Hello, world!");
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_STRING), str, "Hello, world!");
    az_string_unref(str);
    /* NULL string instance is an empty string */
    AZClass *str_klass = AZ_CLASS_FROM_TYPE(AZ_TYPE_STRING);
    uint8_t buf[16];
    memset(buf, 0xAA, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(0, str_klass->to_string(&str_klass->impl, NULL, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_UINT8(0, buf[0]);
    /* Fallback methods */
    check_to_string(AZ_IMPL_FROM_TYPE(AZ_TYPE_CLASS), (void *) &AZUint32Klass, "uint32 class");
    uint8_t any_buf[16];
    memset(any_buf, 0xAA, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(3, az_any_to_string(&AZAnyKlass.impl, NULL, NULL, 0));
    TEST_ASSERT_EQUAL_UINT(3, az_any_to_string(&AZAnyKlass.impl, NULL, any_buf, sizeof(any_buf)));
    TEST_ASSERT_EQUAL_STRING("Any", (const char *) any_buf);
    memset(any_buf, 0xAA, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(4, AZPointerKlass.to_string(&AZPointerKlass.impl, NULL, any_buf, sizeof(any_buf)));
    TEST_ASSERT_EQUAL_STRING("null", (const char *) any_buf);
    /* Multi-part builders must not write for NULL destination even with non-zero dlen */
    unsigned int any_len = az_any_to_string(&AZUint32Klass.impl, &u32, any_buf, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(any_len, az_any_to_string(&AZUint32Klass.impl, &u32, NULL, 128));
    unsigned int impl_len = AZImplKlass.to_string(&AZImplKlass.impl, &AZUint32Klass.impl, any_buf, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(impl_len, AZImplKlass.to_string(&AZImplKlass.impl, &AZUint32Klass.impl, NULL, 128));
    AZPackedValue pv = {0};
    az_packed_value_set_from_type(&pv, AZ_TYPE_UINT32, &u32);
    AZClass *pv_klass = AZ_CLASS_FROM_TYPE(AZ_TYPE_PACKED_VALUE);
    unsigned int pv_len = pv_klass->to_string(&pv_klass->impl, &pv, any_buf, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(pv_len, pv_klass->to_string(&pv_klass->impl, &pv, NULL, 128));
    unsigned int refof_type = az_reference_of_get_type(AZ_TYPE_UINT32);
    AZReferenceOf *ref = az_reference_of_new(AZ_TYPE_UINT32);
    AZClass *refof_klass = AZ_CLASS_FROM_TYPE(refof_type);
    unsigned int ref_len = refof_klass->to_string(&refof_klass->impl, ref, any_buf, sizeof(any_buf));
    TEST_ASSERT_EQUAL_UINT(ref_len, refof_klass->to_string(&refof_klass->impl, ref, NULL, 128));
    az_instance_delete(refof_type, ref);
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
verify_list(AZArrayList *alist, const unsigned int idx[], const unsigned int types[])
{
    void *coll_inst;
    const AZCollectionImplementation *coll_impl = (AZCollectionImplementation *) az_instance_get_interface((AZImplementation *) AZArrayListKlass, alist, AZ_TYPE_COLLECTION, &coll_inst);
    TEST_ASSERT(coll_impl == &AZArrayListKlass->list_impl.collection_impl);
    TEST_ASSERT(coll_inst == alist);
    unsigned int size = az_collection_get_size(coll_impl, coll_inst);
    TEST_ASSERT(size == alist->list.collection.size);
    for (unsigned int i = 0; i < size; i++) {
        uint8_t buf[256];
        memset(buf, (char) idx[i], 256);
        AZClass *klass = AZ_CLASS_FROM_TYPE(types[idx[i]]);
        AZValue val;
        const AZImplementation *impl = az_list_get_element(&AZArrayListKlass->list_impl, alist, i, &val, 16);
        if (klass->instance_size <= 16) {
            TEST_ASSERT(impl == &klass->impl);
            if (!klass->instance_size) continue;
            TEST_ASSERT(az_value_equals(&klass->impl, &val, (const AZValue *) buf));
        } else {
            TEST_ASSERT(impl == &AZBoxedValueKlass.klass.impl);
            AZBoxedValue *boxed = (AZBoxedValue *) val.block;
            TEST_ASSERT(boxed->klass == klass);
            TEST_ASSERT(az_value_equals(&boxed->klass->impl, &boxed->val, (const AZValue *) buf));
        }
    }
}

static void
print_list(AZArrayList *alist, FILE *ofs)
{
    fprintf(stdout, "List [val_size=%d length=%llu]:", alist->val_size, alist->list.collection.size);
    for (unsigned int i = 0; i < alist->list.collection.size; i++) {
        AZArrayListEntry *entry = az_array_list_get_entry(alist, i);
        fprintf (stdout, " %d", (entry->impl) ? AZ_IMPL_TYPE(entry->impl) : 0);
    }
    fprintf(stdout, "\n");
}

static void
test_array_list()
{
    unsigned int types[10];
    az_init();
    for (unsigned int i = 0; i < 10; i++) {
        unsigned int instance_size = 4 * i;
        uint8_t name[32];
        snprintf((char *) name, 32, "struct_%d", instance_size);
        AZClass *klass = az_register_type(&types[i], name, AZ_TYPE_STRUCT, sizeof(AZClass), instance_size, AZ_FLAG_FINAL, 0, 0, NULL, NULL, NULL);
        TEST_ASSERT(klass->instance_size == instance_size);
    }
    AZArrayList *alist = az_array_list_new(AZ_TYPE_ANY, 8);
    for (unsigned int i = 0; i < 10; i++) {
        uint32_t inst = i;
        TEST_ASSERT(az_array_list_append(alist, &AZUint32Klass.impl, &inst));
    }
    for (unsigned int i = 0; i < 10; i++) {
        AZValue val;
        const AZImplementation *impl = az_list_get_element(&AZArrayListKlass->list_impl, alist, i, &val, 16);
        TEST_ASSERT(impl == &AZUint32Klass.impl);
        TEST_ASSERT(val.uint32_v == i);
    }
    uint8_t buf[256] = {0};
    /* Trsy lists with element size 8...64 */
    for (unsigned int s = 8; s <= 64; s = s << 1) {
        /* Create new list with value_size s */
        alist = az_array_list_new(AZ_TYPE_ANY, s);
        /* Fill it with objects 0..9 */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) i, 256);
            TEST_ASSERT(az_array_list_append(alist, AZ_IMPL_FROM_TYPE(types[i]), &buf));
        }
        //print_list(alist, stdout);
        /* Verify */
        unsigned int *idx = (unsigned int[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        verify_list(alist, idx, types);
        /* Insert 9..0 into position 5 */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) i, 256);
            TEST_ASSERT(az_array_list_insert(alist, 5, AZ_IMPL_FROM_TYPE(types[i]), &buf));
        }
        //print_list(alist, stdout);
        /* Verify */
        idx = (unsigned int[]) {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 5, 6, 7, 8, 9};
        verify_list(alist, idx, types);
        /* Delete positions 1,3,5...*/
        for (unsigned int i = 0; i < 10; i++) {
            az_array_list_remove(alist, i + 1);
        }
        //print_list(alist, stdout);
        idx = (unsigned int[]) {0, 2, 4, 8, 6, 4, 2, 0, 6, 8};
        verify_list(alist, idx, types);
        /* Replace */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) (9 - i), 256);
            az_array_list_set_element(alist, i, AZ_IMPL_FROM_TYPE(types[9 - i]), &buf);
        }
        //print_list(alist, stdout);
        idx = (unsigned int[]) {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        verify_list(alist, idx, types);
        az_array_list_delete(alist);
    }
}

static void
test_array()
{
    uint32_t b32[1000];
    az_init();
    for (unsigned int i = 0; i < 1000; i++) b32[i] = i;
    AZArrayObject *aof = az_array_object_new_static(AZ_TYPE_UINT32, 1000, b32);
    void *inst;
    const AZListImplementation *impl = az_array_object_get_list(aof, &inst);
    for (unsigned int i = 0; i < 1000; i++) {
        AZValue val;
        const AZImplementation *el_impl = az_list_get_element(impl, inst, i, &val, 16);
        TEST_ASSERT(el_impl == &AZUint32Klass.impl);
        TEST_ASSERT(val.int32_v == b32[i]);
    }
}
