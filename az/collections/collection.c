#define __AZ_COLLECTION_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#include <stdlib.h>

#include <az/boxed-interface.h>
#include <az/private.h>
#include <az/extend.h>
#include <az/context.h>

#include "collection.h"

/* AZInterface implementation */
static void collection_class_init (AZCollectionClass* klass);
static unsigned int collection_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);

enum {
	FUNC_CONTAINS,
	FUNC_GET_ITERATOR,
	PROP_SIZE,
	COLL_NUM_PROPERTIES
};

static unsigned int collection_type = 0;
static AZCollectionClass *collection_class;

unsigned int
az_collection_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(collection_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!collection_type) {
		collection_class = (AZCollectionClass *) az_register_interface_type (&collection_type, (const unsigned char *) "AZCollection", AZ_TYPE_INTERFACE,
			sizeof(AZCollectionClass), sizeof(AZCollectionImplementation), 0, AZ_FLAG_ABSTRACT,
			0, COLL_NUM_PROPERTIES,
			(void (*) (AZClass *)) collection_class_init,
			NULL,
			NULL, NULL);
	}
	t = collection_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static int
collection_call_contains (unsigned int n_args, AZStoreElement *elems, AZValue *vals, const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx)
{
	const AZImplementation *impl = elems[0].impl;
	void *inst = az_value_store_instance_at(elems, 0, vals);
	if (impl == &AZBoxedInterfaceKlass.klass.impl) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	unsigned int contains = az_collection_contains ((AZCollectionImplementation *) impl, (AZCollection *) inst, elems[1].impl, az_value_store_instance_at(elems, 1, vals));
	*ret_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOOLEAN);
	ret_val->boolean_v = contains;
	return AZ_OK;
}

static int
collection_call_get_iterator (unsigned int n_args, AZStoreElement *elems, AZValue *vals, const AZImplementation **ret_impl, AZValue *ret_val, unsigned int ret_size, AZContext *ctx)
{
	const AZImplementation *impl = elems[0].impl;
	void *inst = az_value_store_instance_at(elems, 0, vals);
	if (impl == &AZBoxedInterfaceKlass.klass.impl) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	*ret_impl = az_collection_get_iterator((AZCollectionImplementation *) impl, (AZCollection *) inst, ret_val);
	return 1;
}

static void
collection_class_init (AZCollectionClass *klass)
{
	az_class_define_method_va ((AZClass *) klass, FUNC_CONTAINS, (const uint8_t *) "contains", collection_call_contains, AZ_TYPE_BOOLEAN, 1, AZ_TYPE_ANY);
	az_class_define_method_va ((AZClass *) klass, FUNC_GET_ITERATOR, (const uint8_t *) "getIterator", collection_call_get_iterator, AZ_TYPE_ANY, 0);
	az_class_define_property ((AZClass *) klass, PROP_SIZE, (const uint8_t *) "size", AZ_TYPE_UINT64, 0, AZ_FIELD_INSTANCE, AZ_FIELD_READ_METHOD, 0, 0, NULL, NULL);
	klass->iface_class.klass.get_property = collection_get_property;
}

static unsigned int
collection_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	if (idx == PROP_SIZE) {
		*prop_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT64);
		prop_val->uint64_v = az_collection_get_size ((const AZCollectionImplementation *) impl, inst);
		return 1;
	}
	return 0;
}
