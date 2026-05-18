#ifndef __AZ_MAP_H__
#define __AZ_MAP_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#define AZ_TYPE_MAP (az_map_get_type ())

typedef struct _AZMap AZMap; 
typedef struct _AZMapImplementation AZMapImplementation;
typedef struct _AZMapClass AZMapClass;

#include <az/collections/set.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* An hybrid interface that allows lookup by key
*/

struct _AZMap {
	AZCollection collection;
};

struct _AZMapImplementation {
	AZCollectionImplementation collection_impl;
	AZSetImplementation keyset_impl;
	/*
	 * AZCollection methods:
	 *  get_element_type
	 *  get_size
	 *  contains
	 *  get_element
	 * access values
	 */
	unsigned int (*get_key_type) (const AZMapImplementation *map_impl, AZMap *map_inst);
	const AZImplementation *(*get_key) (const AZMapImplementation *map_impl, AZMap *map_inst, const AZValue *iter, AZValue *val, unsigned int size);
	unsigned int (*contains_key) (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, const void *key_inst);
	const AZImplementation *(*lookup) (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size);
};

struct _AZMapClass {
	AZCollectionClass collection_class;
};

unsigned int az_map_get_type (void);

static inline const AZSetImplementation *
az_map_get_keys (const AZMapImplementation *map_impl, AZMap *map_inst, AZSet **inst)
{
	*inst = (AZSet *) map_inst;
	return &map_impl->keyset_impl;
}

static inline unsigned int
az_map_get_key_type (const AZMapImplementation *map_impl, AZMap *map_inst)
{
	return map_impl->get_key_type(map_impl, map_inst);
}

static inline const AZImplementation *
az_map_get_key (const AZMapImplementation *map_impl, AZMap *map_inst, const AZValue *iter, AZValue *val, unsigned int size)
{
	return map_impl->get_key (map_impl, map_inst, iter, val, size);
}

static inline unsigned int
az_map_contains_key (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, const void *key_inst)
{
	return map_impl->contains_key (map_impl, map_inst, key_impl, key_inst);
}

static inline const AZImplementation *
az_map_lookup (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size)
{
	return map_impl->lookup (map_impl, map_inst, key_impl, key_inst, val, size);
}

#ifdef __cplusplus
};
#endif

#endif

