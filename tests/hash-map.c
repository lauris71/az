#define __HASH_MAP_TEST_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/value.h>
#include <az/interface.h>
#include <az/instance.h>
#include <az/packed-value.h>
#include <az/collections/collection.h>
#include <az/collections/map.h>
#include <az/collections/set.h>
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
    az_instance_init((const AZImplementation *) impl, &hmap);

    TEST_ASSERT_EQUAL_UINT(az_collection_get_element_type((AZCollectionImplementation *) impl, &hmap.map.collection), AZ_TYPE_INT32);
    TEST_ASSERT_EQUAL_UINT(az_collection_get_element_type((AZCollectionImplementation *) impl, &hmap.map.collection), AZ_TYPE_INT32);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hmap.map.collection.size);

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

    TEST_ASSERT_EQUAL_UINT(n_entries / 2, hmap.map.collection.size);

    for (unsigned int i = 1; i < n_entries; i += 2) {
        TEST_ASSERT(az_hash_map_exists(impl, &hmap, &keys[i]));
        TEST_ASSERT(az_hash_map_remove(impl, &hmap, &keys[i]));
        TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
    }

    TEST_ASSERT_EQUAL_UINT(0, hmap.map.collection.size);

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

static void
test_insert_remove_all(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) impl, &hmap);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hmap.map.collection.size);

    unsigned int n_removed = az_hash_map_remove_all(impl, &hmap, remove_odd_val, NULL);
    TEST_ASSERT_TRUE(n_removed > 0);
    TEST_ASSERT_EQUAL_UINT(n_entries - n_removed, hmap.map.collection.size);

    for (unsigned int i = 0; i < n_entries; i++) {
        if (vals[i] & 1) {
            TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
        } else {
            TEST_ASSERT(az_hash_map_exists(impl, &hmap, &keys[i]));
        }
    }

    az_hash_map_clear(impl, &hmap);
    TEST_ASSERT_EQUAL_UINT(0, hmap.map.collection.size);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        TEST_ASSERT(!az_hash_map_exists(impl, &hmap, &keys[i]));
    }

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

static void
test_overwrite_remove_val(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) impl, &hmap);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.map.collection.size);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        uint32_t new_val = vals[i] + 1000;
        az_hash_map_insert(impl, &hmap, &keys[i], &new_val);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.map.collection.size);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        const int32_t *found = (const int32_t *) az_hash_map_lookup(impl, &hmap, &keys[i]);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_INT32(vals[i] + 1000, *found);
    }

    TEST_ASSERT_EQUAL_UINT(NUM_ENTRIES, hmap.map.collection.size);

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

static void
test_iterator(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) impl, &hmap);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    const AZCollectionImplementation *coll = &impl->map_impl.collection_impl;
    const AZMapImplementation *map = &impl->map_impl;

    unsigned int count = 0;
    AZValue iter;
    const AZImplementation *iter_impl = az_collection_get_iterator(coll, &hmap.map.collection, &iter);
    while (iter_impl) {
        AZValue val;
        const AZImplementation *elem_impl = az_collection_get_element(coll, &hmap.map.collection, &iter, &val, sizeof(AZValue));
        TEST_ASSERT_NOT_NULL(elem_impl);

        AZValue kval;
        const AZImplementation *key_ret = az_map_get_key(map, (AZMap *) &hmap, &iter, &kval, sizeof(AZValue));
        TEST_ASSERT_NOT_NULL(key_ret);

        const int32_t *lookup_val = (const int32_t *) az_hash_map_lookup(impl, &hmap, &kval.int32_v);
        TEST_ASSERT_NOT_NULL(lookup_val);
        TEST_ASSERT_EQUAL_INT32(val.int32_v, *lookup_val);

        count++;
        iter_impl = az_collection_iterator_next(coll, &hmap.map.collection, &iter);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, count);

    az_hash_map_clear(impl, &hmap);
    TEST_ASSERT_EQUAL_UINT(0, hmap.map.collection.size);

    iter_impl = az_collection_get_iterator(coll, &hmap.map.collection, &iter);
    TEST_ASSERT_NULL(iter_impl);

    az_instance_finalize((const AZImplementation *) impl, &hmap);
}

static void
test_keyset(const AZHashMapImplementation *impl, int32_t *keys, int32_t *vals, unsigned int n_entries)
{
    AZHashMap hmap;
    az_instance_init((const AZImplementation *) impl, &hmap);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_map_insert(impl, &hmap, &keys[i], &vals[i]);
    }

    const AZMapImplementation *map = &impl->map_impl;

    AZSet *keyset_inst;
    const AZSetImplementation *keyset = az_map_get_keys(map, (AZMap *) &hmap, &keyset_inst);
    TEST_ASSERT_NOT_NULL(keyset);

    const AZCollectionImplementation *keyset_coll = &keyset->collection_impl;
    TEST_ASSERT_EQUAL_UINT(n_entries, az_collection_get_size(keyset_coll, &keyset_inst->collection));
    TEST_ASSERT_EQUAL_UINT(AZ_TYPE_INT32, az_collection_get_element_type(keyset_coll, &keyset_inst->collection));

    unsigned int count = 0;
    AZValue iter;
    const AZImplementation *iter_impl = az_collection_get_iterator(keyset_coll, &keyset_inst->collection, &iter);
    while (iter_impl) {
        AZValue kval;
        const AZImplementation *elem_impl = az_collection_get_element(keyset_coll, &keyset_inst->collection, &iter, &kval, sizeof(AZValue));
        TEST_ASSERT_NOT_NULL(elem_impl);

        TEST_ASSERT(az_hash_map_exists(impl, &hmap, &kval.int32_v));

        count++;
        iter_impl = az_collection_iterator_next(keyset_coll, &keyset_inst->collection, &iter);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, count);

    for (unsigned int i = 0; i < n_entries; i++) {
        TEST_ASSERT(az_collection_contains(keyset_coll, &keyset_inst->collection, &AZInt32Klass.impl, &keys[i]));
    }

    int32_t absent_key = -999999;
    TEST_ASSERT(!az_collection_contains(keyset_coll, &keyset_inst->collection, &AZInt32Klass.impl, &absent_key));

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

    test_iterator(&impl, keys, vals, NUM_ENTRIES);

    test_keyset(&impl, keys, vals, NUM_ENTRIES);

    test_insert_remove_all(&impl, keys, vals, NUM_ENTRIES);

    test_overwrite_remove_val(&impl, keys, vals, NUM_ENTRIES);
}
