#ifndef __AZ_LIST_H__
#define __AZ_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_LIST (az_list_get_type ())

typedef struct _AZList AZList;
typedef struct _AZListImplementation AZListImplementation;
typedef struct _AZListClass AZListClass;

#include <az/collections/collection.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* An hybrid interface that allows sequential access by index
* The iterator of a list is always unsigned integer
*/

struct _AZListImplementation {
	AZCollectionImplementation collection_impl;
	const AZImplementation *(*get_element) (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);
	int (*set_element) (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, const AZImplementation *impl, void *inst);
};

struct _AZListClass {
	AZCollectionClass collection_class;
};

unsigned int az_list_get_type (void);

static inline const AZImplementation *
az_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	return list_impl->get_element (list_impl, list_inst, idx, val, size);
}

static inline int
az_list_set_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, const AZImplementation *impl, void *inst)
{
	if (!list_impl->set_element) {
		return AZ_NOT_IMPLEMENTED;
	}
	return list_impl->set_element (list_impl, list_inst, idx, impl, inst);
}

#ifdef __cplusplus
};
#endif

#endif

