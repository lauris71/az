#ifndef __AZ_STATIC_ARRAY_OF_H__
#define __AZ_STATIC_ARRAY_OF_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

typedef struct _AZStaticArrayOf AZStaticArrayOf;
typedef struct _AZStaticArrayOfClass AZStaticArrayOfClass;

#define AZ_TYPE_ABSTRACT_STATIC_ARRAY_OF az_abstract_static_array_of_get_type ()
#define AZ_TYPE_STATIC_ARRAY_OF(t) az_static_array_of_get_type (t)

#include <arikkei/arikkei-utils.h>

#include <az/collections/array.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Composite value type that wraps a fixed-size C array of final value types.
 *
 * The element array is externally managed (static or allocated outside this code),
 * so the type never allocates or frees the values pointer.
 *
 * Being a value type (parent AZ_TYPE_STRUCT), instances are passed by value
 * and have no reference counting.
 */

struct _AZStaticArrayOf {
	AZArray array;
};

struct _AZStaticArrayOfClass {
	AZClass klass;
	AZArrayImplementation array_impl;
};

unsigned int az_abstract_static_array_of_get_type (void);
unsigned int az_static_array_of_get_type (unsigned int element_type);

ARIKKEI_INLINE AZValue *
az_static_array_of_value_at (const AZStaticArrayOfClass *klass, const AZStaticArrayOf *sarr, unsigned int idx)
{
	return az_array_value_at(&klass->array_impl, &sarr->array, idx);
}

#ifdef __cplusplus
};
#endif

#endif
