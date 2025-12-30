#define __AZ_LIST_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <az/interface.h>
#include <az/packed-value.h>
#include <az/extend.h>

#include "list.h"

/* AZInterface implementation */
static void list_class_init (AZListClass *klass);
static void list_implementation_init (AZListImplementation *impl);
/* AZCollection implementation */
static unsigned int list_get_iterator (const AZCollectionImplementation *collection_impl, void *collection_inst, AZPackedValue *iterator);
static unsigned int list_iterator_next (const AZCollectionImplementation *collection_impl, void *collection_inst, AZPackedValue *iterator);
static const AZImplementation *list_get_element (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZPackedValue *iterator, AZValue *val, unsigned int size);
/* AZInstance implementation */
static unsigned int list_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);

static unsigned int list_type = 0;
static AZListClass *list_class;

enum {
	PROP_LENGTH,
	NUM_PROPERTIES
};

unsigned int
az_list_get_type (void)
{
	if (!list_type) {
		list_class = (AZListClass *) az_register_interface_type (&list_type, (const unsigned char *) "AZList", AZ_TYPE_COLLECTION,
			sizeof (AZListClass), sizeof (AZListImplementation), 0, AZ_FLAG_ZERO_MEMORY,
			(void (*) (AZClass *)) list_class_init,
			(void (*) (AZImplementation *)) list_implementation_init,
			NULL, NULL);
	}
	return list_type;
}

static void
list_class_init (AZListClass *klass)
{
	az_class_set_num_properties ((AZClass*) klass, NUM_PROPERTIES);
	az_class_define_property ((AZClass*) klass, PROP_LENGTH, (const unsigned char *) "length", AZ_TYPE_UINT32, 0, AZ_FIELD_INSTANCE, AZ_FIELD_READ_METHOD, 0, 0, NULL, NULL);
	klass->collection_class.interface_class.klass.get_property = list_get_property;
}

static void
list_implementation_init (AZListImplementation *impl)
{
	AZCollectionImplementation *collection = (AZCollectionImplementation *) impl;
	collection->get_iterator = list_get_iterator;
	collection->iterator_next = list_iterator_next;
	collection->get_element = list_get_element;
}

static unsigned int
list_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	*prop_impl = ( AZImplementation*) az_type_get_class (AZ_TYPE_UINT32);
	prop_val->uint32_v = az_collection_get_size ((AZCollectionImplementation *) impl, inst);
	return 1;
}

static unsigned int
list_get_iterator (const AZCollectionImplementation *collection_impl, void *collection_inst, AZPackedValue *iterator)
{
	az_packed_value_set_unsigned_int (iterator, AZ_TYPE_UINT32, 0);
	return 1;
}

static unsigned int
list_iterator_next (const AZCollectionImplementation *collection_impl, void *collection_inst, AZPackedValue *iterator)
{
	if (iterator->v.uint32_v >= az_collection_get_size (collection_impl, collection_inst)) return 0;
	iterator->v.uint32_v += 1;
	return 1;
}

static const AZImplementation *
list_get_element (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZPackedValue *iterator, AZValue *val, unsigned int size)
{
	return az_list_get_element ((AZListImplementation *) collection_impl, collection_inst, iterator->v.uint32_v, val,  size);
}
