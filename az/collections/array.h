#ifndef __AZ_ARRAY_H__
#define __AZ_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

typedef struct _AZArrayImplementation AZArrayImplementation;
typedef struct _AZInterfaceClass AZArrayClass;

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
 * @brief A ligthweight interface for interacting with C arrays
 * 
 * For each C array one can construct separate AZArrayImplementation defining it's type and size.
 * Notice that AZArray does not implement AZCollection to keep the implementation size and initialization
 * needs minimal. If you need a collection (list) access, use an object frontend (or write your own).
 * An array can only contain final types and objects because we do not have any method to keep track of derived types.
 * 
 */
struct _AZArrayImplementation {
	const AZClass *elem_class;
	unsigned int length;
};

unsigned int az_array_get_type ();

void az_array_impl_init(AZArrayImplementation *impl, unsigned int elem_type, unsigned int size);

static inline AZValue *
az_array_value_at (const AZArrayImplementation *impl, void *inst, unsigned int idx)
{
	return (AZValue *) ((char *) inst + idx * AZ_CLASS_ELEMENT_SIZE(impl->elem_class));
}

/**
 * @brief a frontend object to array
 * 
 * It implements array in instance and list in class.
 * 
 */
struct _AZArrayObject {
	AZObject object;
	AZArrayImplementation impl;
	void *values;
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
	return (AZValue *) ((char *) aobj->values + idx * AZ_CLASS_ELEMENT_SIZE(aobj->impl.elem_class));
}

#ifdef __cplusplus
};
#endif

#endif
