#define __AZ_ARRAY_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/collections/array.h>
#include <az/boxed-value.h>

#include <az/extend.h>

static unsigned int array_of_type = 0;
static AZArrayClass *array_of_class;

static void array_class_init(AZArrayClass *klass);
static void array_implementation_init(AZArrayImplementation *impl);
static unsigned int array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
static unsigned int array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);

unsigned int
az_array_get_type ()
{
	if (!array_of_type) {
		array_of_class = (AZArrayClass *) az_register_interface_type (&array_of_type, (const unsigned char *) "AZArray", AZ_TYPE_INTERFACE,
			sizeof (AZArrayClass), sizeof (AZArrayImplementation), 0, 0,
			(void (*) (AZClass *)) array_class_init,
			(void (*) (AZImplementation *)) array_implementation_init,
			NULL, NULL);
	}
	return array_of_type;
}

static void
array_class_init (AZArrayClass *klass)
{
	klass->klass.serialize = array_serialize;
	klass->klass.deserialize = array_deserialize;
}

static void
array_implementation_init (AZArrayImplementation *impl)
{
	impl->elem_class = NULL;
	impl->length = 0;
}

static unsigned int
array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZArrayImplementation *aimpl = (AZArrayImplementation *) impl;
	unsigned int len = 0;
	for (unsigned int i = 0; i < aimpl->length; i++) {
		len += az_instance_serialize(&aimpl->elem_class->impl, az_array_value_at(aimpl, inst, i), d + len, (len <= dlen) ? dlen - len : 0, ctx);
	}
	return len;
}

static unsigned int
array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZArrayImplementation *aimpl = (AZArrayImplementation *) impl;
	void *inst = az_value_new_array(&aimpl->elem_class->impl, aimpl->length);
	unsigned int len = 0;
	for (unsigned int i = 0; i < aimpl->length; i++) {
		len += az_value_deserialize(&aimpl->elem_class->impl, az_array_value_at(aimpl, inst, i), s + len, slen - len, ctx);
	}
	value->block = inst;
	return len;
}

void
az_array_impl_init(AZArrayImplementation *impl, unsigned int elem_type, unsigned int length)
{
	az_implementation_init_by_type((AZImplementation *) impl, AZ_TYPE_ARRAY);
	arikkei_return_if_fail(!AZ_TYPE_IS_INTERFACE(elem_type));
	arikkei_return_if_fail(AZ_TYPE_IS_OBJECT(elem_type) || AZ_TYPE_IS_FINAL(elem_type));
	impl->elem_class = AZ_CLASS_FROM_TYPE(elem_type);
	impl->length = length;
}

static void array_object_class_init(AZArrayObjectClass *klass);
static void array_object_instance_init(AZArrayObjectClass *klass, AZArrayObject *aobj);
static void array_object_instance_finalize(AZArrayObjectClass *klass, AZArrayObject *aobj);

static unsigned int array_object_element_type(const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int array_object_size(const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int array_object_contains(const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *array_object_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

static unsigned int az_array_object_type = 0;
static AZArrayObjectClass *az_array_object_class = NULL;

unsigned int
az_array_object_get_type ()
{
	if (!az_array_object_type) {
		az_array_object_class = (AZArrayObjectClass *) az_register_type (&az_array_object_type, (const unsigned char *) "AZArrayOfObject", AZ_TYPE_OBJECT,
			sizeof (AZArrayObjectClass), sizeof (AZArrayObject), AZ_FLAG_FINAL,
			(void (*) (AZClass *)) array_object_class_init,
			(void (*) (const AZImplementation *, void *)) array_object_instance_init,
			(void (*) (const AZImplementation *, void *)) array_object_instance_finalize);
	}
	return az_array_object_type;
}

static void
array_object_class_init(AZArrayObjectClass *klass)
{
	az_class_set_num_interfaces((AZClass *) klass, 1);
	az_class_declare_interface((AZClass *) klass, 0, AZ_TYPE_ARRAY, ARIKKEI_OFFSET(AZArrayObjectClass, list_impl), 0);
	klass->list_impl.collection_impl.get_element_type = array_object_element_type;
	klass->list_impl.collection_impl.get_size = array_object_size;
	klass->list_impl.collection_impl.contains = array_object_contains;
	klass->list_impl.get_element = array_object_get_element;
}

static void
array_object_instance_init(AZArrayObjectClass *klass, AZArrayObject *obj)
{
	az_implementation_init_by_type((AZImplementation *) &obj->impl, AZ_TYPE_ARRAY);
}

static void
array_object_instance_finalize(AZArrayObjectClass *klass, AZArrayObject *aobj)
{
	if (!az_object_flags((AZObject *) aobj, AZ_ARRAY_OBJ_FLAG_OWNED)) {
		az_value_delete_array(&aobj->impl.elem_class->impl, aobj->values, aobj->impl.length);
	}
}

unsigned int
array_object_element_type(const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZArrayObject *aof = (AZArrayObject *) coll_inst;
	return aof->impl.elem_class->impl.type;
}

unsigned int
array_object_size(const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZArrayObject *aof = (AZArrayObject *) coll_inst;
	return aof->impl.length;
}

unsigned int
array_object_contains(const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayObject *aof = (AZArrayObject *) coll_inst;
	for (unsigned int i = 0; i < aof->impl.length; i++) {
		const AZValue *val = (const AZValue *) ((char *) inst + i * AZ_CLASS_ELEMENT_SIZE(aof->impl.elem_class));
		if (az_value_equals_instance_autobox(&aof->impl.elem_class->impl, val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
array_object_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayObject *aof = (AZArrayObject *) list_inst;
	return az_value_copy_autobox(&aof->impl.elem_class->impl, val, (const AZValue *) ((char *) aof->values + idx * AZ_CLASS_ELEMENT_SIZE(aof->impl.elem_class)), size);
}

AZArrayObject *
az_array_object_new(unsigned int elem_type, unsigned int length)
{
	arikkei_return_val_if_fail(!AZ_TYPE_IS_INTERFACE(elem_type), NULL);
	arikkei_return_val_if_fail(AZ_TYPE_IS_OBJECT(elem_type) || AZ_TYPE_IS_FINAL(elem_type), NULL);
	AZArrayObject *obj = (AZArrayObject *) az_object_new(AZ_TYPE_ARRAY_OBJECT);
	az_object_set_flags((AZObject *) obj, AZ_ARRAY_OBJ_FLAG_OWNED);
	obj->impl.elem_class = AZ_CLASS_FROM_TYPE(elem_type);
	obj->impl.length = length;
	obj->values = az_value_new_array(&obj->impl.elem_class->impl, length);
	return obj;
}

AZArrayObject *
az_array_object_new_static(unsigned int elem_type, unsigned int length, void *values)
{
	arikkei_return_val_if_fail(AZ_TYPE_IS_OBJECT(elem_type) || AZ_TYPE_IS_FINAL(elem_type), NULL);
	AZArrayObject *obj = (AZArrayObject *) az_object_new(AZ_TYPE_ARRAY_OBJECT);
	obj->values = values;
	obj->impl.elem_class = AZ_CLASS_FROM_TYPE(elem_type);
	obj->impl.length = length;
	return obj;
}

const AZListImplementation *
az_array_object_get_list(AZArrayObject *obj, void **inst)
{
	*inst = obj;
	return &az_array_object_class->list_impl;
}
