#ifndef __AZ_MAP_H__
#define __AZ_MAP_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#define AZ_TYPE_MAP (az_map_get_type ())

typedef struct _AZMapImplementation AZMapImplementation;
typedef struct _AZMapClass AZMapClass;

#include <az/classes/collection.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* An hybrid interface that allows lookup by key
*/

struct _AZMapImplementation {
	AZCollectionImplementation collection_impl;
	unsigned int (*get_key_type) (const AZMapImplementation *map_impl, void *map_inst);
	/* Get corresponding key using the same iterator as value */
	const AZImplementation *(*get_key) (const AZMapImplementation *map_impl, void *map_inst, const AZPackedValue *iter, AZValue64 *val);
	/* Get keys as distinct collection */
	const AZCollectionImplementation *(*get_keys) (const AZMapImplementation *map_impl, void *map_inst, void **inst);
	const AZImplementation *(*lookup) (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue64 *val);
};

struct _AZMapClass {
	AZCollectionClass collection_class;
};

unsigned int az_map_get_type (void);

unsigned int az_map_get_key_type (const AZMapImplementation *map_impl, void *map_inst);
/* Get corresponding key using the same iterator as value */
const AZImplementation *az_map_get_key (const AZMapImplementation *map_impl, void *map_inst, const AZPackedValue *iterator, AZValue64 *val);
/* Get keys as distinct collection */
const AZCollectionImplementation *az_map_get_keys (const AZMapImplementation *map_impl, void *map_inst, void **inst);
const AZImplementation *az_map_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue64 *val);

#ifdef __cplusplus
};
#endif

#endif

