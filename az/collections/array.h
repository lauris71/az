#ifndef __AZ_ARRAY_H__
#define __AZ_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

typedef struct _AZArray AZArray;
typedef struct _AZArrayClass AZArrayClass;

typedef struct _AZArrayObject AZArrayObject;
typedef struct _AZArrayObjectClass AZArrayObjectClass;

#define AZ_TYPE_ARRAY az_array_get_type()
#define AZ_TYPE_ARRAY_OBJECT az_array_object_get_type()

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
	uint32_t elem_type;
	uint32_t length;
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
	return (AZValue *) ((char *) array->values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(array->elem_type)));
}

/**
 * @brief a frontend object to array
 * 
 * It encapsulates an array and ensures the lifecycle
 * 
 */
struct _AZArrayObject {
	AZObject object;
	AZArray array;
};

struct _AZArrayObjectClass {
	AZObjectClass object_class;
	AZListImplementation list_impl;
};

#define AZ_ARRAY_OBJ_FLAG_OWNED 0x2

unsigned int az_array_object_get_type ();

AZArrayObject *az_array_object_new(unsigned int elem_type, unsigned int size);
AZArrayObject *az_array_object_new_static(unsigned int elem_type, unsigned int size, void *values);

const AZListImplementation *az_array_object_get_list(AZArrayObject *obj, void **inst);

static inline AZValue *
az_array_object_value_at (AZArrayObject *aobj, unsigned int idx)
{
	return (AZValue *) ((char *) aobj->array.values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(aobj->array.elem_type)));
}

#ifdef __cplusplus
};
#endif

#endif
