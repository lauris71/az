#define __AZ_ARRAY_C__

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
#include <az/collections/array.h>

#include <az/extend.h>

static unsigned int array_type = 0;
static AZArrayClass *array_class;

static void array_class_init (AZArrayClass *klass);
static void array_implementation_init (AZArrayImplementation *impl);
static unsigned int array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
static unsigned int array_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen);

static unsigned int array_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
static unsigned int array_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

unsigned int
az_array_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(array_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!array_type) {
		array_class = (AZArrayClass *) az_register_interface_type (&array_type, (const unsigned char *) "AZArray", AZ_TYPE_LIST,
			sizeof(AZArrayClass), sizeof(AZArrayImplementation), sizeof(AZArray), AZ_FLAG_ZERO_MEMORY,
			0, 0,
			(void (*) (AZClass *)) array_class_init,
			(void (*) (AZImplementation *)) array_implementation_init,
			NULL, NULL);
	}
	t = array_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
array_class_init (AZArrayClass *klass)
{
	((AZClass *) klass)->serialize = array_serialize;
	((AZClass *) klass)->to_string = array_to_string;
}

static void
array_implementation_init (AZArrayImplementation *impl)
{
	impl->list_impl.collection_impl.get_element_type = array_get_element_type;
	impl->list_impl.collection_impl.contains = array_contains;
	impl->list_impl.get_element = array_get_element;
	impl->elem_impl = NULL;
}

static unsigned int
array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) impl;
	AZArray *array = (AZArray *) inst;
	unsigned int len = az_instance_serialize(&AZUint64Klass.impl, &array->list.collection.size, d, dlen, ctx);
	for (unsigned int i = 0; i < array->list.collection.size; i++) {
		len += az_instance_serialize(array_impl->elem_impl, az_array_value_at(array_impl, array, i), d + len, (len <= dlen) ? dlen - len : 0, ctx);
	}
	return len;
}

static unsigned int
array_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) impl;
	AZArray *array = (AZArray *) inst;
	unsigned int pos = 0;
	arikkei_return_val_if_fail (!array->list.collection.size || (array_impl->elem_impl != NULL), 0);
	/* Nothing is written when destination is NULL */
	if (!d) dlen = 0;
	if (d && (pos < dlen)) d[pos] = '[';
	pos++;
	for (unsigned int i = 0; i < array->list.collection.size; i++) {
		if (i) {
			if (d && (pos < dlen)) d[pos] = ',';
			pos++;
		}
		pos += az_instance_to_string (array_impl->elem_impl, az_value_get_inst (array_impl->elem_impl, az_array_value_at (array_impl, array, i)), (d) ? d + pos : NULL, (dlen > pos) ? dlen - pos : 0);
	}
	if (d && (pos < dlen)) d[pos] = ']';
	pos++;
	if (d && (pos < dlen)) d[pos] = 0;
	return pos;
}

static unsigned int
array_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) coll_impl;
	return AZ_IMPL_TYPE(array_impl->elem_impl);
}

static unsigned int
array_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) coll_impl;
	AZArray *array = (AZArray *) coll_inst;
	for (unsigned int i = 0; i < array->list.collection.size; i++) {
		const AZValue *val = (const AZValue *) ((char *) array->values + i * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(AZ_IMPL_TYPE(array_impl->elem_impl))));
		if (az_value_equals_instance_autobox(array_impl->elem_impl, val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) list_impl;
	AZArray *array = (AZArray *) list_inst;
	return az_value_copy_autobox(array_impl->elem_impl, val, (const AZValue *) ((char *) array->values + idx * AZ_CLASS_ELEMENT_SIZE(AZ_CLASS_FROM_TYPE(AZ_IMPL_TYPE(array_impl->elem_impl)))), size);
}

unsigned int
az_array_deserialize (const AZArrayImplementation *array_impl, AZArray *array, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	unsigned int len = az_value_deserialize(&AZUint64Klass.impl, (AZValue *) &array->list.collection.size, s + len, (len <= slen) ? slen - len : 0, ctx);
	array->values = az_value_new_array(array_impl->elem_impl, array->list.collection.size);
	for (unsigned int i = 0; i < array->list.collection.size; i++) {
		len += az_value_deserialize(array_impl->elem_impl, az_array_value_at(array_impl, array, i), s + len, (len <= slen) ? slen - len : 0, ctx);
	}
	return len;
}
