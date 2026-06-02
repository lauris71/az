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
static unsigned int array_list_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *array_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

AZArrayListClass *AZArrayListKlass = NULL;

unsigned int
az_array_list_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		AZArrayListKlass = (AZArrayListClass *) az_register_type (&type, (const unsigned char *) "AZArrayList", AZ_TYPE_BLOCK, sizeof (AZArrayListClass), sizeof (AZArrayList), 0,
			1, 0,
			(void (*) (AZClass *)) array_list_class_init,
			(void (*) (const AZImplementation *, void *)) array_list_init,
			(void (*) (const AZImplementation *, void *)) array_list_finalize);

		AZArrayListKlass->list_impl.collection_impl.contains = array_list_contains;
		AZArrayListKlass->list_impl.get_element = array_list_get_element;
		AZArrayListKlass->default_size = 4;
	}
	return type;
}

static void
array_list_class_init (AZArrayListClass *klass)
{
	az_class_declare_interface((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET(AZArrayListClass, list_impl), 0);
}

static void
array_list_init (AZArrayListClass *klass, AZArrayList *alist)
{
	alist->element_type = AZ_TYPE_ANY;
	alist->list.collection.size = 0;
	alist->val_size = 8;
	alist->data_size = 0;
	alist->data = NULL;
}

static void
array_list_finalize (AZArrayListClass *klass, AZArrayList *alist)
{
	for (unsigned int i = 0; i < alist->list.collection.size; i++) {
		AZArrayListEntry *entry = az_array_list_get_entry(alist, i);
        az_value_clear (entry->impl, (AZValue *) entry->val);
	}
	if (alist->data) free (alist->data);
}

static unsigned int
array_list_contains (const AZCollectionImplementation *coll_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZArrayList *alist = (AZArrayList *) collection_inst;
	for (unsigned int i = 0; i < alist->list.collection.size; i++) {
		AZArrayListEntry *entry = az_array_list_get_entry(alist, i);
		if (az_value_equals_instance_autobox(entry->impl, (const AZValue *) entry->val, impl, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
array_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZArrayList *alist = (AZArrayList *) list_inst;
	AZArrayListEntry *entry = az_array_list_get_entry(alist, idx);
	if (entry->impl) {
		return az_value_copy_autobox (entry->impl, val, (const AZValue *) entry->val, size);
	}
	return entry->impl;
}

AZArrayList *
az_array_list_new(unsigned int el_type, unsigned int val_size)
{
	AZArrayList *alist = az_instance_new(AZ_TYPE_ARRAY_LIST);
	alist->element_type = el_type;
	alist->val_size = (val_size + 0x7) & 0xfffffff8;
	return alist;
}

void
az_array_list_clear(AZArrayList *alist)
{
	for (unsigned int i = 0; i < alist->list.collection.size; i++) {
		AZArrayListEntry *entry = az_array_list_get_entry(alist, i);
        az_value_clear (entry->impl, (AZValue *) entry->val);
	}
	alist->list.collection.size = 0;
}

unsigned int
az_array_list_set_element (AZArrayList *alist, unsigned int idx, const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail(!impl || az_type_is_a(AZ_IMPL_TYPE(impl), alist->element_type), 0);
	arikkei_return_val_if_fail(idx < alist->list.collection.size, 0);
	AZArrayListEntry *entry = az_array_list_get_entry(alist, idx);
	az_value_clear(entry->impl, (AZValue *) entry->val);
	entry->impl = az_value_set_from_inst_autobox(impl, (AZValue *) entry->val, alist->val_size, inst);
	return 1;
}

unsigned int
az_array_list_append(AZArrayList *alist, const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail(!impl || az_type_is_a(AZ_IMPL_TYPE(impl), alist->element_type), 0);
	if (alist->list.collection.size >= alist->data_size) {
		alist->data_size = (alist->data_size) ? alist->data_size << 1 : 8;
		alist->data = (AZArrayListEntry *) realloc(alist->data, alist->data_size * az_array_list_entry_size(alist));
	}
	AZArrayListEntry *entry = az_array_list_get_entry(alist, alist->list.collection.size);
	entry->impl = az_value_set_from_inst_autobox(impl, (AZValue *) entry->val, alist->val_size, inst);
	alist->list.collection.size += 1;
	return 1;
}

unsigned int
az_array_list_insert(AZArrayList *alist, unsigned int idx, const AZImplementation *impl, void *inst)
{
	arikkei_return_val_if_fail(!impl || az_type_is_a(AZ_IMPL_TYPE(impl), alist->element_type), 0);
	arikkei_return_val_if_fail(idx <= alist->list.collection.size, 0);
	if (alist->list.collection.size >= alist->data_size) {
		alist->data_size = (alist->data_size) ? alist->data_size << 1 : 8;
		alist->data = (AZArrayListEntry *) realloc(alist->data, alist->data_size * az_array_list_entry_size(alist));
	}
	AZArrayListEntry *entry = az_array_list_get_entry(alist, idx);
	memmove((char *) entry + az_array_list_entry_size(alist), (char *) entry, (alist->list.collection.size - idx) * az_array_list_entry_size(alist));
	entry->impl = az_value_set_from_inst_autobox(impl, (AZValue *) entry->val, alist->val_size, inst);
	alist->list.collection.size += 1;
	return 1;
}

unsigned int
az_array_list_remove(AZArrayList *alist, unsigned int idx)
{
	arikkei_return_val_if_fail(idx < alist->list.collection.size, 0);
	AZArrayListEntry *entry = az_array_list_get_entry(alist, idx);
	az_value_clear(entry->impl, (AZValue *) entry->val);
	memmove((char *) entry, (char *) entry + az_array_list_entry_size(alist), (alist->list.collection.size - idx - 1) * az_array_list_entry_size(alist));
	alist->list.collection.size -= 1;
	return 1;
}
