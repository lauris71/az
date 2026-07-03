#define __AZ_ARRAY_OBJECT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/base.h>
#include <az/boxed-value.h>
#include <az/classes/array-object.h>

#include <az/extend.h>

static void array_object_class_init(AZArrayObjectClass *klass);
static void array_object_instance_init(AZArrayObjectClass *klass, AZArrayObject *aobj);
static void array_object_instance_finalize(AZArrayObjectClass *klass, AZArrayObject *aobj);

static unsigned int array_object_element_type(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
static unsigned int array_object_contains(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *array_object_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

static unsigned int az_array_object_type = 0;
static AZArrayObjectClass *az_array_object_class = NULL;

unsigned int
az_array_object_get_type ()
{
	if (!az_array_object_type) {
		az_array_object_class = (AZArrayObjectClass *) az_register_type (&az_array_object_type, (const unsigned char *) "AZArrayOfObject", AZ_TYPE_OBJECT,
			sizeof (AZArrayObjectClass), sizeof (AZArrayObject), AZ_FLAG_FINAL,
			1, 0,
			(void (*) (AZClass *)) array_object_class_init,
			(void (*) (const AZImplementation *, void *)) array_object_instance_init,
			(void (*) (const AZImplementation *, void *)) array_object_instance_finalize);
	}
	return az_array_object_type;
}

static void
array_object_class_init(AZArrayObjectClass *klass)
{
	az_class_declare_interface((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET(AZArrayObjectClass, list_impl), 0);
	klass->list_impl.collection_impl.get_element_type = array_object_element_type;
	klass->list_impl.collection_impl.contains = array_object_contains;
	klass->list_impl.get_element = array_object_get_element;
}

static void
array_object_instance_init(AZArrayObjectClass *klass, AZArrayObject *obj)
{
}

static void
array_object_instance_finalize(AZArrayObjectClass *klass, AZArrayObject *aobj)
{
	if (!az_object_flags((AZObject *) aobj, AZ_ARRAY_OBJ_FLAG_OWNED)) {
		az_value_delete_array(AZ_IMPL_FROM_TYPE(aobj->array.element_type), aobj->array.values, aobj->array.collection.size);
	}
}

unsigned int
array_object_element_type(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZArrayObject *aof = (AZArrayObject *) coll_inst;
	return aof->array.element_type;
}

unsigned int
array_object_contains(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayObject *aobj = (AZArrayObject *) coll_inst;
	for (unsigned int i = 0; i < aobj->array.collection.size; i++) {
		const AZValue *val = (const AZValue *) ((char *) aobj->array.values + i * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(aobj->array.element_type)));
		if (az_value_equals_instance_autobox(AZ_IMPL_FROM_TYPE(aobj->array.element_type), val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
array_object_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayObject *aobj = (AZArrayObject *) list_inst;
	return az_value_copy_autobox(AZ_IMPL_FROM_TYPE(aobj->array.element_type), val, (const AZValue *) ((char *) aobj->array.values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(aobj->array.element_type))), size);
}

AZArrayObject *
az_array_object_new(unsigned int elem_type, unsigned int length)
{
	arikkei_return_val_if_fail(!AZ_TYPE_IS_INTERFACE(elem_type), NULL);
	arikkei_return_val_if_fail(AZ_TYPE_IS_OBJECT(elem_type) || AZ_TYPE_IS_FINAL(elem_type), NULL);
	AZArrayObject *obj = (AZArrayObject *) az_object_new(AZ_TYPE_ARRAY_OBJECT);
	az_object_set_flags((AZObject *) obj, AZ_ARRAY_OBJ_FLAG_OWNED);
	obj->array.collection.size = length;
	obj->array.element_type = elem_type;
	obj->array.values = az_value_new_array(AZ_IMPL_FROM_TYPE(elem_type), length);
	return obj;
}

AZArrayObject *
az_array_object_new_static(unsigned int elem_type, unsigned int length, void *values)
{
	arikkei_return_val_if_fail(AZ_TYPE_IS_OBJECT(elem_type) || AZ_TYPE_IS_FINAL(elem_type), NULL);
	AZArrayObject *obj = (AZArrayObject *) az_object_new(AZ_TYPE_ARRAY_OBJECT);
	obj->array.collection.size = length;
	obj->array.element_type = elem_type;
	obj->array.values = values;
	return obj;
}

const AZListImplementation *
az_array_object_get_list(AZArrayObject *obj, void **inst)
{
	*inst = obj;
	return &az_array_object_class->list_impl;
}
