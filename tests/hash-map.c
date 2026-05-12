#define __HASH_MAP_TEST_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/value.h>
#include <az/interface.h>
#include <az/instance.h>
#include <az/collections/hash-map.h>

#include "unity/unity.h"

#define NUM_ENTRIES 10000

static uint32_t
int32_hash(AZHashMapImplementation *impl, const AZValue *key)
{
    uint32_t x = (uint32_t) key->int32_v;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static unsigned int
int32_equal(AZHashMapImplementation *impl, const AZValue *lhs, const AZValue *rhs)
{
    return lhs->int32_v == rhs->int32_v;
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
remove_odd_val(const AZValue *key, const AZValue *val, void *data)
{
    return val->int32_v & 1;
}

void
test_hash_map(void)
{
    az_init();

    AZHashMapImplementation impl;
    hash_map_impl_setup(&impl, 31);

    AZHashMap hmap;
    az_instance_init((const AZImplementation *) &impl, &hmap);

    int32_t vals[NUM_ENTRIES];

    srand(42);
    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        vals[i] = (int32_t) rand();
    }

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key, val;
        key.int32_v = (int32_t) i;
        val.int32_v = vals[i];
        az_hash_map_insert(&impl, &hmap, &key, &val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key;
        key.int32_v = (int32_t) i;
        TEST_ASSERT(az_hash_map_exists(&impl, &hmap, &key));
        const AZValue *found = az_hash_map_lookup(&impl, &hmap, &key);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_INT32(vals[i], found->int32_v);
    }

    for (unsigned int i = 0; i < NUM_ENTRIES; i += 2) {
        AZValue key;
        key.int32_v = (int32_t) i;
        TEST_ASSERT(az_hash_map_remove(&impl, &hmap, &key));
        TEST_ASSERT(!az_hash_map_exists(&impl, &hmap, &key));
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES / 2, hmap.n_entries);

    for (unsigned int i = 1; i < NUM_ENTRIES; i += 2) {
        AZValue key;
        key.int32_v = (int32_t) i;
        TEST_ASSERT(az_hash_map_exists(&impl, &hmap, &key));
    }

    for (unsigned int i = 1; i < NUM_ENTRIES; i += 2) {
        AZValue key;
        key.int32_v = (int32_t) i;
        TEST_ASSERT(az_hash_map_remove(&impl, &hmap, &key));
        TEST_ASSERT(!az_hash_map_exists(&impl, &hmap, &key));
    }

    TEST_ASSERT_EQUAL_UINT(0, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key, val;
        key.int32_v = (int32_t) i;
        val.int32_v = vals[i];
        az_hash_map_insert(&impl, &hmap, &key, &val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    unsigned int n_removed = az_hash_map_remove_all(&impl, &hmap, remove_odd_val, NULL);
    TEST_ASSERT_TRUE(n_removed > 0);
    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES - n_removed, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key;
        key.int32_v = (int32_t) i;
        if (vals[i] & 1) {
            TEST_ASSERT(!az_hash_map_exists(&impl, &hmap, &key));
        } else {
            TEST_ASSERT(az_hash_map_exists(&impl, &hmap, &key));
        }
    }

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key, val;
        key.int32_v = (int32_t) i;
        val.int32_v = vals[i];
        az_hash_map_insert(&impl, &hmap, &key, &val);
    }

    az_hash_map_clear(&impl, &hmap);
    TEST_ASSERT_EQUAL_UINT(0, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key;
        key.int32_v = (int32_t) i;
        TEST_ASSERT(!az_hash_map_exists(&impl, &hmap, &key));
    }

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key, val;
        key.int32_v = (int32_t) i;
        val.int32_v = vals[i];
        az_hash_map_insert(&impl, &hmap, &key, &val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key, val;
        key.int32_v = (int32_t) i;
        val.int32_v = vals[i] + 1000;
        az_hash_map_insert(&impl, &hmap, &key, &val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.n_entries);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        AZValue key;
        key.int32_v = (int32_t) i;
        const AZValue *found = az_hash_map_lookup(&impl, &hmap, &key);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_INT32(vals[i] + 1000, found->int32_v);
    }

    az_instance_finalize((const AZImplementation *) &impl, &hmap);
}
