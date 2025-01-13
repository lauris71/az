#ifndef __AZ_COLLECTION_H__
#define __AZ_COLLECTION_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_COLLECTION (az_collection_get_type ())

typedef struct _AZCollectionImplementation AZCollectionImplementation;
typedef struct _AZCollectionClass AZCollectionClass;

#include <az/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZCollectionImplementation {
	AZImplementation implementation;
	unsigned int (*get_element_type) (const AZCollectionImplementation *coll_impl, void *coll_inst);
	unsigned int (*get_size) (const AZCollectionImplementation *coll_impl, void *coll_inst);
	unsigned int (*contains) (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
	unsigned int (*get_iterator) (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iterator);
	unsigned int (*iterator_next) (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iterator);
	const AZImplementation *(*get_element) (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZPackedValue *iterator, AZValue64 *val);
};

struct _AZCollectionClass {
	AZInterfaceClass interface_class;
};

unsigned int az_collection_get_type (void);

/* Returns base type that all elements are guaranteed to be assignable to */
unsigned int az_collection_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst);
unsigned int az_collection_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst);
unsigned int az_collection_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
/* Return 0 if unsuccessful */
unsigned int az_collection_get_iterator (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iterator);
unsigned int az_collection_iterator_next (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iterator);
const AZImplementation *az_collection_get_element (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZPackedValue *iterator, AZValue64 *val);

#ifdef __cplusplus
};
#endif

#endif

