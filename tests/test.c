#define __TEST_C__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <arikkei/arikkei-threads.h>

#include <az/az.h>
#include <az/base.h>
#include <az/boxed-value.h>
#include <az/extend.h>
#include <az/function.h>
#include <az/function-native.h>
#include <az/function-value.h>
#include <az/packed-value.h>
#include <az/reference-of.h>
#include <az/string.h>
#include <az/types.h>
#include <az/value.h>
#include <az/classes/active-object.h>
#include <az/collections/array-list.h>
#include <az/collections/array.h>
#include <az/classes/array-object.h>
#include <az/collections/hash-map.h>
#include <az/collections/hash-set.h>

#include "unity/unity.h"

static void test_types();
static void test_types_mt();
static void test_to_string();
static void test_boxed_value();
static void test_array_list();
static void test_array();
static void test_call_native();

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

static void
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
