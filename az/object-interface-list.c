#define __AZ_OBJECT_INTERFACE_LIST_CPP__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/class.h>
#include <az/object-interface-list.h>

static void object_interface_list_class_init (AZObjectInterfaceListClass *klass);
static void object_interface_list_init (AZObjectInterfaceListClass *klass, AZObjectInterfaceList *objifl);
static void object_interface_list_finalize (AZObjectInterfaceListClass *klass, AZObjectInterfaceList *objifl);

/* AZCollection implementation */
static unsigned int object_interface_list_get_element_type (const AZCollectionImplementation *impl, void *collection_instance);
static unsigned int object_interface_list_get_size (const AZCollectionImplementation *impl, void *collection_instance);
static unsigned int object_interface_list_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *object_interface_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *val);

/* Method implementations */
static unsigned int object_interface_list_call_lookUp (AZPackedValue *thisval, AZPackedValue *retval, AZPackedValue *args);

static unsigned int object_interface_list_type = 0;

unsigned int
az_object_interface_list_get_type (void)
{
	if (!object_interface_list_type) {
		az_register_type (&object_interface_list_type, (const unsigned char *) "AZObjectInterfaceList", AZ_TYPE_STRUCT, sizeof (AZObjectInterfaceListClass), sizeof (AZObjectInterfaceList), AZ_FLAG_ZERO_MEMORY,
			(void (*) (AZClass *)) object_interface_list_class_init,
			(void (*) (const AZImplementation *, void *)) object_interface_list_init,
			(void (*) (const AZImplementation *, void *)) object_interface_list_finalize);
	}
	return object_interface_list_type;
}

static void
object_interface_list_class_init (AZObjectInterfaceListClass *klass)
{
	/* Interfaces */
	az_class_set_num_interfaces ((AZClass *) klass, 1);
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET (AZObjectInterfaceListClass, list_implementation), 0);
	/* Array implementation */
	klass->list_implementation.collection_impl.get_element_type = object_interface_list_get_element_type;
	klass->list_implementation.collection_impl.get_size = object_interface_list_get_size;
	klass->list_implementation.collection_impl.contains = object_interface_list_contains;
	klass->list_implementation.get_element = object_interface_list_get_element;
}

static void
object_interface_list_init (AZObjectInterfaceListClass *klass, AZObjectInterfaceList *objifl)
{
	objifl->size = 16;
	objifl->elements = (AZObjectInterfaceListElement *) malloc (objifl->size * sizeof (AZObjectInterfaceListElement));
}

static void
object_interface_list_finalize (AZObjectInterfaceListClass *klass, AZObjectInterfaceList *objifl)
{
	while (objifl->length) {
		az_object_unref (objifl->elements[--objifl->length].obj);
	}
	free (objifl->elements);
}

static unsigned int
object_interface_list_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZObjectInterfaceList *objifl = (AZObjectInterfaceList *) collection_inst;
	return objifl->interface_type;
}

static unsigned int
object_interface_list_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZObjectInterfaceList *objifl = (AZObjectInterfaceList *) collection_inst;
	return objifl->length;
}

static unsigned int
object_interface_list_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZObjectInterfaceList *objifl = ( AZObjectInterfaceList *) collection_inst;
	unsigned int i;
	for (i = 0; i < objifl->length; i++) {
		if ((objifl->elements[i].impl == impl) && (objifl->elements[i].inst == inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
object_interface_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *val)
{
	AZObjectInterfaceList *objifl = (AZObjectInterfaceList *) list_inst;
	arikkei_return_val_if_fail (idx < objifl->length, 0);
	val->value.block = objifl->elements[idx].inst;
	return objifl->elements[idx].impl;
}

void
az_object_interface_list_setup (AZObjectInterfaceList *objifl, unsigned int object_type, unsigned int interface_type)
{
	arikkei_return_if_fail (objifl != NULL);
	arikkei_return_if_fail (az_type_is_a (object_type, AZ_TYPE_OBJECT));
	arikkei_return_if_fail (az_type_is_a (interface_type, AZ_TYPE_INTERFACE));
	az_instance_init (objifl, AZ_TYPE_OBJECT_INTERFACE_LIST);
	objifl->object_type = object_type;
	objifl->interface_type = interface_type;
}

void
az_object_interface_list_release (AZObjectInterfaceList *objifl)
{
	arikkei_return_if_fail (objifl != NULL);
	az_instance_finalize (objifl, AZ_TYPE_OBJECT_INTERFACE_LIST);
}

AZObjectInterfaceList *
az_object_interface_list_new (unsigned int object_type, unsigned int interface_type)
{
	arikkei_return_val_if_fail (az_type_is_a (object_type, AZ_TYPE_OBJECT), NULL);
	arikkei_return_val_if_fail (az_type_is_a (interface_type, AZ_TYPE_INTERFACE), NULL);
	AZObjectInterfaceList *objifl = (AZObjectInterfaceList *) malloc (sizeof (AZObjectInterfaceList));
	az_object_interface_list_setup (objifl, object_type, interface_type);
	return objifl;
}

void
az_object_interface_list_delete (AZObjectInterfaceList *objifl)
{
	arikkei_return_if_fail (objifl != NULL);
	az_object_interface_list_release (objifl);
	free (objifl);
}

void
az_object_interface_list_append_object (AZObjectInterfaceList *objifl, AZObject *obj)
{
	arikkei_return_if_fail (objifl != NULL);
	arikkei_return_if_fail (obj != NULL);
	arikkei_return_if_fail (az_object_is_a (obj, objifl->object_type));
	arikkei_return_if_fail (az_object_implements (obj, objifl->interface_type));
	if (objifl->length >= objifl->size) {
		objifl->size = objifl->size << 1;
		objifl->elements = (AZObjectInterfaceListElement *) realloc (objifl->elements, objifl->size * sizeof (AZObjectInterfaceListElement));
	}
	objifl->elements[objifl->length].impl = az_object_get_interface (obj, objifl->interface_type, &objifl->elements[objifl->length].inst);
	objifl->elements[objifl->length].obj = obj;
	az_object_ref (obj);
	objifl->length += 1;
}

void
az_object_interface_list_remove_object (AZObjectInterfaceList *objifl, AZObject *obj)
{
	unsigned int i;
	arikkei_return_if_fail (objifl != NULL);
	arikkei_return_if_fail (obj != NULL);
	arikkei_return_if_fail (az_object_is_a (obj, objifl->object_type));
	arikkei_return_if_fail (az_object_implements (obj, objifl->interface_type));
	for (i = 0; i < objifl->length; i++) {
		if (objifl->elements[i].obj == obj) {
			az_object_interface_list_remove_object_by_index (objifl, i);
			return;
		}
	}
	fprintf (stderr, "az_object_interface_list_remove_object: object not in list\n");
}

void
az_object_interface_list_remove_object_by_index (AZObjectInterfaceList *objifl, unsigned int idx)
{
	AZObject *obj;
	arikkei_return_if_fail (objifl != NULL);
	arikkei_return_if_fail (idx < objifl->length);
	obj = objifl->elements[idx].obj;
	objifl->length -= 1;
	if (idx < objifl->length) {
		memcpy (&objifl->elements[idx], &objifl->elements[idx + 1], (objifl->length - idx) * sizeof (AZObjectInterfaceListElement));
	}
	az_object_unref (obj);
}

void
az_object_interface_list_clear (AZObjectInterfaceList *objifl)
{
	arikkei_return_if_fail (objifl != NULL);
	while (objifl->length) {
		az_object_unref (objifl->elements[--objifl->length].obj);
	}
}

unsigned int
az_object_interface_list_contains (AZObjectInterfaceList *objifl, AZObject *obj)
{
	arikkei_return_val_if_fail (objifl != NULL, 0);
	arikkei_return_val_if_fail (obj != NULL, 0);
	arikkei_return_val_if_fail (az_object_is_a (obj, objifl->object_type), 0);
	arikkei_return_val_if_fail (az_object_implements (obj, objifl->interface_type), 0);
	for (unsigned int i = 0; i < objifl->length; i++) {
		if (objifl->elements[i].obj == obj) return 1;
	}
	return 0;
}

