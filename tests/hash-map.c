#define __HASH_MAP_TEST_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/value.h>
#include <az/interface.h>
#include <az/instance.h>
#include <az/collections/collection.h>
#include <az/collections/hash-map.h>

#include "unity/unity.h"

#define NUM_ENTRIES 10000

static uint32_t
int32_hash(const AZHashMapImplementation *impl, const void *key)
{
    uint32_t x = *((const uint32_t *) key);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static unsigned int
int32_equal(const AZHashMapImplementation *impl, const void *lhs, const void *rhs)
{
    return *((const uint32_t *) lhs) == *((const uint32_t *) rhs);
}

static unsigned int
align16(unsigned int v)
{
    return (v + 15) & ~(unsigned int) 15;
}

static void
hash_map_impl_setup(AZHashMapImplementation *impl, unsigned int root_size)
{
    unsigned int key_size = 4;
    unsigned int val_size = 4;
    unsigned int key_offset = 8;
    unsigned int val_offset = align16(key_offset + key_size);
    unsigned int entry_size = align16(val_offset + val_size);

    az_implementation_init_by_type((AZImplementation *) impl, AZ_TYPE_HASH_MAP);
    impl->key_impl = &AZInt32Klass.impl;
    impl->val_impl = &AZInt32Klass.impl;
    impl->root_size = root_size;
    impl->entry_size = entry_size;
    impl->key_offset = key_offset;
    impl->key_size = key_size;
    impl->val_offset = val_offset;
    impl->val_size = val_size;
    impl->hash = int32_hash;
    impl->equal = int32_equal;
}

static unsigned int
remove_odd_val(const void *key, const void *val, void *data)
{
    return *((const int32_t *) val) & 1;
}

static void
test_insert_remove(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) &impl, &hmap);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hmap.n_entries);

    for (unsigned int i = 0; i < n_entries; i++) {
        TEST_ASSERT(az_hash_map_exists(impl, &hmap, &keys[i]));
        const int32_t *found = (const int32_t *) az_hash_map_lookup(impl, &hmap, &keys[i]);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_INT32(vals[i], *found);
    }

    for (unsigned int i = 0; i < n_entries; i += 2) {
        TEST_ASSERT(az_hash_map_remove(impl, &hmap, &keys[i]));
        TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
    }

    TEST_ASSERT_EQUAL_UINT(n_entries / 2, hmap.n_entries);

    for (unsigned int i = 1; i < n_entries; i += 2) {
        TEST_ASSERT(az_hash_map_exists(impl, &hmap, &keys[i]));
        TEST_ASSERT(az_hash_map_remove(impl, &hmap, &keys[i]));
        TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
    }

    TEST_ASSERT_EQUAL_UINT(0, hmap.n_entries);

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

static void
test_insert_remove_all(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) &impl, &hmap);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hmap.n_entries);

    unsigned int n_removed = az_hash_map_remove_all(impl, &hmap, remove_odd_val, NULL);
    TEST_ASSERT_TRUE(n_removed > 0);
    TEST_ASSERT_EQUAL_UINT(n_entries - n_removed, hmap.n_entries);

    for (unsigned int i = 0; i < n_entries; i++) {
        if (vals[i] & 1) {
            TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
        } else {
            TEST_ASSERT(az_hash_map_exists(impl, &hmap, &keys[i]));
        }
    }

    az_hash_map_clear(impl, &hmap);
    TEST_ASSERT_EQUAL_UINT(0, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
    }

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

static void
test_overwrite_remove_val(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) &impl, &hmap);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        uint32_t new_val = vals[i] + 1000;
        az_hash_map_insert(impl, &hmap, &keys[i], &new_val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        const int32_t *found = (const int32_t *) az_hash_map_lookup(impl, &hmap, &keys[i]);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_INT32(vals[i] + 1000, *found);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        uint32_t new_val = vals[i] + 1000;
        TEST_ASSERT(az_hash_map_exists_val(impl, &hmap, &new_val));
    }

    int32_t absent_val = -999999;
    TEST_ASSERT(!az_hash_map_exists_val(impl, &hmap, &absent_val));

    az_hash_map_clear(impl, &hmap);
    uint32_t new_val_0 = vals[0] + 1000;
    TEST_ASSERT(!az_hash_map_exists_val(impl, &hmap, &new_val_0));

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        TEST_ASSERT(az_hash_map_exists_val(impl, &hmap, &vals[i]));
    }

    az_hash_map_remove(impl, &hmap, &keys[0]);
    TEST_ASSERT(!az_hash_map_exists_val(impl, &hmap, &vals[0]));
    for (unsigned int i = 1; i < NUM_ENTRIES; i++) {
        TEST_ASSERT(az_hash_map_exists_val(impl, &hmap, &vals[i]));
    }

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

void
test_hash_map(void)
{
    az_init();

    AZHashMapImplementation impl;
    hash_map_impl_setup(&impl, 31);

    int32_t keys[NUM_ENTRIES];
    int32_t vals[NUM_ENTRIES];

    srand(42);
    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        keys[i] = (int32_t) i;
        vals[i] = (int32_t) rand();
    }

    test_insert_remove(&impl, keys, vals, NUM_ENTRIES);

    test_insert_remove_all(&impl, keys, vals, NUM_ENTRIES);

    test_overwrite_remove_val(&impl, keys, vals, NUM_ENTRIES);
}
