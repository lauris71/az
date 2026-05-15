#ifndef __AZ_HASH_SET_H__
#define __AZ_HASH_SET_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2026
*/

#define AZ_TYPE_HASH_SET (az_hash_set_get_type ())

typedef struct _AZHashSet AZHashSet;
typedef struct _AZHashSetImplementation AZHashSetImplementation;
typedef struct _AZHashSetClass AZHashSetClass;
typedef struct _AZHashSetEntry AZHashSetEntry;

#include <az/collections/set.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A set implementation based on a hash table.
 *
 * The collection interface accesses elements.
 * The iterator is 64-bit unsigned integer with the following layout:
 *   - 63-32: current root index
 *   - 31-0: current entry index
 *
 */
struct _AZHashSet {
    AZSet set;
    unsigned int root_size;
    unsigned int size;
    unsigned int free;
    AZHashSetEntry *entries;
};

struct _AZHashSetImplementation {
	AZSetImplementation set_impl;
    const AZImplementation *elem_impl;
    unsigned int root_size;
    unsigned int entry_size;
    uint16_t elem_offset;
    uint16_t elem_size;

    uint32_t (*hash) (const AZHashSetImplementation *impl, const void *elem);
    unsigned int (*equal) (const AZHashSetImplementation *impl, const void *lhs, const void *rhs);
};

struct _AZHashSetClass {
	AZSetClass set_class;
};

unsigned int az_hash_set_get_type (void);

void az_hash_set_insert(const AZHashSetImplementation *impl, AZHashSet *hset, void *elem);
unsigned int az_hash_set_remove(const AZHashSetImplementation *impl, AZHashSet *hset, const void *elem);
void az_hash_set_clear(const AZHashSetImplementation *impl, AZHashSet *hset);
unsigned int az_hash_set_contains(const AZHashSetImplementation *impl, AZHashSet *hset, const void *elem);
unsigned int az_hash_set_forall (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int (* forall) (const void *, void *), void *data);
unsigned int az_hash_set_remove_all (const AZHashSetImplementation *impl, AZHashSet *hset, unsigned int (*remove) (const void *, void *), void *data);

#ifdef __cplusplus
};
#endif

#endif
