#ifndef __AZ_ARRAY_H__
#define __AZ_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

typedef struct _AZArray AZArray;
typedef struct _AZArrayClass AZArrayClass;

#define AZ_TYPE_ARRAY az_array_get_type()

#include <az/collections/list.h>
#include <az/object.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A ligthweight value type for interacting with C arrays
 * 
 * An array can only contain final types or objects because we do not have any method to keep track of derived types.
 * 
 */
struct _AZArray {
	AZCollection collection;
	unsigned int element_type;
	void *values;
};

struct _AZArrayClass {
	AZClass klass;
	AZListImplementation list_impl;
};

unsigned int az_array_get_type ();

static inline AZValue *
az_array_value_at (const AZArray *array, unsigned int idx)
{
	return (AZValue *) ((char *) array->values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(array->element_type)));
}

#ifdef __cplusplus
};
#endif

#endif
