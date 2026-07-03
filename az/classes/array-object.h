#ifndef __AZ_ARRAY_OBJECT_H__
#define __AZ_ARRAY_OBJECT_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

typedef struct _AZArrayObject AZArrayObject;
typedef struct _AZArrayObjectClass AZArrayObjectClass;

#define AZ_TYPE_ARRAY_OBJECT az_array_object_get_type()

#include <az/object.h>
#include <az/collections/array.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZArrayObject {
	AZObject object;
	AZArray array;
};

struct _AZArrayObjectClass {
	AZObjectClass object_class;
	AZArrayImplementation array_impl;
};

#define AZ_ARRAY_OBJ_FLAG_OWNED 0x2

unsigned int az_array_object_get_type ();

AZArrayObject *az_array_object_new(unsigned int elem_type, unsigned int size);
AZArrayObject *az_array_object_new_static(unsigned int elem_type, unsigned int size, void *values);

const AZListImplementation *az_array_object_get_list(AZArrayObject *obj, void **inst);

static inline AZValue *
az_array_object_value_at (AZArrayObject *aobj, unsigned int idx)
{
	const AZArrayObjectClass *klass = (const AZArrayObjectClass *) ((AZObject *) aobj)->klass;
	return (AZValue *) ((char *) aobj->array.values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_IMPL(klass->array_impl.elem_impl)));
}

#ifdef __cplusplus
};
#endif

#endif
