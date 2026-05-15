#define __HASH_SET_TEST_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/az.h>
#include <az/base.h>
#include <az/value.h>
#include <az/packed-value.h>
#include <az/interface.h>
#include <az/instance.h>
#include <az/collections/collection.h>
#include <az/collections/hash-set.h>

#include "unity/unity.h"

#define NUM_ENTRIES 10000

static uint32_t
int32_hash(const AZHashSetImplementation *impl, const void *elem)
{
    uint32_t x = *((const uint32_t *) elem);
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static unsigned int
int32_equal(const AZHashSetImplementation *impl, const void *lhs, const void *rhs)
{
    return *((const uint32_t *) lhs) == *((const uint32_t *) rhs);
}

static unsigned int
align16(unsigned int v)
{
    return (v + 15) & ~(unsigned int) 15;
}

static void
hash_set_impl_setup(AZHashSetImplementation *impl, unsigned int root_size)
{
    unsigned int elem_size = 4;
    unsigned int elem_offset = 8;
    unsigned int entry_size = align16(elem_offset + elem_size);

    az_implementation_init_by_type((AZImplementation *) impl, AZ_TYPE_HASH_SET);
    impl->elem_impl = &AZInt32Klass.impl;
    impl->root_size = root_size;
    impl->entry_size = entry_size;
    impl->elem_offset = elem_offset;
    impl->elem_size = elem_size;
    impl->hash = int32_hash;
    impl->equal = int32_equal;
}

static unsigned int
remove_odd_elem(const void *elem, void *data)
{
    return *((const int32_t *) elem) & 1;
}

static void
test_insert_remove(const AZHashSetImplementation *impl, int32_t *elems, unsigned int n_entries)
{
    AZHashSet hset;
    az_instance_init((const AZImplementation *) impl, &hset);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hset.set.collection.size);

    for (unsigned int i = 0; i < n_entries; i++) {
        TEST_ASSERT(az_hash_set_contains(impl, &hset, &elems[i]));
    }

    for (unsigned int i = 0; i < n_entries; i += 2) {
        TEST_ASSERT(az_hash_set_remove(impl, &hset, &elems[i]));
        TEST_ASSERT(!az_hash_set_contains(impl, &hset, &elems[i]));
    }

    TEST_ASSERT_EQUAL_UINT(n_entries / 2, hset.set.collection.size);

    for (unsigned int i = 1; i < n_entries; i += 2) {
        TEST_ASSERT(az_hash_set_contains(impl, &hset, &elems[i]));
        TEST_ASSERT(az_hash_set_remove(impl, &hset, &elems[i]));
        TEST_ASSERT(!az_hash_set_contains(impl, &hset, &elems[i]));
    }

    TEST_ASSERT_EQUAL_UINT(0, hset.set.collection.size);

    az_instance_finalize((const AZImplementation *) impl, &hset);
}

static void
test_insert_duplicate(const AZHashSetImplementation *impl, int32_t *elems, unsigned int n_entries)
{
    AZHashSet hset;
    az_instance_init((const AZImplementation *) impl, &hset);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hset.set.collection.size);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hset.set.collection.size);

    az_instance_finalize((const AZImplementation *) impl, &hset);
}

static void
test_insert_remove_all(const AZHashSetImplementation *impl, int32_t *elems, unsigned int n_entries)
{
    AZHashSet hset;
    az_instance_init((const AZImplementation *) impl, &hset);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, hset.set.collection.size);

    unsigned int n_removed = az_hash_set_remove_all(impl, &hset, remove_odd_elem, NULL);
    TEST_ASSERT_TRUE(n_removed > 0);
    TEST_ASSERT_EQUAL_UINT(n_entries - n_removed, hset.set.collection.size);

    for (unsigned int i = 0; i < n_entries; i++) {
        if (elems[i] & 1) {
            TEST_ASSERT(!az_hash_set_contains(impl, &hset, &elems[i]));
        } else {
            TEST_ASSERT(az_hash_set_contains(impl, &hset, &elems[i]));
        }
    }

    az_hash_set_clear(impl, &hset);
    TEST_ASSERT_EQUAL_UINT(0, hset.set.collection.size);

    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        TEST_ASSERT(!az_hash_set_contains(impl, &hset, &elems[i]));
    }

    az_instance_finalize((const AZImplementation *) impl, &hset);
}

static void
test_iterator(const AZHashSetImplementation *impl, int32_t *elems, unsigned int n_entries)
{
    AZHashSet hset;
    az_instance_init((const AZImplementation *) impl, &hset);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    const AZCollectionImplementation *coll = &impl->set_impl.collection_impl;

    unsigned int count = 0;
    AZValue iter;
    const AZImplementation *iter_impl = az_collection_get_iterator(coll, &hset.set.collection, &iter);
    while (iter_impl) {
        AZValue val;
        const AZImplementation *elem_impl = az_collection_get_element(coll, &hset.set.collection, &iter, &val, sizeof(AZValue));
        TEST_ASSERT_NOT_NULL(elem_impl);

        TEST_ASSERT(az_hash_set_contains(impl, &hset, &val.int32_v));

        count++;
        iter_impl = az_collection_iterator_next(coll, &hset.set.collection, &iter);
    }

    TEST_ASSERT_EQUAL_UINT(n_entries, count);

    az_hash_set_clear(impl, &hset);
    TEST_ASSERT_EQUAL_UINT(0, hset.set.collection.size);

    iter_impl = az_collection_get_iterator(coll, &hset.set.collection, &iter);
    TEST_ASSERT_NULL(iter_impl);

    az_instance_finalize((const AZImplementation *) impl, &hset);
}

static void
test_collection_interface(const AZHashSetImplementation *impl, int32_t *elems, unsigned int n_entries)
{
    AZHashSet hset;
    az_instance_init((const AZImplementation *) impl, &hset);

    for (unsigned int i = 0; i < n_entries; i++) {
        az_hash_set_insert(impl, &hset, &elems[i]);
    }

    const AZCollectionImplementation *coll = &impl->set_impl.collection_impl;

    TEST_ASSERT_EQUAL_UINT(AZ_TYPE_INT32, az_collection_get_element_type(coll, &hset.set.collection));
    TEST_ASSERT_EQUAL_UINT(n_entries, az_collection_get_size(coll, &hset.set.collection));
    TEST_ASSERT(az_collection_contains(coll, &hset.set.collection, &AZInt32Klass.impl, &elems[0]));
    TEST_ASSERT(!az_collection_contains(coll, &hset.set.collection, &AZInt32Klass.impl, &(int32_t){-999999}));

    az_instance_finalize((const AZImplementation *) impl, &hset);
}

void
test_hash_set(void)
{
    az_init();

    AZHashSetImplementation impl;
    hash_set_impl_setup(&impl, 31);

    int32_t elems[NUM_ENTRIES];

    srand(42);
    for (unsigned int i = 0; i < NUM_ENTRIES; i++) {
        elems[i] = (int32_t) i;
    }

    test_insert_remove(&impl, elems, NUM_ENTRIES);
    test_insert_duplicate(&impl, elems, NUM_ENTRIES);
    test_insert_remove_all(&impl, elems, NUM_ENTRIES);
    test_iterator(&impl, elems, NUM_ENTRIES);
    test_collection_interface(&impl, elems, NUM_ENTRIES);
}
