#define __TEST_C__

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include <arikkei/arikkei-threads.h>
#include <arikkei/arikkei-utils.h>

#include <az/az.h>
#include <az/base.h>
#include <az/boxed-interface.h>
#include <az/boxed-value.h>
#include <az/extend.h>
#include <az/function.h>
#include <az/function-native.h>
#include <az/function-value.h>
#include <az/interface.h>
#include <az/packed-value.h>
#include <az/reference-of.h>
#include <az/string.h>
#include <az/types.h>
#include <az/value.h>
#include <az/classes/active-object.h>
#include <az/classes/object-list.h>
#include <az/classes/weak-object-list.h>
#include <az/collections/array-list.h>
#include <az/collections/array.h>
#include <az/classes/array-object.h>
#include <az/collections/hash-map.h>
#include <az/collections/hash-set.h>
#include <az/classes/value-array.h>

#include "unity/unity.h"

static void test_compile();
static void test_types_mt();
static void test_to_string();
static void test_boxed_value();
static void test_array();
static void test_object_list();
static void test_masked_field();
static void test_masked_field_widths();
static void test_circular_reference();
static void test_strings_mt();
static void test_boxed_interface_conversion();
static void test_value_array();

void test_array_list();
void test_hash_map(void);
void test_hash_set(void);
void test_call_native();

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
            RUN_TEST(test_compile);
        } else if (!strcmp(argv[i], "types-mt")) {
            RUN_TEST(test_types_mt);
        } else if (!strcmp(argv[i], "to-string")) {
            RUN_TEST(test_to_string);
        } else if (!strcmp(argv[i], "boxed-value")) {
            RUN_TEST(test_boxed_value);
        } else if (!strcmp(argv[i], "array-list")) {
            RUN_TEST(test_array_list);
        } else if (!strcmp(argv[i], "array")) {
            RUN_TEST(test_array);
        } else if (!strcmp(argv[i], "call-native")) {
            RUN_TEST(test_call_native);
        } else if (!strcmp(argv[i], "object-list")) {
            RUN_TEST(test_object_list);
        } else if (!strcmp(argv[i], "masked-field")) {
            RUN_TEST(test_masked_field);
        } else if (!strcmp(argv[i], "masked-field-widths")) {
            RUN_TEST(test_masked_field_widths);
        } else if (!strcmp(argv[i], "circular-reference")) {
            RUN_TEST(test_circular_reference);
        } else if (!strcmp(argv[i], "strings-mt")) {
            RUN_TEST(test_strings_mt);
        } else if (!strcmp(argv[i], "boxed-interface-conversion")) {
            RUN_TEST(test_boxed_interface_conversion);
        } else if (!strcmp(argv[i], "value-array")) {
            RUN_TEST(test_value_array);
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
test_compile()
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
 * Race lazy type registration from multiple threads.
 *
 * Every get_type call has to return the same typecode in all threads, whether the
 * type was registered by this or another thread. The reference-of calls force the
 * subtype array to grow (realloc) while other threads are registering.
 */
#define MT_NUM_THREADS 8
#define MT_NUM_RESULTS 8

typedef struct {
    unsigned int seed;
    unsigned int results[MT_NUM_RESULTS];
} MTData;

static int
get_type_thread (void *arg)
{
    MTData *d = (MTData *) arg;
    static const unsigned int vtypes[4] = {AZ_TYPE_INT32, AZ_TYPE_UINT64, AZ_TYPE_FLOAT, AZ_TYPE_DOUBLE};
    d->results[0] = az_array_get_type();
    d->results[1] = az_hash_map_get_type();
    d->results[2] = az_active_object_get_type();
    d->results[3] = az_array_object_get_type();
    /* Register all subtypes in thread-dependent order, store indexed by element type */
    for (unsigned int k = 0; k < 4; k++) {
        unsigned int idx = (k + d->seed) % 4;
        d->results[4 + idx] = az_reference_of_get_type(vtypes[idx]);
    }
    return 0;
}

static void
test_types_mt()
{
    az_init();
    thrd_t threads[MT_NUM_THREADS];
    MTData data[MT_NUM_THREADS];
    for (int i = 0; i < MT_NUM_THREADS; i++) {
        data[i].seed = (unsigned int) i;
        TEST_ASSERT(thrd_create(&threads[i], get_type_thread, &data[i]) == thrd_success);
    }
    for (int i = 0; i < MT_NUM_THREADS; i++) {
        TEST_ASSERT(thrd_join(threads[i], NULL) == thrd_success);
    }
    /* All threads have to agree on every typecode */
    for (int r = 0; r < MT_NUM_RESULTS; r++) {
        TEST_ASSERT(data[0].results[r] != 0);
        for (int i = 1; i < MT_NUM_THREADS; i++) {
            TEST_ASSERT_EQUAL_UINT(data[0].results[r], data[i].results[r]);
        }
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
    /* Newly allocated string (fits the local buffer) */
    uint8_t *allocated = az_instance_to_string_new(impl, inst);
    TEST_ASSERT_EQUAL_STRING(expected, (const char *) allocated);
    free(allocated);
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
    /* Newly allocated string (does not fit the local buffer, rendered twice) */
    uint8_t long_buf[300];
    memset(long_buf, 'x', sizeof(long_buf) - 1);
    long_buf[sizeof(long_buf) - 1] = 0;
    AZString *long_str = az_string_new(long_buf);
    uint8_t *long_result = az_instance_to_string_new(AZ_IMPL_FROM_TYPE(AZ_TYPE_STRING), long_str);
    TEST_ASSERT_EQUAL_STRING((const char *) long_buf, (const char *) long_result);
    free(long_result);
    az_string_unref(long_str);
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
    /* Boxed values only hold values bigger than 16 bytes - register a big struct */
    typedef struct { double v[4]; } BigVal;
    static unsigned int big_type = 0;
    if (!big_type) {
        az_register_type(&big_type, (const unsigned char *) "BigVal", AZ_TYPE_STRUCT, sizeof(AZClass), sizeof(BigVal), AZ_FLAG_FINAL, 0, 0, NULL, NULL, NULL);
    }
    AZClass *big_klass = AZ_CLASS_FROM_TYPE(big_type);
    /* Small value (fits everywhere) is never boxed */
    AZValue val0, val1;
    val0.cdouble_v = (AZComplexDouble) {1.0, -1.0};
    const AZImplementation *impl = az_value_copy_autobox(&AZComplexDoubleKlass.impl, &val1, &val0, 16);
    TEST_ASSERT(impl == &AZComplexDoubleKlass.impl);
    TEST_ASSERT(val1.cdouble_v.r == 1.0 && val1.cdouble_v.i == -1.0);
    /* Big value is boxed into a small destination and unboxed back */
    AZValue big_src;
    az_value_init(&big_klass->impl, &big_src);
    ((BigVal *) &big_src)->v[0] = 3.0;
    ((BigVal *) &big_src)->v[3] = -7.0;
    AZValue big_dst;
    impl = az_value_copy_autobox(&big_klass->impl, &big_dst, &big_src, 16);
    TEST_ASSERT(impl == AZ_BOXED_VALUE_IMPL);
    TEST_ASSERT(big_dst.block != NULL);
    /* Copy back out of the box into a full-size value */
    AZValue64 big_back;
    impl = az_value_copy_autobox(AZ_BOXED_VALUE_IMPL, &big_back.value, &big_dst, 64);
    TEST_ASSERT(impl == &big_klass->impl);
    TEST_ASSERT(((BigVal *) &big_back)->v[0] == 3.0 && ((BigVal *) &big_back)->v[3] == -7.0);
    az_boxed_value_unref((AZBoxedValue *) big_dst.block);
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
    /* AZArray interface to_string prints [element,element...] */
    uint32_t av[3] = {1, 22, 333};
    AZArray arr = {0};
    arr.list.collection.size = 3;
    arr.values = av;
    check_to_string((const AZImplementation *) impl, &arr, "[1,22,333]");
    /* Empty array */
    AZArray empty = {0};
    check_to_string((const AZImplementation *) impl, &empty, "[]");
}

/*
 * AZObjectList and AZWeakObjectList
 */

static unsigned int test_active_object_type = 0;

static unsigned int
test_active_object_get_type (void)
{
    if (!test_active_object_type) {
        az_register_type (&test_active_object_type, (const unsigned char *) "TestActiveObject", AZ_TYPE_ACTIVE_OBJECT, sizeof (AZActiveObjectClass), sizeof (AZActiveObject), 0, 0, 0, NULL, NULL, NULL);
    }
    return test_active_object_type;
}

static void
test_object_list()
{
    az_init();
    unsigned int ao_type = test_active_object_get_type();

    /* Strong list */
    {
        AZObjectList *list = az_object_list_new (ao_type);
        AZActiveObject *o1 = (AZActiveObject *) az_object_new (ao_type);
        AZActiveObject *o2 = (AZActiveObject *) az_object_new (ao_type);
        AZActiveObject *o3 = (AZActiveObject *) az_object_new (ao_type);
        az_object_list_append_object (list, (AZObject *) o1);
        az_object_list_append_object (list, (AZObject *) o2);
        TEST_ASSERT_EQUAL_UINT (2, list->list.collection.size);
        /* The list holds strong references */
        TEST_ASSERT_EQUAL_UINT (2, o1->object.reference.refcount);
        az_object_list_insert_object (list, (AZObject *) o3, 1);
        TEST_ASSERT_EQUAL_UINT (3, list->list.collection.size);
        TEST_ASSERT (list->objects[1] == (AZObject *) o3);
        TEST_ASSERT (az_object_list_contains (list, (AZObject *) o2));
        az_object_list_remove_object (list, (AZObject *) o3);
        TEST_ASSERT_EQUAL_UINT (2, list->list.collection.size);
        TEST_ASSERT_EQUAL_UINT (1, o3->object.reference.refcount);
        az_object_list_remove_object_by_index (list, 0);
        TEST_ASSERT_EQUAL_UINT (1, list->list.collection.size);
        TEST_ASSERT (list->objects[0] == (AZObject *) o2);
        az_object_list_clear (list);
        TEST_ASSERT_EQUAL_UINT (0, list->list.collection.size);
        TEST_ASSERT_EQUAL_UINT (1, o2->object.reference.refcount);
        az_object_list_delete (list);
        az_object_unref ((AZObject *) o1);
        az_object_unref ((AZObject *) o2);
        az_object_unref ((AZObject *) o3);
    }
    /* Weak list */
    {
        AZWeakObjectList *list = az_weak_object_list_new (AZ_TYPE_ACTIVE_OBJECT);
        AZActiveObject *o1 = (AZActiveObject *) az_object_new (ao_type);
        AZActiveObject *o2 = (AZActiveObject *) az_object_new (ao_type);
        az_weak_object_list_append_object (list, o1);
        az_weak_object_list_append_object (list, o2);
        TEST_ASSERT_EQUAL_UINT (2, list->list.collection.size);
        /* The list does not hold strong references */
        TEST_ASSERT_EQUAL_UINT (1, o1->object.reference.refcount);
        TEST_ASSERT (az_weak_object_list_contains (list, o1));
        /* Explicit removal */
        az_weak_object_list_remove_object (list, o1);
        TEST_ASSERT_EQUAL_UINT (1, list->list.collection.size);
        /* Shutdown removes the object from the list automatically (and frees it) */
        az_object_shutdown ((AZObject *) o2);
        TEST_ASSERT_EQUAL_UINT (0, list->list.collection.size);
        az_weak_object_list_delete (list);
        az_object_unref ((AZObject *) o1);
    }
}

/*
 * Masked (bit-field) property read/write
 */

typedef struct {
    uint32_t flags;
} MaskedInst;

#define MASKED_NUM_PROPS 4
#define MASKED_PROP_BOOL 0
#define MASKED_PROP_BOOL_INV 1
#define MASKED_PROP_UINT3 2
#define MASKED_PROP_UINT8 3

static unsigned int masked_type = 0;

static void
masked_class_init (AZClass *klass)
{
    /* Boolean at bit 0 (stored in a uint32_t) */
    az_class_define_property_value (klass, MASKED_PROP_BOOL, (const uint8_t *) "flag", AZ_TYPE_BOOLEAN, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedInst, flags));
    klass->props_self[MASKED_PROP_BOOL].value_type_idx = AZ_TYPE_IDX_UINT32;
    klass->props_self[MASKED_PROP_BOOL].shift = 0;
    klass->props_self[MASKED_PROP_BOOL].mask_width = 1;
    klass->props_self[MASKED_PROP_BOOL].bits = 0;
    /* Inverted boolean at bit 1 (stored value is XORed with 1 on read) */
    az_class_define_property_value (klass, MASKED_PROP_BOOL_INV, (const uint8_t *) "flagInv", AZ_TYPE_BOOLEAN, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedInst, flags));
    klass->props_self[MASKED_PROP_BOOL_INV].value_type_idx = AZ_TYPE_IDX_UINT32;
    klass->props_self[MASKED_PROP_BOOL_INV].shift = 1;
    klass->props_self[MASKED_PROP_BOOL_INV].mask_width = 1;
    klass->props_self[MASKED_PROP_BOOL_INV].bits = 1;
    /* 3-bit unsigned integer at bits 2-4 (stored in a uint32_t) */
    az_class_define_property_value (klass, MASKED_PROP_UINT3, (const uint8_t *) "nibble", AZ_TYPE_UINT32, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedInst, flags));
    klass->props_self[MASKED_PROP_UINT3].value_type_idx = AZ_TYPE_IDX_UINT32;
    klass->props_self[MASKED_PROP_UINT3].shift = 2;
    klass->props_self[MASKED_PROP_UINT3].mask_width = 3;
    klass->props_self[MASKED_PROP_UINT3].bits = 0;
    /* 8-bit unsigned integer at bits 8-15 (stored in a uint32_t) */
    az_class_define_property_value (klass, MASKED_PROP_UINT8, (const uint8_t *) "byte", AZ_TYPE_UINT32, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedInst, flags));
    klass->props_self[MASKED_PROP_UINT8].value_type_idx = AZ_TYPE_IDX_UINT32;
    klass->props_self[MASKED_PROP_UINT8].shift = 8;
    klass->props_self[MASKED_PROP_UINT8].mask_width = 8;
    klass->props_self[MASKED_PROP_UINT8].bits = 0;
}

static unsigned int
masked_get_type (void)
{
    if (!masked_type) {
        az_register_type (&masked_type, (const unsigned char *) "MaskedTest", AZ_TYPE_STRUCT, sizeof (AZClass), sizeof (MaskedInst),
            AZ_FLAG_FINAL, 0, MASKED_NUM_PROPS, masked_class_init, NULL, NULL);
    }
    return masked_type;
}

static void
test_masked_field()
{
    az_init();
    unsigned int type = masked_get_type();
    AZClass *klass = AZ_CLASS_FROM_TYPE (type);
    MaskedInst inst;
    inst.flags = 0;

    const AZImplementation *prop_impl;
    AZValue64 prop_val;
    AZValue set_val;

    /* Read default (all zero) values */
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "flag", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN));
    TEST_ASSERT_EQUAL_UINT (0, prop_val.value.boolean_v);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "flagInv", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (1, prop_val.value.boolean_v);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "nibble", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT32));
    TEST_ASSERT_EQUAL_UINT (0, prop_val.value.uint32_v);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "byte", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (0, prop_val.value.uint32_v);

    /* Write boolean true */
    set_val.boolean_v = 1;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "flag", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x1, inst.flags);
    /* Read it back */
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "flag", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (1, prop_val.value.boolean_v);

    /* Write inverted boolean true (stored bit becomes 0) */
    set_val.boolean_v = 1;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "flagInv", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x1, inst.flags);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "flagInv", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (1, prop_val.value.boolean_v);
    /* Write inverted boolean false (stored bit becomes 1) */
    set_val.boolean_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "flagInv", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x3, inst.flags);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "flagInv", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (0, prop_val.value.boolean_v);

    /* Write 3-bit unsigned integer */
    set_val.uint32_v = 5;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "nibble", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT32), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x17, inst.flags);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "nibble", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (5, prop_val.value.uint32_v);
    /* Overflowing value is masked */
    set_val.uint32_v = 0xff;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "nibble", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT32), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x1f, inst.flags);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "nibble", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (7, prop_val.value.uint32_v);

    /* Write 8-bit unsigned integer */
    set_val.uint32_v = 0xa5;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "byte", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT32), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0xa51f, inst.flags);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "byte", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (0xa5, prop_val.value.uint32_v);

    /* Writing one field does not disturb others */
    set_val.boolean_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "flag", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0xa51e, inst.flags);
    set_val.uint32_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "byte", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT32), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT32 (0x1e, inst.flags);
}

/*
 * Masked properties stored in fields of the same width as the property type
 */

typedef struct {
    uint8_t u8;
    uint16_t u16;
    uint64_t u64;
} MaskedWidthInst;

#define MASKEDW_NUM_PROPS 6
#define MASKEDW_PROP_U8 0
#define MASKEDW_PROP_U16 1
#define MASKEDW_PROP_U64 2
#define MASKEDW_PROP_U8_IN_U64 3
#define MASKEDW_PROP_BOOL_IN_U64 4
#define MASKEDW_PROP_BOOL_IN_U8 5

static unsigned int maskedw_type = 0;

static void
maskedw_class_init (AZClass *klass)
{
    /* 3-bit uint8 stored in a uint8_t (bits 1-3) */
    az_class_define_property_value (klass, MASKEDW_PROP_U8, (const uint8_t *) "v8", AZ_TYPE_UINT8, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u8));
    klass->props_self[MASKEDW_PROP_U8].value_type_idx = AZ_TYPE_IDX_UINT8;
    klass->props_self[MASKEDW_PROP_U8].shift = 1;
    klass->props_self[MASKEDW_PROP_U8].mask_width = 3;
    klass->props_self[MASKEDW_PROP_U8].bits = 0;
    /* 5-bit uint16 stored in a uint16_t (bits 4-8) */
    az_class_define_property_value (klass, MASKEDW_PROP_U16, (const uint8_t *) "v16", AZ_TYPE_UINT16, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u16));
    klass->props_self[MASKEDW_PROP_U16].value_type_idx = AZ_TYPE_IDX_UINT16;
    klass->props_self[MASKEDW_PROP_U16].shift = 4;
    klass->props_self[MASKEDW_PROP_U16].mask_width = 5;
    klass->props_self[MASKEDW_PROP_U16].bits = 0;
    /* 20-bit uint64 stored in a uint64_t (bits 40-59) */
    az_class_define_property_value (klass, MASKEDW_PROP_U64, (const uint8_t *) "v64", AZ_TYPE_UINT64, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u64));
    klass->props_self[MASKEDW_PROP_U64].value_type_idx = AZ_TYPE_IDX_UINT64;
    klass->props_self[MASKEDW_PROP_U64].shift = 40;
    klass->props_self[MASKEDW_PROP_U64].mask_width = 20;
    klass->props_self[MASKEDW_PROP_U64].bits = 0;
    /* uint8 property stored in a uint64_t (bits 8-15) */
    az_class_define_property_value (klass, MASKEDW_PROP_U8_IN_U64, (const uint8_t *) "v8in64", AZ_TYPE_UINT8, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u64));
    klass->props_self[MASKEDW_PROP_U8_IN_U64].value_type_idx = AZ_TYPE_IDX_UINT64;
    klass->props_self[MASKEDW_PROP_U8_IN_U64].shift = 8;
    klass->props_self[MASKEDW_PROP_U8_IN_U64].mask_width = 8;
    klass->props_self[MASKEDW_PROP_U8_IN_U64].bits = 0;
    /* Boolean property stored in a uint64_t (bit 63) */
    az_class_define_property_value (klass, MASKEDW_PROP_BOOL_IN_U64, (const uint8_t *) "bin64", AZ_TYPE_BOOLEAN, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u64));
    klass->props_self[MASKEDW_PROP_BOOL_IN_U64].value_type_idx = AZ_TYPE_IDX_UINT64;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U64].shift = 63;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U64].mask_width = 1;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U64].bits = 0;
    /* Boolean property stored in a uint8_t (bit 7) */
    az_class_define_property_value (klass, MASKEDW_PROP_BOOL_IN_U8, (const uint8_t *) "bin8", AZ_TYPE_BOOLEAN, 0,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_VALUE, ARIKKEI_OFFSET (MaskedWidthInst, u8));
    klass->props_self[MASKEDW_PROP_BOOL_IN_U8].value_type_idx = AZ_TYPE_IDX_UINT8;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U8].shift = 7;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U8].mask_width = 1;
    klass->props_self[MASKEDW_PROP_BOOL_IN_U8].bits = 0;
}

static unsigned int
maskedw_get_type (void)
{
    if (!maskedw_type) {
        az_register_type (&maskedw_type, (const unsigned char *) "MaskedWidthTest", AZ_TYPE_STRUCT, sizeof (AZClass), sizeof (MaskedWidthInst),
            AZ_FLAG_FINAL, 0, MASKEDW_NUM_PROPS, maskedw_class_init, NULL, NULL);
    }
    return maskedw_type;
}

static void
test_masked_field_widths()
{
    az_init();
    unsigned int type = maskedw_get_type();
    AZClass *klass = AZ_CLASS_FROM_TYPE (type);
    MaskedWidthInst inst;
    inst.u8 = 0;
    inst.u16 = 0;
    inst.u64 = 0;

    const AZImplementation *prop_impl;
    AZValue64 prop_val;
    AZValue set_val;

    /* uint8 storage */
    set_val.uint8_v = 5;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT8 (0x0a, inst.u8);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8));
    TEST_ASSERT_EQUAL_UINT8 (5, prop_val.value.uint8_v);
    /* Overflow is masked, other bits preserved */
    inst.u8 |= 0x81;
    set_val.uint8_v = 0xff;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT8 (0x8f, inst.u8);

    /* uint16 storage */
    set_val.uint16_v = 17;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v16", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT16), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT16 (0x0110, inst.u16);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "v16", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT16));
    TEST_ASSERT_EQUAL_UINT16 (17, prop_val.value.uint16_v);
    inst.u16 |= 0x8001;
    set_val.uint16_v = 0xffff;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v16", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT16), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT16 (0x81f1, inst.u16);

    /* uint64 storage */
    set_val.uint64_v = 0xabcde;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v64", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT64), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0xabcde0000000000ULL, inst.u64);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "v64", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT64));
    TEST_ASSERT_EQUAL_UINT64 (0xabcde, prop_val.value.uint64_v);
    inst.u64 |= 0xff;
    set_val.uint64_v = 0xffffffffffffffffULL;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v64", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT64), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0x0fffff00000000ffULL, inst.u64);

    /* uint8 property stored in a uint64_t */
    inst.u64 = 0;
    set_val.uint8_v = 0x5a;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8in64", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0x5a00ULL, inst.u64);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8in64", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8));
    TEST_ASSERT_EQUAL_UINT8 (0x5a, prop_val.value.uint8_v);
    /* Other bits of the container are preserved */
    inst.u64 |= 0xff;
    set_val.uint8_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "v8in64", AZ_IMPL_FROM_TYPE (AZ_TYPE_UINT8), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0xffULL, inst.u64);

    /* Boolean property stored in a uint64_t */
    inst.u64 = 0;
    set_val.boolean_v = 1;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin64", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0x8000000000000000ULL, inst.u64);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin64", &prop_impl, &prop_val));
    TEST_ASSERT (prop_impl == AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN));
    TEST_ASSERT_EQUAL_UINT (1, prop_val.value.boolean_v);
    set_val.boolean_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin64", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT64 (0, inst.u64);

    /* Boolean property stored in a uint8_t */
    inst.u8 = 0;
    set_val.boolean_v = 1;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin8", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT8 (0x80, inst.u8);
    TEST_ASSERT (az_instance_get_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin8", &prop_impl, &prop_val));
    TEST_ASSERT_EQUAL_UINT (1, prop_val.value.boolean_v);
    /* Other bits of the container are preserved */
    inst.u8 |= 0x01;
    set_val.boolean_v = 0;
    TEST_ASSERT (az_instance_set_property_by_key (&klass->impl, &inst, (const uint8_t *) "bin8", AZ_IMPL_FROM_TYPE (AZ_TYPE_BOOLEAN), &set_val, NULL));
    TEST_ASSERT_EQUAL_UINT8 (0x01, inst.u8);
}

/*
 * Circular type references in class constructors
 *
 * Types referenced from a class constructor are reserved immediately (valid
 * typecode) but constructed lazily if the registration was nested inside another
 * class construction. Consequently circular references through properties and even
 * interface implementation resolve regardless of registration order; only genuine
 * extends/implements cycles are rejected (with a clear error).
 */

typedef struct {
    uint32_t v;
} CircInst;

static unsigned int circ_a_type = 0;
static unsigned int circ_b_type = 0;

static unsigned int circ_b_get_type (void);

static void
circ_a_class_init (AZClass *klass)
{
    /* Property of type B - reserves B (nested registration is deferred) */
    az_class_define_property_value (klass, 0, (const uint8_t *) "b", circ_b_get_type (), 1,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET (CircInst, v));
    TEST_ASSERT (circ_b_type != 0);
}

static void
circ_b_class_init (AZClass *klass)
{
    /* B is constructed lazily after A was fully constructed */
    TEST_ASSERT (circ_a_type != 0);
    az_class_define_property_value (klass, 0, (const uint8_t *) "a", circ_a_type, 1,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET (CircInst, v));
}

static unsigned int
circ_a_get_type (void)
{
    if (!circ_a_type) {
        az_register_type (&circ_a_type, (const unsigned char *) "CircA", AZ_TYPE_STRUCT, sizeof (AZClass), sizeof (CircInst),
            AZ_FLAG_FINAL, 0, 1, circ_a_class_init, NULL, NULL);
    }
    return circ_a_type;
}

static unsigned int
circ_b_get_type (void)
{
    if (!circ_b_type) {
        az_register_type (&circ_b_type, (const unsigned char *) "CircB", AZ_TYPE_STRUCT, sizeof (AZClass), sizeof (CircInst),
            AZ_FLAG_FINAL, 0, 1, circ_b_class_init, NULL, NULL);
    }
    return circ_b_type;
}

/* Circular interface implementation (works with deferred construction) */

typedef struct {
    AZClass klass;
    AZImplementation iface_impl;
} CircImplClass;

static unsigned int circ_iface_type = 0;
static unsigned int circ_impl_type = 0;

static unsigned int circ_impl_get_type (void);

static void
circ_iface_class_init (AZClass *klass)
{
    /* Reserves CircImpl (nested registration is deferred, not constructed here) */
    az_class_define_property_value (klass, 0, (const uint8_t *) "impl", circ_impl_get_type (), 1,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_NONE, 0);
}

static void
circ_impl_class_init (AZClass *klass)
{
    /* CircImpl is constructed lazily, after the interface was fully constructed */
    az_class_declare_interface (klass, 0, circ_iface_type, ARIKKEI_OFFSET (CircImplClass, iface_impl), 0);
}

static unsigned int
circ_iface_get_type (void)
{
    if (!circ_iface_type) {
        az_register_interface_type (&circ_iface_type, (const unsigned char *) "CircIface", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 0, 1,
            circ_iface_class_init, NULL, NULL, NULL);
    }
    return circ_iface_type;
}

static unsigned int
circ_impl_get_type (void)
{
    if (!circ_impl_type) {
        az_register_type (&circ_impl_type, (const unsigned char *) "CircImpl", AZ_TYPE_STRUCT, sizeof (CircImplClass), sizeof (CircInst),
            AZ_FLAG_FINAL, 1, 0, circ_impl_class_init, NULL, NULL);
    }
    return circ_impl_type;
}

/*
 * The esoteric case: A implements B and C, B extends C and D, C has a property of A.
 * Previously the result depended on the registration entry point.
 */

typedef struct {
    AZClass klass;
    AZImplementation b_impl;
    AZImplementation c_impl;
} CDAClass;

typedef struct {
    uint32_t b_data;
    uint32_t c_data;
} CDAInst;

static unsigned int cd_a_type = 0, cd_b_type = 0, cd_c_type = 0, cd_d_type = 0;

static unsigned int cd_a_get_type (void);

static void
cd_c_class_init (AZClass *klass)
{
    /* Property referring to A - reserves A, does not construct it */
    az_class_define_property_value (klass, 0, (const uint8_t *) "a", cd_a_get_type (), 1,
        AZ_FIELD_INSTANCE, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET (CDAInst, b_data));
}

static unsigned int
cd_c_get_type (void)
{
    if (!cd_c_type) {
        az_register_interface_type (&cd_c_type, (const unsigned char *) "CDC", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 0, 1,
            cd_c_class_init, NULL, NULL, NULL);
    }
    return cd_c_type;
}

static unsigned int
cd_d_get_type (void)
{
    if (!cd_d_type) {
        az_register_interface_type (&cd_d_type, (const unsigned char *) "CDD", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 0, 0,
            NULL, NULL, NULL, NULL);
    }
    return cd_d_type;
}

static void
cd_b_class_init (AZClass *klass)
{
    /* B extends C and D; declaring C constructs it (reserving A, not constructing it) */
    az_class_declare_interface (klass, 0, cd_c_get_type (), 0, 0);
    az_class_declare_interface (klass, 1, cd_d_get_type (), 0, 0);
}

static unsigned int
cd_b_get_type (void)
{
    if (!cd_b_type) {
        az_register_interface_type (&cd_b_type, (const unsigned char *) "CDB", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 2, 0,
            cd_b_class_init, NULL, NULL, NULL);
    }
    return cd_b_type;
}

static void
cd_a_class_init (AZClass *klass)
{
    /* A is constructed on first access; B and C are fully constructed by then */
    az_class_declare_interface (klass, 0, cd_b_get_type (), ARIKKEI_OFFSET (CDAClass, b_impl), ARIKKEI_OFFSET (CDAInst, b_data));
    az_class_declare_interface (klass, 1, cd_c_get_type (), ARIKKEI_OFFSET (CDAClass, c_impl), ARIKKEI_OFFSET (CDAInst, c_data));
}

static unsigned int
cd_a_get_type (void)
{
    if (!cd_a_type) {
        az_register_type (&cd_a_type, (const unsigned char *) "CDA", AZ_TYPE_STRUCT, sizeof (CDAClass), sizeof (CDAInst),
            AZ_FLAG_FINAL, 2, 0, cd_a_class_init, NULL, NULL);
    }
    return cd_a_type;
}

/* Genuine extends cycle: X extends Y, Y extends X - the inner edge is rejected */

static unsigned int cyc_x_type = 0, cyc_y_type = 0;

static unsigned int cyc_y_get_type (void);

static void
cyc_x_class_init (AZClass *klass)
{
    az_class_declare_interface (klass, 0, cyc_y_get_type (), 0, 0);
}

static void
cyc_y_class_init (AZClass *klass)
{
    /* X is mid-construction: rejected with a clear error, Y completes without it */
    az_class_declare_interface (klass, 0, cyc_x_type, 0, 0);
}

static unsigned int
cyc_x_get_type (void)
{
    if (!cyc_x_type) {
        az_register_interface_type (&cyc_x_type, (const unsigned char *) "CycX", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 1, 0,
            cyc_x_class_init, NULL, NULL, NULL);
    }
    return cyc_x_type;
}

static unsigned int
cyc_y_get_type (void)
{
    if (!cyc_y_type) {
        az_register_interface_type (&cyc_y_type, (const unsigned char *) "CycY", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 1, 0,
            cyc_y_class_init, NULL, NULL, NULL);
    }
    return cyc_y_type;
}

static void
test_circular_reference()
{
    az_init();
    /* Circular property references by typecode */
    unsigned int ta = circ_a_get_type ();
    unsigned int tb = circ_b_get_type ();
    AZClass *ka = AZ_CLASS_FROM_TYPE (ta);
    TEST_ASSERT (ka != NULL);
    TEST_ASSERT_EQUAL_UINT (tb, ka->props_self[0].type);
    /* B is constructed on first access */
    AZClass *kb = AZ_CLASS_FROM_TYPE (tb);
    TEST_ASSERT (kb != NULL);
    TEST_ASSERT_EQUAL_UINT (ta, kb->props_self[0].type);
    /* Circular interface implementation now works (deferred construction) */
    unsigned int ti = circ_iface_get_type ();
    unsigned int tm = circ_impl_get_type ();
    TEST_ASSERT (AZ_CLASS_FROM_TYPE (ti) != NULL);
    TEST_ASSERT (AZ_CLASS_FROM_TYPE (tm) != NULL);
    TEST_ASSERT (az_type_implements (tm, ti));
    /* The esoteric case: entry from B, property/interface mix */
    unsigned int tcb = cd_b_get_type ();
    unsigned int tcc = cd_c_get_type ();
    unsigned int tcd = cd_d_get_type ();
    unsigned int tca = cd_a_get_type ();
    TEST_ASSERT (az_type_implements (tcb, tcc));
    TEST_ASSERT (az_type_implements (tcb, tcd));
    TEST_ASSERT (az_type_implements (tca, tcb));
    TEST_ASSERT (az_type_implements (tca, tcc));
    /* Transitive: A implements D through B */
    TEST_ASSERT (az_type_implements (tca, tcd));
    /* Genuine interface cycle: the closing edge is rejected, everything completes */
    unsigned int tx = cyc_x_get_type ();
    unsigned int ty = cyc_y_get_type ();
    TEST_ASSERT (AZ_CLASS_FROM_TYPE (tx) != NULL);
    TEST_ASSERT (AZ_CLASS_FROM_TYPE (ty) != NULL);
    TEST_ASSERT (az_type_implements (tx, ty));
    TEST_ASSERT (!az_type_implements (ty, tx));
}

/*
 * Race string collation from multiple threads.
 *
 * All threads interning the same content have to get the same collated AZString
 * instance, and the collation table has to survive concurrent
 * new/lookup/unref/concat (including disposal churn and resurrection).
 */

#define SMT_NUM_THREADS 8
#define SMT_NUM_ITERS 2000
#define SMT_NUM_STRINGS 8

static const char *smt_strings[SMT_NUM_STRINGS] = {
    "alpha", "beta", "gamma", "delta", "alphabeta", "deltata", "epsilon", "zeta"
};

static AZString *smt_results[SMT_NUM_THREADS][SMT_NUM_STRINGS];
static AZString *smt_concat[SMT_NUM_THREADS];

static int
string_mt_thread (void *arg)
{
    unsigned int seed = (unsigned int) (uintptr_t) arg;
    AZString *alpha = az_string_new ((const unsigned char *) "alpha");
    AZString *beta = az_string_new ((const unsigned char *) "beta");
    for (unsigned int i = 0; i < SMT_NUM_ITERS; i++) {
        unsigned int idx = (seed + i) % SMT_NUM_STRINGS;
        /* Intern, verify collation against lookup, and churn the previous ref */
        AZString *s = az_string_new ((const unsigned char *) smt_strings[idx]);
        AZString *l = az_string_lookup ((const unsigned char *) smt_strings[idx]);
        if (l) {
            if (l != s) return (int) (idx + 1);
            az_string_unref (l);
        }
        if (smt_results[seed][idx]) az_string_unref (smt_results[seed][idx]);
        smt_results[seed][idx] = s;
        /* Concat exercises the create-collate-discard path */
        AZString *c = az_string_concat (alpha, beta);
        if (smt_concat[seed]) az_string_unref (smt_concat[seed]);
        smt_concat[seed] = c;
    }
    az_string_unref (alpha);
    az_string_unref (beta);
    return 0;
}

static void
test_strings_mt()
{
    az_init();
    thrd_t threads[SMT_NUM_THREADS];
    for (int i = 0; i < SMT_NUM_THREADS; i++) {
        TEST_ASSERT(thrd_create(&threads[i], string_mt_thread, (void *) (uintptr_t) i) == thrd_success);
    }
    int results[SMT_NUM_THREADS];
    for (int i = 0; i < SMT_NUM_THREADS; i++) {
        TEST_ASSERT(thrd_join(threads[i], &results[i]) == thrd_success);
        TEST_ASSERT_EQUAL (0, results[i]);
    }
    /* All threads agree on every collated instance */
    AZString *alphabet = az_string_lookup ((const unsigned char *) "alphabeta");
    TEST_ASSERT (alphabet != NULL);
    for (unsigned int idx = 0; idx < SMT_NUM_STRINGS; idx++) {
        AZString *ref = az_string_lookup ((const unsigned char *) smt_strings[idx]);
        TEST_ASSERT (ref != NULL);
        for (int i = 0; i < SMT_NUM_THREADS; i++) {
            TEST_ASSERT (smt_results[i][idx] == NULL || smt_results[i][idx] == ref);
        }
        az_string_unref (ref);
    }
    for (int i = 0; i < SMT_NUM_THREADS; i++) {
        TEST_ASSERT (smt_concat[i] == alphabet);
    }
    az_string_unref (alphabet);
    /* Cleanup */
    for (int i = 0; i < SMT_NUM_THREADS; i++) {
        for (unsigned int idx = 0; idx < SMT_NUM_STRINGS; idx++) {
            if (smt_results[i][idx]) az_string_unref (smt_results[i][idx]);
        }
        if (smt_concat[i]) az_string_unref (smt_concat[i]);
    }
}

/*
 * Boxed interface conversion
 *
 * A boxed interface contains the whole instantiable object (for lifetime) and the
 * presented interface view onto it. Converting a boxed interface to a supertype
 * interface has to preserve the presented view, not re-resolve the target from
 * the containing object: the keyset view of a map is a collection, but the map
 * itself is also a collection - resolving from the map would yield the wrong view.
 */

typedef struct {
    AZClass klass;
    AZImplementation keys_impl;
    AZImplementation coll_impl;
} BIMapClass;

typedef struct {
    uint32_t keys_data;
    uint32_t coll_data;
} BIMapInst;

static unsigned int bi_coll_type = 0;
static unsigned int bi_keys_type = 0;
static unsigned int bi_map_type = 0;

static unsigned int
bi_coll_get_type (void)
{
    if (!bi_coll_type) {
        az_register_interface_type (&bi_coll_type, (const unsigned char *) "BIColl", AZ_TYPE_INTERFACE,
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 0, 0,
            NULL, NULL, NULL, NULL);
    }
    return bi_coll_type;
}

static unsigned int
bi_keys_get_type (void)
{
    if (!bi_keys_type) {
        az_register_interface_type (&bi_keys_type, (const unsigned char *) "BIKeys", bi_coll_get_type (),
            sizeof (AZInterfaceClass), sizeof (AZImplementation), sizeof (uint32_t), AZ_FLAG_ABSTRACT, 0, 0,
            NULL, NULL, NULL, NULL);
    }
    return bi_keys_type;
}

static void
bi_map_class_init (AZClass *klass)
{
    /* The map presents both the keyset view and a direct collection view */
    az_class_declare_interface (klass, 0, bi_keys_get_type (), ARIKKEI_OFFSET (BIMapClass, keys_impl), ARIKKEI_OFFSET (BIMapInst, keys_data));
    az_class_declare_interface (klass, 1, bi_coll_get_type (), ARIKKEI_OFFSET (BIMapClass, coll_impl), ARIKKEI_OFFSET (BIMapInst, coll_data));
}

static unsigned int
bi_map_get_type (void)
{
    if (!bi_map_type) {
        az_register_type (&bi_map_type, (const unsigned char *) "BIMap", AZ_TYPE_BLOCK, sizeof (BIMapClass), sizeof (BIMapInst),
            AZ_FLAG_FINAL, 2, 0, bi_map_class_init, NULL, NULL);
    }
    return bi_map_type;
}

static void
test_boxed_interface_conversion()
{
    az_init();
    unsigned int keys_type = bi_keys_get_type ();
    unsigned int coll_type = bi_coll_get_type ();
    unsigned int map_type = bi_map_get_type ();
    BIMapInst minst = { 111, 222 };
    /* Box the map presenting the Keys view */
    AZBoxedInterface *box = az_boxed_interface_new_from_impl_instance (AZ_IMPL_FROM_TYPE(map_type), &minst, keys_type);
    TEST_ASSERT (box->inst == &minst.keys_data);
    /* Keys -> Coll: the keyset view has to be preserved (not re-resolved from the map) */
    const AZImplementation *dst_impl = NULL;
    AZValue dst_val;
    AZValue src;
    src.reference = &box->reference;
    TEST_ASSERT_EQUAL_INT (AZ_CONVERSION_EXACT,
        az_value_convert_autobox (&dst_impl, &dst_val, sizeof (dst_val), AZ_BOXED_INTERFACE_IMPL, &src, coll_type, AZ_CONVERT_AUTO));
    AZBoxedInterface *out = (AZBoxedInterface *) dst_val.reference;
    /* Keys is a subtype of Coll: the original box is kept */
    TEST_ASSERT (out == box);
    TEST_ASSERT (out->inst == &minst.keys_data);
    az_boxed_interface_unref (out);
    az_boxed_interface_unref (box);
}

/*
 * AZValueArray: a dynamic array of arbitrary values (subtypes of the pre-set type).
 * Values <= 8 bytes are stored inline in the entry array, bigger values in a
 * separate data pool in units of 16-byte (AZValue) blocks.
 */

/* Struct types of different sizes for the test */
typedef struct { uint32_t marker; uint8_t data[252]; } VAStruct;

static unsigned int va_types[5] = {0, 0, 0, 0, 0};

/* sizes: 4, 8 (inline), 16, 24, 40 (data pool: 1, 2, 3 blocks) */
static const unsigned int va_sizes[5] = {4, 8, 16, 24, 40};

static unsigned int
va_type_get (unsigned int i)
{
    if (!va_types[i]) {
        uint8_t name[32];
        snprintf ((char *) name, 32, "VAStruct%u", va_sizes[i]);
        az_register_type (&va_types[i], name, AZ_TYPE_STRUCT, sizeof (AZClass), va_sizes[i], AZ_FLAG_FINAL, 0, 0, NULL, NULL, NULL);
    }
    return va_types[i];
}

/* Fill a struct with an index-dependent pattern */
static void
va_fill (AZValue *v, unsigned int size, unsigned int seed)
{
    uint8_t *p = (uint8_t *) v;
    for (unsigned int i = 0; i < size; i++) p[i] = (uint8_t) (seed + i);
}

static unsigned int
va_check (AZValue *v, unsigned int size, unsigned int seed)
{
    uint8_t *p = (uint8_t *) v;
    for (unsigned int i = 0; i < size; i++) {
        if (p[i] != (uint8_t) (seed + i)) return 0;
    }
    return 1;
}

/* Read an element and verify its content */
static void
va_verify_element (AZValueArray *va, unsigned int idx, unsigned int type_idx, unsigned int seed)
{
    void *list_inst;
    const AZImplementation *list_impl = az_instance_get_interface (AZ_IMPL_FROM_TYPE(AZ_TYPE_VALUE_ARRAY), va, AZ_TYPE_LIST, &list_inst);
    const AZImplementation *impl;
    AZValue64 val;
    impl = az_list_get_element ((const AZListImplementation *) list_impl, list_inst, idx, &val.value, sizeof (AZValue64));
    TEST_ASSERT (impl == AZ_IMPL_FROM_TYPE(va_type_get (type_idx)));
    TEST_ASSERT (va_check (&val.value, va_sizes[type_idx], seed));
}

static void
test_value_array()
{
    az_init();
    az_value_array_get_type ();
    AZValueArray *va = (AZValueArray *) az_instance_new (AZ_TYPE_VALUE_ARRAY);
    az_value_array_set_length (va, 8);

    /* Fill with inline values (4 and 8 bytes) */
    for (unsigned int i = 0; i < 8; i++) {
        AZValue64 v;
        va_fill (&v.value, va_sizes[i % 2], i);
        az_value_array_set_element_from_val (va, i, AZ_IMPL_FROM_TYPE(va_type_get (i % 2)), &v.value);
    }
    for (unsigned int i = 0; i < 8; i++) {
        va_verify_element (va, i, i % 2, i);
    }

    /* Replace with big values (16/24/40 bytes -> data pool) */
    for (unsigned int i = 0; i < 8; i++) {
        AZValue64 v;
        unsigned int t = 2 + (i % 3);
        va_fill (&v.value, va_sizes[t], 100 + i);
        az_value_array_set_element_from_val (va, i, AZ_IMPL_FROM_TYPE(va_type_get (t)), &v.value);
    }
    /* Every big value must read back its own content */
    for (unsigned int i = 0; i < 8; i++) {
        va_verify_element (va, i, 2 + (i % 3), 100 + i);
    }

    /* Mix: replace some big with small and vice versa */
    AZValue64 s0; va_fill (&s0.value, 4, 50);
    az_value_array_set_element_from_val (va, 1, AZ_IMPL_FROM_TYPE(va_type_get (0)), &s0.value);   /* big -> small */
    AZValue64 s1; va_fill (&s1.value, 40, 60);
    az_value_array_set_element_from_val (va, 4, AZ_IMPL_FROM_TYPE(va_type_get (4)), &s1.value);   /* big -> bigger */
    AZValue64 s2; va_fill (&s2.value, 8, 70);
    az_value_array_set_element_from_val (va, 2, AZ_IMPL_FROM_TYPE(va_type_get (1)), &s2.value);   /* big -> small */
    /* Final state: idx0=S16(100), idx1=S4(50), idx2=S8(70), idx3=S16(103), idx4=S40(60), idx5=S40(105), idx6=S16(106), idx7=S24(107) */
    va_verify_element (va, 0, 2, 100);
    va_verify_element (va, 1, 0, 50);
    va_verify_element (va, 2, 1, 70);
    va_verify_element (va, 3, 2, 103);
    va_verify_element (va, 4, 4, 60);
    va_verify_element (va, 5, 4, 105);
    va_verify_element (va, 6, 2, 106);
    va_verify_element (va, 7, 3, 107);

    /* Remove elements from the middle/end */
    az_value_array_set_length (va, 5);
    TEST_ASSERT_EQUAL_UINT (5, va->list.collection.size);
    va_verify_element (va, 0, 2, 100);
    va_verify_element (va, 1, 0, 50);
    va_verify_element (va, 2, 1, 70);
    va_verify_element (va, 3, 2, 103);
    va_verify_element (va, 4, 4, 60);

    /* Grow again and refill */
    az_value_array_set_length (va, 10);
    TEST_ASSERT_EQUAL_UINT (10, va->list.collection.size);
    for (unsigned int i = 5; i < 10; i++) {
        AZValue64 v;
        unsigned int t = i % 5;
        va_fill (&v.value, va_sizes[t], 200 + i);
        az_value_array_set_element_from_val (va, i, AZ_IMPL_FROM_TYPE(va_type_get (t)), &v.value);
    }
    for (unsigned int i = 5; i < 10; i++) {
        va_verify_element (va, i, i % 5, 200 + i);
    }
    /* The survivors are intact */
    va_verify_element (va, 0, 2, 100);
    va_verify_element (va, 4, 4, 60);

    az_instance_delete (AZ_TYPE_VALUE_ARRAY, va);
}
