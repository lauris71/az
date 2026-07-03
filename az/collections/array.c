#define __AZ_ARRAY_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/base.h>
#include <az/boxed-value.h>
#include <az/collections/array.h>

#include <az/extend.h>

static unsigned int array_type = 0;
static AZArrayClass *array_class;

static void array_class_init(AZArrayClass *klass);
static unsigned int array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
static unsigned int array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);

static unsigned int array_get_element_type(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
static unsigned int array_contains(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

unsigned int
az_array_get_type ()
{
	if (!array_type) {
		array_class = (AZArrayClass *) az_register_type (&array_type, (const unsigned char *) "AZArray", AZ_TYPE_STRUCT,
			sizeof(AZArrayClass), sizeof(AZArray), 0,
			1, 0,
			(void (*) (AZClass *)) array_class_init,
			NULL,
			NULL);
	}
	return array_type;
}

static void
	array_class_init (AZArrayClass *klass)
{
	az_class_declare_interface(&klass->klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET(AZArrayClass, list_impl), 0);
	klass->klass.serialize = array_serialize;
	klass->klass.deserialize = array_deserialize;
	klass->list_impl.collection_impl.get_element_type = array_get_element_type;
	klass->list_impl.collection_impl.contains = array_contains;
	klass->list_impl.get_element = array_get_element;
}

static unsigned int
array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZArray *array = (AZArray *) inst;
	unsigned int len = az_instance_serialize(&AZUint32Klass.impl, &array->element_type, d, dlen, ctx);
	len += az_instance_serialize(&AZUint32Klass.impl, &array->collection.size, d, dlen, ctx);
	for (unsigned int i = 0; i < array->collection.size; i++) {
		len += az_instance_serialize(AZ_IMPL_FROM_TYPE(array->element_type), az_array_value_at(array, i), d + len, (len <= dlen) ? dlen - len : 0, ctx);
	}
	return len;
}

/* fixme: Exception on EOF */
static unsigned int
array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZArray *array = (AZArray *) value;
	unsigned int len = 0;
	len += az_value_deserialize(&AZUint32Klass.impl, (AZValue *) &array->element_type, s + len, (len <= slen) ? slen - len : 0, ctx);
	len += az_value_deserialize(&AZUint32Klass.impl, (AZValue *) &array->collection.size, s + len, (len <= slen) ? slen - len : 0, ctx);
	array->values = az_value_new_array(AZ_IMPL_FROM_TYPE(array->element_type), array->collection.size);
	for (unsigned int i = 0; i < array->collection.size; i++) {
		len += az_value_deserialize(AZ_IMPL_FROM_TYPE(array->element_type), az_array_value_at(array, i), s + len, (len <= slen) ? slen - len : 0, ctx);
	}
	return len;
}

unsigned int
array_get_element_type(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZArray *array = (AZArray *) coll_inst;
	return array->element_type;
}

unsigned int
array_contains(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZArray *array = (AZArray *) coll_inst;
	for (unsigned int i = 0; i < array->collection.size; i++) {
		const AZValue *val = (const AZValue *) ((char *) array->values + i * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(array->element_type)));
		if (az_value_equals_instance_autobox(AZ_IMPL_FROM_TYPE(array->element_type), val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArray *array = (AZArray *) list_inst;
	return az_value_copy_autobox(AZ_IMPL_FROM_TYPE(array->element_type), val, (const AZValue *) ((char *) array->values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(array->element_type))), size);
}
