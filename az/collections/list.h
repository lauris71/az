#ifndef __AZ_LIST_H__
#define __AZ_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_LIST (az_list_get_type ())

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

#ifdef __cplusplus
};
#endif

#endif

