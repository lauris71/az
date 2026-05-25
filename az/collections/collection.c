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

#include "collection.h"

/* AZInterface implementation */
static void collection_class_init (AZCollectionClass* klass);

static unsigned int collection_call_contains (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
static unsigned int collection_call_get_iterator (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

enum {
	FUNC_CONTAINS,
	FUNC_GET_ITERATOR,
	PROP_SIZE,
	NUM_PROPERTIES
};

static unsigned int collection_type = 0;
static AZCollectionClass *collection_class;

unsigned int
az_collection_get_type (void)
{
	if (!collection_type) {
		collection_class = (AZCollectionClass *) az_register_interface_type (&collection_type, (const unsigned char *) "AZCollection", AZ_TYPE_INTERFACE,
			sizeof(AZCollectionClass), sizeof(AZCollectionImplementation), sizeof(AZCollection), AZ_FLAG_ABSTRACT,
			(void (*) (AZClass *)) collection_class_init,
			NULL,
			NULL, NULL);
	}
	return collection_type;
}

static void
collection_class_init (AZCollectionClass *klass)
{
	az_class_set_num_properties ((AZClass *) klass, NUM_PROPERTIES);
	az_class_define_method_va ((AZClass *) klass, FUNC_CONTAINS, (const unsigned char *) "contains", collection_call_contains, AZ_TYPE_BOOLEAN, 1, AZ_TYPE_ANY);
	az_class_define_method_va ((AZClass *) klass, FUNC_GET_ITERATOR, (const unsigned char *) "getIterator", collection_call_get_iterator, AZ_TYPE_ANY, 0);
	az_class_define_property ((AZClass *) klass, PROP_SIZE, (const unsigned char *) "size", AZ_TYPE_UINT32, 0, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET(AZCollection,size), NULL, NULL);
}

static unsigned int
collection_call_contains (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	/* fixme: Automatic deboxing at function call? */
	const AZCollectionImplementation *impl;
	void *inst;
	if (AZ_IMPL_TYPE(arg_impls[0]) == AZ_TYPE_BOXED_INTERFACE) {
		impl = (const AZCollectionImplementation *) ((AZBoxedInterface *) arg_vals[0]->reference)->impl;
		inst = ((AZBoxedInterface *) arg_vals[0]->reference)->inst;
	} else {
		impl = (const AZCollectionImplementation *) arg_impls[0];
		inst = az_value_get_inst(arg_impls[0], arg_vals[0]);
	}
	unsigned int contains = az_collection_contains (impl, inst, arg_impls[1], az_value_get_inst(arg_impls[1], arg_vals[1]));
	*ret_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOOLEAN);
	ret_val->value.boolean_v = contains;
	return 1;
}

static unsigned int
collection_call_get_iterator (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	const AZCollectionImplementation *impl;
	void *inst;
	if (AZ_IMPL_TYPE(arg_impls[0]) == AZ_TYPE_BOXED_INTERFACE) {
		impl = (const AZCollectionImplementation *) ((AZBoxedInterface *) arg_vals[0]->reference)->impl;
		inst = ((AZBoxedInterface *) arg_vals[0]->reference)->inst;
	} else {
		impl = (const AZCollectionImplementation *) arg_impls[0];
		inst = az_value_get_inst(arg_impls[0], arg_vals[0]);
	}
	*ret_impl = az_collection_get_iterator(impl, inst, &ret_val->value);
	return 1;
}

