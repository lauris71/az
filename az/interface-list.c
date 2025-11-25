#define __AZ_INTERFACE_LIST_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <az/value.h>
#include <az/interface-list.h>
#include <az/extend.h>

static void interface_list_class_init (AZInterfaceListClass* klass);
static void interface_list_init (AZInterfaceListClass* klass, AZInterfaceList* ifl);
static void interface_list_finalize (AZInterfaceListClass* klass, AZInterfaceList* ifl);

/* AZCollection implementation */
static unsigned int interface_list_get_element_type (const AZCollectionImplementation* impl, void* collection_instance);
static unsigned int interface_list_get_size (const AZCollectionImplementation* impl, void* collection_instance);
static unsigned int interface_list_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation* interface_list_get_element (const AZListImplementation* list_impl, void* list_inst, unsigned int idx, AZValue64 *val);

/* Method implementations */
static unsigned int interface_list_call_lookUp (AZPackedValue* thisval, AZPackedValue* retval, AZPackedValue* args);

static unsigned int interface_list_type = 0;

unsigned int
az_interface_list_get_type (void)
{
	if (!interface_list_type) {
		az_register_type (&interface_list_type, (const unsigned char*) "AZInterfaceList", AZ_TYPE_STRUCT, sizeof (AZInterfaceListClass), sizeof (AZInterfaceList), AZ_FLAG_ZERO_MEMORY,
			(void (*) (AZClass*)) interface_list_class_init,
			(void (*) (const AZImplementation*, void*)) interface_list_init,
			(void (*) (const AZImplementation*, void*)) interface_list_finalize);
	}
	return interface_list_type;
}

static void
interface_list_class_init (AZInterfaceListClass* klass)
{
	/* Interfaces */
	az_class_set_num_interfaces (( AZClass*) klass, 1);
	az_class_declare_interface (( AZClass*) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET (AZInterfaceListClass, list_impl), 0);
	/* Array implementation */
	klass->list_impl.collection_impl.get_element_type = interface_list_get_element_type;
	klass->list_impl.collection_impl.get_size = interface_list_get_size;
	klass->list_impl.collection_impl.contains = interface_list_contains;
	klass->list_impl.get_element = interface_list_get_element;
}

static void
interface_list_init (AZInterfaceListClass* klass, AZInterfaceList* ifl)
{
	ifl->size = 16;
	ifl->elements = (AZInterfaceValue *) malloc (ifl->size * sizeof (AZInterfaceValue));
}

static void
interface_list_finalize (AZInterfaceListClass* klass, AZInterfaceList* ifl)
{
	free (ifl->elements);
}

static unsigned int
interface_list_get_element_type (const AZCollectionImplementation* collection_impl, void* collection_inst)
{
	AZInterfaceList* ifl = ( AZInterfaceList*) collection_inst;
	return ifl->iface_type;
}

static unsigned int
interface_list_get_size (const AZCollectionImplementation* collection_impl, void* collection_inst)
{
	AZInterfaceList* ifl = ( AZInterfaceList*) collection_inst;
	return ifl->length;
}

static unsigned int
interface_list_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZInterfaceList *ifl = (AZInterfaceList *) collection_inst;
	unsigned int i;
	for (i = 0; i < ifl->length; i++) {
		if ((ifl->elements[i].impl == impl) && (ifl->elements[i].inst == inst)) return 1;
	}
	return 0;
}

static const AZImplementation*
interface_list_get_element (const AZListImplementation* list_impl, void* list_inst, unsigned int idx, AZValue64 *val)
{
	AZInterfaceList* ifl = ( AZInterfaceList*) list_inst;
	arikkei_return_val_if_fail (idx < ifl->length, 0);
	val->value.block = ifl->elements[idx].inst;
	return ifl->elements[idx].impl;
}

void
az_interface_list_setup (AZInterfaceList* ifl, unsigned int iface_type)
{
	arikkei_return_if_fail (ifl != NULL);
	arikkei_return_if_fail (az_type_is_a (iface_type, AZ_TYPE_INTERFACE));
	az_instance_init (ifl, AZ_TYPE_INTERFACE_LIST);
	ifl->iface_type = iface_type;
}

void
az_interface_list_release (AZInterfaceList* ifl)
{
	arikkei_return_if_fail (ifl != NULL);
	az_instance_finalize (ifl, AZ_TYPE_INTERFACE_LIST);
}

AZInterfaceList*
az_interface_list_new (unsigned int iface_type)
{
	arikkei_return_val_if_fail (az_type_is_a (iface_type, AZ_TYPE_INTERFACE), NULL);
	AZInterfaceList* ifl = ( AZInterfaceList*) malloc (sizeof (AZInterfaceList));
	az_interface_list_setup (ifl, iface_type);
	return ifl;
}

void
az_interface_list_delete (AZInterfaceList* ifl)
{
	arikkei_return_if_fail (ifl != NULL);
	az_interface_list_release (ifl);
	free (ifl);
}

void
az_interface_list_append (AZInterfaceList* ifl, AZImplementation* impl, void* inst)
{
	arikkei_return_if_fail (ifl != NULL);
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
	arikkei_return_if_fail (az_type_is_a (AZ_IMPL_TYPE(impl), ifl->iface_type));
	if (ifl->length >= ifl->size) {
		ifl->size = ifl->size << 1;
		ifl->elements = (AZInterfaceValue *) realloc (ifl->elements, ifl->size * sizeof (AZInterfaceValue));
	}
	ifl->elements[ifl->length].impl = impl;
	ifl->elements[ifl->length].inst = inst;
	ifl->length += 1;
}

void
az_interface_list_remove (AZInterfaceList* ifl, void* inst)
{
	unsigned int i;
	arikkei_return_if_fail (ifl != NULL);
	arikkei_return_if_fail (inst != NULL);
	for (i = 0; i < ifl->length; i++) {
		if (ifl->elements[i].inst == inst) {
			az_interface_list_remove_by_index (ifl, i);
			return;
		}
	}
	fprintf (stderr, "az_interface_list_remove: instance not in list\n");
}

void
az_interface_list_remove_by_index (AZInterfaceList* ifl, unsigned int idx)
{
	arikkei_return_if_fail (ifl != NULL);
	arikkei_return_if_fail (idx < ifl->length);
	ifl->length -= 1;
	if (idx < ifl->length) {
		memcpy (&ifl->elements[idx], &ifl->elements[idx + 1], (ifl->length - idx) * sizeof (AZInterfaceValue));
	}
}

void
az_interface_list_clear (AZInterfaceList* ifl)
{
	arikkei_return_if_fail (ifl != NULL);
	ifl->length = 0;
}

unsigned int
az_interface_list_contains (AZInterfaceList* ifl, void* inst)
{
	arikkei_return_val_if_fail (ifl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
	for (unsigned int i = 0; i < ifl->length; i++) {
		if (ifl->elements[i].inst == inst) return 1;
	}
	return 0;
}

