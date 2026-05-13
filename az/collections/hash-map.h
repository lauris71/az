#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2026
*/

#define AZ_TYPE_HASH_MAP (az_hash_map_get_type ())

typedef struct _AZHashMap AZHashMap;
typedef struct _AZHashMapImplementation AZHashMapImplementation;
typedef struct _AZHashMapClass AZHashMapClass;
typedef struct _AZHashMapEntry AZHashMapEntry;

#include <az/collections/map.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A map implementation based on a hash table.
 * 
 * The collection interface access values.
 * The iterator is 64-bit unsigned integer with the following layout:
 *   - 63-32: current root index
 *   - 31-0: current entry index
 * 
 */
struct _AZHashMap {
    unsigned int root_size;
    unsigned int size;
    unsigned int free;
    unsigned int n_entries;
    AZHashMapEntry *entries;
};

struct _AZHashMapImplementation {
	AZMapImplementation map_impl;
    const AZImplementation *key_impl;
    const AZImplementation *val_impl;
    unsigned int root_size;
    unsigned int entry_size;
	uint16_t key_offset;
	uint16_t key_size;
	uint16_t val_offset;
	uint16_t val_size;

    uint32_t (*hash) (const AZHashMapImplementation *impl, const void *key);
    unsigned int (*equal) (const AZHashMapImplementation *impl, const void *lhs, const void *rhs);
};

struct _AZHashMapClass {
	AZMapClass map_class;
};

unsigned int az_hash_map_get_type (void);

void az_hash_map_insert(const AZHashMapImplementation *impl, AZHashMap *hmap, void *key, void *val);
unsigned int az_hash_map_remove(const AZHashMapImplementation *impl, AZHashMap *hmap, const void *key);
void az_hash_map_clear(const AZHashMapImplementation *impl, AZHashMap *hmap);
unsigned int az_hash_map_exists(const AZHashMapImplementation *impl, AZHashMap *hmap, const void *key);
unsigned int az_hash_map_exists_val(const AZHashMapImplementation *impl, AZHashMap *hmap, const void *val);
const void *az_hash_map_lookup(const AZHashMapImplementation *impl, AZHashMap *hmap, const void *key);
unsigned int az_hash_map_forall (const AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (* forall) (const void *, const void *, void *), void *data);
unsigned int az_hash_map_remove_all (const AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (*remove) (const void *, const void *, void *), void *data);

#ifdef __cplusplus
};
#endif

#endif
