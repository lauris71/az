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

    uint32_t (*hash) (AZHashMapImplementation *impl, const AZValue *key);
	unsigned int (*equal) (AZHashMapImplementation *impl, const AZValue *lhs, const AZValue *rhs);
};

struct _AZHashMapClass {
	AZMapClass map_class;
};

unsigned int az_hash_map_get_type (void);

void az_hash_map_insert(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key, const AZValue *val);
unsigned int az_hash_map_remove(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key);
void az_hash_map_clear(AZHashMapImplementation *impl, AZHashMap *hmap);
unsigned int az_hash_map_exists(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key);
const AZValue *az_hash_map_lookup(AZHashMapImplementation *impl, AZHashMap *hmap, const AZValue *key);
/* Stop if forall returns 0 */
unsigned int az_hash_map_forall (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (* forall) (const AZValue *, const AZValue *, void *), void *data);
/* Remove entry if remove returns 1, return number of entries removed */
unsigned int az_hash_map_remove_all (AZHashMapImplementation *impl, AZHashMap *hmap, unsigned int (*remove) (const AZValue *, const AZValue *, void *), void *data);

#ifdef __cplusplus
};
#endif

#endif
