#ifndef __AZ_WEAK_REFERENCE_C__
#define __AZ_WEAK_REFERENCE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

#include "weak-reference.h"

static void weak_reference_finalize (AZWeakReferenceClass *klass, AZWeakReference *ref);
/* Listener */
static void weak_reference_object_dispose (AZActiveObject *object, void *data);

AZObjectEventVector weak_reference_event_vec = {
	weak_reference_object_dispose
};

static unsigned int weak_reference_type = 0;

unsigned int
az_weak_reference_get_type (void)
{
	if (!weak_reference_type) {
		az_register_type (&weak_reference_type, (const unsigned char *) "WeakReference", AZ_TYPE_ANY, sizeof (AZWeakReferenceClass), sizeof (AZWeakReference), AZ_CLASS_ZERO_MEMORY | AZ_FLAG_FINAL,
			NULL, NULL,
			(void (*) (const AZImplementation *, void *)) weak_reference_finalize);
	}
	return weak_reference_type;
}

static void
weak_reference_finalize (AZWeakReferenceClass *klass, AZWeakReference *ref)
{
	if (ref->object) {
		az_active_object_remove_listener_by_data (ref->object, ref);
		//az_object_unref (&ref->object->object);
	}
}

void
az_weak_reference_set (AZWeakReference *ref, AZActiveObject *object)
{
	if (ref->object) az_weak_reference_clear (ref);
	if (object) {
		ref->object = object;
		az_active_object_add_listener (object, &weak_reference_event_vec, sizeof (AZObjectEventVector), ref);
	}
}

void
az_weak_reference_clear (AZWeakReference *ref)
{
	if (ref->object) {
		az_active_object_remove_listener_by_data (ref->object, ref);
		ref->object = NULL;
	}
}

static void
weak_reference_object_dispose (AZActiveObject *object, void *data)
{
	AZWeakReference *ref = (AZWeakReference *) data;
	ref->object = NULL;
}

#endif

