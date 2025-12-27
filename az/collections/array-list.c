#define __AZ_ARRAY_LIST_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/boxed-value.h>
#include <az/packed-value.h>
#include <az/private.h>
#include <az/extend.h>

#include "array-list.h"

static void array_list_class_init (AZArrayListClass *klass);
static void array_list_init (AZArrayListClass *klass, AZArrayList *alist);
static void array_list_finalize (AZArrayListClass *klass, AZArrayList *alist);

/* AZCollection implementation */
static unsigned int array_list_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int array_list_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int array_list_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *array_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

AZArrayListClass *AZArrayListKlass = NULL;

unsigned int
az_array_list_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		AZArrayListKlass = (AZArrayListClass *) az_register_type (&type, (const unsigned char *) "AZArrayList", AZ_TYPE_BLOCK, sizeof (AZArrayListClass), sizeof (AZArrayList), 0,
			(void (*) (AZClass *)) array_list_init,
			(void (*) (const AZImplementation *, void *)) array_list_init,
			(void (*) (const AZImplementation *, void *)) array_list_finalize);

		AZArrayListKlass->list_implementation.collection_impl.get_element_type = array_list_get_element_type;
		AZArrayListKlass->list_implementation.collection_impl.get_size = array_list_get_size;
		AZArrayListKlass->list_implementation.collection_impl.contains = array_list_contains;
		AZArrayListKlass->list_implementation.get_element = array_list_get_element;
		AZArrayListKlass->default_size = 4;
	}
	return type;
}

static void
array_list_init (AZArrayListClass *klass, AZArrayList *alist)
{
	alist->type = AZ_TYPE_ANY;
	alist->size = 0;
	alist->length = 0;
	alist->val_size = 8;
	alist->values = NULL;
}

static void
array_list_finalize (AZArrayListClass *klass, AZArrayList *alist)
{
	unsigned int i;
	for (unsigned int i = 0; i < alist->length; i++) {
        az_value_clear (alist->values[i].impl, (AZValue *) alist->values[i].val);
	}
	if (alist->values) free (alist->values);
}

static unsigned int
array_list_get_element_type (const AZCollectionImplementation *coll_impl, void *collection_inst)
{
	AZArrayList *alist = (AZArrayList *) collection_inst;
	return alist->type;
}

static unsigned int
array_list_get_size (const AZCollectionImplementation *coll_impl, void *collection_inst)
{
	AZArrayList *alist = (AZArrayList *) collection_inst;
	return alist->length;
}

static unsigned int
array_list_contains (const AZCollectionImplementation *coll_impl, void *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayList *alist = (AZArrayList *) collection_inst;
	for (unsigned int i = 0; i < alist->length; i++) {
		const AZImplementation *el_impl = alist->values[i].impl;
		const AZValue *el_val = (const AZValue *) alist->values[i].val;
		if (el_impl == &AZBoxedValueKlass.klass.impl) {
			AZBoxedValue *boxed = (AZBoxedValue *) el_val->block;
			el_impl = &boxed->klass->impl;
			el_val = &boxed->val;
		}
		if (el_impl != impl) continue;
		if (AZ_IMPL_IS_BLOCK(el_impl)) {
			if (el_val->block == inst) return 1;
		} else {
			AZClass *klass = AZ_CLASS_FROM_IMPL(el_impl);
			if (klass->instance_size && !memcmp(el_val, inst, klass->instance_size)) return 1;
		}
	}
	return 0;
}

static const AZImplementation *
array_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayList *alist = (AZArrayList *) list_inst;
	if (alist->values[idx].impl) {
		return az_value_copy_autobox (alist->values[idx].impl, val, (const AZValue *) alist->values[idx].val, size);
	}
	return alist->values[idx].impl;
}

AZArrayList *
az_array_list_new(unsigned int el_type, unsigned int val_size)
{
	AZArrayList *alist = az_instance_new(AZ_TYPE_ARRAY_LIST);
	alist->type = el_type;
	alist->val_size = val_size;
	return alist;
}

unsigned int
az_array_list_append(AZArrayList *alist, const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail(!impl || az_type_is_a(AZ_IMPL_TYPE(impl), alist->type), 0);
	if (alist->length >= alist->size) {
		alist->size = (alist->size) ? alist->size << 1 : 8;
		alist->values = (AZArrayListEntry *) realloc(alist->values, alist->size * (sizeof(AZArrayListEntry) + alist->val_size - 8));
	}
	alist->values[alist->length].impl = impl;
	az_value_set_from_inst_autobox(impl, (AZValue *) alist->values[alist->length].val, inst, alist->val_size);
	alist->length += 1;
	return 1;
}
