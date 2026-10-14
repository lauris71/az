#ifndef __AZ_SET_H__
#define __AZ_SET_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#define AZ_TYPE_SET (az_set_get_type ())

typedef struct _AZSet AZSet;
typedef struct _AZSetImplementation AZSetImplementation;
typedef struct _AZSetClass AZSetClass;

#include <az/collections/collection.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZSetImplementation {
	AZCollectionImplementation collection_impl;
};

struct _AZSetClass {
	AZCollectionClass collection_class;
};

unsigned int az_set_get_type (void);

#ifdef __cplusplus
};
#endif

#endif
