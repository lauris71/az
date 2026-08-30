#define __AZ_WEAK_OBJECT_LIST_CPP__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdlib.h>
#include <string.h>

#include <az/extend.h>

#include "weak-object-list.h"

static void weak_object_list_class_init (AZWeakObjectListClass *klass);
static void weak_object_list_init (AZWeakObjectListClass *klass, AZWeakObjectList *objl);
static void weak_object_list_finalize (AZWeakObjectListClass *klass, AZWeakObjectList *objl);

/* AZCollection implementation */
static unsigned int weak_object_list_get_element_type (const AZCollectionImplementation *impl, AZCollection *collection_instance);
static unsigned int weak_object_list_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *weak_object_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

/* Method implementations */
static unsigned int weak_object_list_call_Append (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

enum {
	/* Functions */
	FUNC_APPEND,
	NUM_FUNCTIONS,
	/* Values */
	PROP_LENGTH = NUM_FUNCTIONS,
	NUM_PROPERTIES
};

static unsigned int weak_object_list_type = 0;

unsigned int
az_weak_object_list_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(weak_object_list_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!weak_object_list_type) {
		az_register_type (&weak_object_list_type, (const unsigned char *) "AZWeakObjectList", AZ_TYPE_BLOCK, sizeof (AZWeakObjectListClass), sizeof (AZWeakObjectList), AZ_FLAG_ZERO_MEMORY | AZ_FLAG_FINAL,
			1, NUM_PROPERTIES,
			(void (*) (AZClass *)) weak_object_list_class_init,
			(void (*) (const AZImplementation *, void *)) weak_object_list_init,
			(void (*) (const AZImplementation *, void *)) weak_object_list_finalize);
	}
	t = weak_object_list_type;
	AZ_TYPES_UNLOCK();
	return t;
}

static void
weak_object_list_class_init (AZWeakObjectListClass *klass)
{
	/* Interfaces */
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET(AZWeakObjectListClass, list_implementation), ARIKKEI_OFFSET(AZWeakObjectList, list));
	az_class_define_method_va ((AZClass *) klass, FUNC_APPEND, (const unsigned char *) "append", weak_object_list_call_Append, AZ_TYPE_NONE, 1, AZ_TYPE_ACTIVE_OBJECT );
	az_class_define_property ((AZClass *) klass, PROP_LENGTH, (const unsigned char *) "length", AZ_TYPE_UINT32, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZWeakObjectList,list.collection.size), NULL, NULL);
	/* Array implementation */
	klass->list_implementation.collection_impl.get_element_type = weak_object_list_get_element_type;
	klass->list_implementation.collection_impl.contains = weak_object_list_contains;
	klass->list_implementation.get_element = weak_object_list_get_element;
}

static void
weak_object_list_init (AZWeakObjectListClass *klass, AZWeakObjectList *objl)
{
	objl->allocated_size = 16;
	objl->objects = (AZActiveObject **) malloc (objl->allocated_size * sizeof (AZActiveObject *));
}

static void
weak_object_list_finalize (AZWeakObjectListClass *klass, AZWeakObjectList *objl)
{
	for (unsigned int i = 0; i < objl->list.collection.size; i++) {
		az_active_object_remove_listener_by_data (objl->objects[i], objl);
	}
	free (objl->objects);
}

static unsigned int
weak_object_list_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst)
{
	AZWeakObjectList *objl = (AZWeakObjectList *) ARIKKEI_BASE_ADDRESS(AZWeakObjectList,list,collection_inst);
	return objl->type;
}

static unsigned int
weak_object_list_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZWeakObjectList *objl = (AZWeakObjectList *) ARIKKEI_BASE_ADDRESS(AZWeakObjectList,list,collection_inst);
	unsigned int i;
	for (i = 0; i < objl->list.collection.size; i++) {
		if (objl->objects[i] == (AZActiveObject *) inst) return 1;
	}
	return 0;
}

static const AZImplementation *
weak_object_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	const AZImplementation *impl;
	AZWeakObjectList *objl = (AZWeakObjectList *) ARIKKEI_BASE_ADDRESS(AZWeakObjectList,list,list_inst);
	arikkei_return_val_if_fail (idx < objl->list.collection.size, 0);
	az_value_set_object (&impl, val, (AZObject *) objl->objects[idx]);
	return impl;
}

static unsigned int
weak_object_list_call_Append (const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	AZWeakObjectList *objl = (AZWeakObjectList *) arg_vals[0];
	AZActiveObject *obj = (AZActiveObject *) arg_vals[1]->reference;
	az_weak_object_list_append_object (objl, obj);
	return 1;
}

void
az_weak_object_list_setup (AZWeakObjectList *objl, unsigned int type)
{
	arikkei_return_if_fail (objl != NULL);
	arikkei_return_if_fail (AZ_TYPE_IS_INTERFACE (type) || az_type_is_a (type, AZ_TYPE_ACTIVE_OBJECT));
	az_instance_init_by_type (objl, AZ_TYPE_WEAK_OBJECT_LIST);
	objl->type = type;
}

AZWeakObjectList *
az_weak_object_list_new (unsigned int type)
{
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE (type) || az_type_is_a (type, AZ_TYPE_ACTIVE_OBJECT), NULL);
	AZWeakObjectList *objl = (AZWeakObjectList *) malloc (sizeof (AZWeakObjectList));
	az_weak_object_list_setup (objl, type);
	return objl;
}

void
az_weak_object_list_delete (AZWeakObjectList *objl)
{
	arikkei_return_if_fail (objl != NULL);
	az_weak_object_list_release (objl);
	free (objl);
}

static void weak_object_list_object_dispose (AZActiveObject *object, void *data);
static unsigned int weak_object_list_remove_object_internal (AZWeakObjectList *objl, AZActiveObject *object);

static AZObjectEventVector weak_object_list_event_vector = {
	weak_object_list_object_dispose
};

void
az_weak_object_list_append_object (AZWeakObjectList *objl, AZActiveObject *obj)
{
	arikkei_return_if_fail (objl != NULL);
	arikkei_return_if_fail (AZ_IS_ACTIVE_OBJECT (obj));
	arikkei_return_if_fail ((AZ_TYPE_IS_INTERFACE(objl->type) && az_object_implements((AZObject *) obj, objl->type)) || az_object_is_a((AZObject *) obj, objl->type));
	if (objl->list.collection.size >= objl->allocated_size) {
		objl->allocated_size = objl->allocated_size << 1;
		objl->objects = (AZActiveObject **) realloc (objl->objects, objl->allocated_size * sizeof (AZActiveObject *));
	}
	objl->objects[objl->list.collection.size++] = obj;
	az_active_object_add_listener (obj, &weak_object_list_event_vector, sizeof (AZObjectEventVector), objl);
}

void
az_weak_object_list_insert_object (AZWeakObjectList *objl, AZActiveObject *obj, unsigned int pos)
{
	arikkei_return_if_fail (objl != NULL);
	arikkei_return_if_fail (AZ_IS_ACTIVE_OBJECT (obj));
	arikkei_return_if_fail ((AZ_TYPE_IS_INTERFACE(objl->type) && az_object_implements((AZObject *) obj, objl->type)) || az_object_is_a((AZObject *) obj, objl->type));
	arikkei_return_if_fail (pos <= objl->list.collection.size);
	if (objl->list.collection.size >= objl->allocated_size) {
		objl->allocated_size = objl->allocated_size << 1;
		objl->objects = (AZActiveObject **) realloc (objl->objects, objl->allocated_size * sizeof (AZActiveObject *));
	}
	if (pos < objl->list.collection.size) memmove(&objl->objects[pos + 1], &objl->objects[pos], (objl->list.collection.size - pos) * sizeof (AZActiveObject *));
	objl->list.collection.size += 1;
	objl->objects[pos] = obj;
	az_active_object_add_listener (obj, &weak_object_list_event_vector, sizeof (AZObjectEventVector), objl);
}

void
az_weak_object_list_remove_object (AZWeakObjectList *objl, AZActiveObject *obj)
{
	arikkei_return_if_fail (objl != NULL);
	arikkei_return_if_fail (AZ_IS_ACTIVE_OBJECT (obj));
	arikkei_return_if_fail ((AZ_TYPE_IS_INTERFACE(objl->type) && az_object_implements((AZObject *) obj, objl->type)) || az_object_is_a((AZObject *) obj, objl->type));
	if (weak_object_list_remove_object_internal (objl, obj)) {
		az_active_object_remove_listener_by_data (obj, objl);
	}
}

void
az_weak_object_list_remove_object_by_index (AZWeakObjectList *objl, unsigned int idx)
{
	AZActiveObject *obj;
	arikkei_return_if_fail (objl != NULL);
	arikkei_return_if_fail (idx < objl->list.collection.size);
	obj = objl->objects[idx];
	if (weak_object_list_remove_object_internal (objl, obj)) {
		az_active_object_remove_listener_by_data (obj, objl);
	}
}

void
az_weak_object_list_clear (AZWeakObjectList *objl)
{
	unsigned int i;
	arikkei_return_if_fail (objl != NULL);
	for (i = 0; i < objl->list.collection.size; i++) {
		az_active_object_remove_listener_by_data (objl->objects[i], objl);
	}
	objl->list.collection.size = 0;
}

unsigned int
az_weak_object_list_contains (AZWeakObjectList *objl, AZActiveObject *obj)
{
	arikkei_return_val_if_fail (objl != NULL, 0);
	arikkei_return_val_if_fail (AZ_IS_ACTIVE_OBJECT (obj), 0);
	arikkei_return_val_if_fail ((AZ_TYPE_IS_INTERFACE(objl->type) && az_object_implements((AZObject *) obj, objl->type)) || az_object_is_a((AZObject *) obj, objl->type), 0);
	for (unsigned int i = 0; i < objl->list.collection.size; i++) {
		if (objl->objects[i] == obj) return 1;
	}
	return 0;
}

static void
weak_object_list_object_dispose (AZActiveObject *obj, void *data)
{
	AZWeakObjectList *objl = (AZWeakObjectList *) data;
	weak_object_list_remove_object_internal (objl, obj);
}

static unsigned int
weak_object_list_remove_object_internal (AZWeakObjectList *objl, AZActiveObject *obj)
{
	for (unsigned int i = 0; i < objl->list.collection.size; i++) {
		if (objl->objects[i] == obj) {
			if (i < (objl->list.collection.size - 1)) {
				memmove(&objl->objects[i], &objl->objects[i + 1], (objl->list.collection.size - 1 - i) * sizeof (AZActiveObject *));
			}
			objl->list.collection.size -= 1;
			return 1;
		}
	}
	return 0;
}
