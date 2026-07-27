#ifndef __AZ_ARRAY_H__
#define __AZ_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#define AZ_TYPE_ARRAY (az_array_get_type ())

typedef struct _AZArray AZArray;
typedef struct _AZArrayImplementation AZArrayImplementation;
typedef struct _AZArrayClass AZArrayClass;

#include <az/collections/list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A lightweight interface for interacting with C arrays
 *
 * An array can only contain final types or objects because we do not have any method to keep
 * track of the actual implementation of a type.
 * It is an interface because it does not own the data and thus cannot be stored in general case
 * without providing the data owning instance together with it.
 */

struct _AZArray {
	AZList list;
	void *values;
};

struct _AZArrayImplementation {
	AZListImplementation list_impl;
	const AZImplementation *elem_impl;
};

struct _AZArrayClass {
	AZListClass list_class;
};

unsigned int az_array_get_type (void);

static inline AZValue *
az_array_value_at (const AZArrayImplementation *array_impl, const AZArray *array, unsigned int idx)
{
	return (AZValue *) ((char *) array->values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_IMPL(array_impl->elem_impl)));
}

/**
 * @brief Deserializes an array into uninitialized instance
 * 
 * If successful it allocates the storage and the caller is responsible for managing it
 * 
 * @param array_impl The array implementation
 * @param array An unitialized array instance
 * @param s The source buffer
 * @param slen The source buffer length
 * @param ctx The executuon context
 * @return The number of bytes consumed
 */
unsigned int az_array_deserialize (const AZArrayImplementation *array_impl, AZArray *array, const unsigned char *s, unsigned int slen, AZContext *ctx);

#ifdef __cplusplus
};
#endif

#endif
