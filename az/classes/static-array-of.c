#define __AZ_STATIC_ARRAY_OF_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/private.h>
#include <az/extend.h>
#include <az/boxed-value.h>
#include <az/collections/array.h>

#include <az/classes/static-array-of.h>

static unsigned int abstract_static_array_of_type = 0;

unsigned int
az_abstract_static_array_of_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(abstract_static_array_of_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!abstract_static_array_of_type) {
		az_register_type (&abstract_static_array_of_type, (const unsigned char *) "AbstractStaticArrayOf", AZ_TYPE_STRUCT, sizeof (AZStaticArrayOfClass), sizeof (AZStaticArrayOf), AZ_FLAG_ABSTRACT,
			0, 0,
			NULL, NULL, NULL);
	}
	t = abstract_static_array_of_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void static_array_of_class_init (AZStaticArrayOfClass *klass, AZClass *elem_class);
static void static_array_of_instance_init (AZStaticArrayOfClass *klass, AZStaticArrayOf *sarr);
static unsigned int array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
static unsigned int array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);
static unsigned int array_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen);
static unsigned int static_array_of_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
static unsigned int static_array_of_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *static_array_of_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

unsigned int
az_static_array_of_get_type (unsigned int element_type)
{
	static unsigned int num_subtypes = 0;
	static unsigned int *subtypes = NULL;
	arikkei_return_val_if_fail (AZ_TYPE_IS_VALUE(element_type), 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_FINAL(element_type) || AZ_TYPE_IS_OBJECT(element_type), 0);
	AZ_TYPES_LOCK();
	if (AZ_TYPE_INDEX(element_type) >= num_subtypes) {
		unsigned int new_size = (AZ_TYPE_INDEX(element_type) + 1 + 255) & 0xffffff00;
		subtypes = realloc (subtypes, new_size * sizeof (unsigned int));
		memset (&subtypes[num_subtypes], 0, (new_size - num_subtypes) * sizeof (unsigned int));
		num_subtypes = new_size;
	}
	if (!subtypes[AZ_TYPE_INDEX(element_type)]) {
		AZClass *elem_class = AZ_CLASS_FROM_TYPE(element_type);
		unsigned int nlen = (unsigned int) strlen ((const char *) elem_class->name);
		unsigned char *name = malloc (nlen + 16);
		snprintf ((char *) name, nlen + 16, "StaticArrayOf%s", elem_class->name);
		az_register_composite_type (&subtypes[AZ_TYPE_INDEX(element_type)], name, AZ_TYPE_ABSTRACT_STATIC_ARRAY_OF, sizeof (AZStaticArrayOfClass), sizeof (AZStaticArrayOf), AZ_FLAG_FINAL,
			1, 0,
			(void (*) (AZClass *, void *)) static_array_of_class_init,
			(void (*) (const AZImplementation *, void *)) static_array_of_instance_init,
			NULL,
			elem_class);
		free (name);
	}
	unsigned int type = subtypes[AZ_TYPE_INDEX(element_type)];
	AZ_TYPES_UNLOCK();
	return type;
}

static void
static_array_of_class_init (AZStaticArrayOfClass *klass, AZClass *elem_class)
{
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_ARRAY, ARIKKEI_OFFSET(AZStaticArrayOfClass, array_impl), ARIKKEI_OFFSET(AZStaticArrayOf, array));
	klass->array_impl.list_impl.collection_impl.get_element_type = static_array_of_get_element_type;
	klass->array_impl.list_impl.collection_impl.contains = static_array_of_contains;
	klass->array_impl.list_impl.get_element = static_array_of_get_element;
	klass->array_impl.elem_impl = &elem_class->impl;
	((AZClass *) klass)->serialize = array_serialize;
	/*
	 * WARNING: deserialize allocates memory for the values array. Since AZStaticArrayOf
	 * is a value type with externally-managed storage, the caller is responsible for
	 * freeing this memory after deserialization.
	 */
	((AZClass *) klass)->deserialize = array_deserialize;
	((AZClass *) klass)->to_string = array_to_string;
}

static void
static_array_of_instance_init (AZStaticArrayOfClass *klass, AZStaticArrayOf *sarr)
{
	sarr->array.values = NULL;
	sarr->array.list.collection.size = 0;
}

static unsigned int
array_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZStaticArrayOfClass *array_klass = (AZStaticArrayOfClass *) impl;
	AZStaticArrayOf *array = (AZStaticArrayOf *) inst;
	return az_instance_serialize((const AZImplementation *) &array_klass->array_impl, &array->array, d, dlen, ctx);
}

static unsigned int
array_deserialize (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZStaticArrayOfClass *array_klass = (AZStaticArrayOfClass *) impl;
	AZStaticArrayOf *array = (AZStaticArrayOf *) value;
	return az_array_deserialize(&array_klass->array_impl, &array->array, s, slen, ctx);
}

static unsigned int
array_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen)
{
	AZStaticArrayOfClass *array_klass = (AZStaticArrayOfClass *) impl;
	AZStaticArrayOf *array = (AZStaticArrayOf *) inst;
	return az_instance_to_string((const AZImplementation *) &array_klass->array_impl, &array->array, d, dlen);
}

static unsigned int
static_array_of_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) coll_impl;
	return AZ_IMPL_TYPE(array_impl->elem_impl);
}

static unsigned int
static_array_of_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) coll_impl;
	AZStaticArrayOf *sarr = (AZStaticArrayOf *) coll_inst;
	for (unsigned int i = 0; i < sarr->array.list.collection.size; i++) {
		const AZValue *val = az_array_value_at(array_impl, sarr, i);
		if (az_value_equals_instance_autobox(array_impl->elem_impl, val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
static_array_of_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayImplementation *array_impl = (AZArrayImplementation *) list_impl;
	AZStaticArrayOf *sarr = (AZStaticArrayOf *) list_inst;
	return az_value_copy_autobox(array_impl->elem_impl, val, az_array_value_at(array_impl, sarr, idx), size);
}
