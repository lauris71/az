#ifndef __AZ_COLLECTION_H__
#define __AZ_COLLECTION_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_COLLECTION (az_collection_get_type ())

typedef struct _AZCollection AZCollection;
typedef struct _AZCollectionImplementation AZCollectionImplementation;
typedef struct _AZCollectionClass AZCollectionClass;

#include <az/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZCollectionImplementation {
	AZImplementation implementation;
	unsigned int (*get_element_type) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
	unsigned int (*get_size) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
	unsigned int (*contains) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst);
	const AZImplementation *(*get_iterator) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
	const AZImplementation *(*iterator_next) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
	const AZImplementation *(*get_element) (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZValue *iter, AZValue *val, unsigned int size);
};

struct _AZCollectionClass {
	AZInterfaceClass interface_class;
};

unsigned int az_collection_get_type (void);

/* Returns base type that all elements are guaranteed to be assignable to */
static inline unsigned int
az_collection_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	return coll_impl->get_element_type (coll_impl, coll_inst);
}

static inline unsigned int
az_collection_get_size (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	return coll_impl->get_size (coll_impl, coll_inst);
}

static inline unsigned int
az_collection_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst)
{
	return coll_impl->contains (coll_impl, coll_inst, impl, inst);
}

/* Return 0 if unsuccessful */
static inline const AZImplementation *
az_collection_get_iterator (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	return coll_impl->get_iterator (coll_impl, coll_inst, iter);
}

static inline const AZImplementation *
az_collection_iterator_next (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	return coll_impl->iterator_next (coll_impl, coll_inst, iter);
}

static inline const AZImplementation *
az_collection_get_element (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZValue *iter, AZValue *val, unsigned int size)
{
	return coll_impl->get_element (coll_impl, coll_inst, iter, val, size);
}

#ifdef __cplusplus
};
#endif

#endif

