#ifndef __AZ_WEAK_REFERENCE_C__
#define __AZ_WEAK_REFERENCE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

#include <az/private.h>

#include "weak-reference.h"

/* Listener */
static void weak_reference_object_dispose (AZActiveObject *object, void *data);

AZObjectEventVector weak_reference_event_vec = {
	weak_reference_object_dispose
};

void
az_weak_reference_set (AZWeakReference *ref, AZActiveObject *object)
{
	if (ref->ptr) az_weak_reference_clear (ref);
	if (object) {
		ref->ptr = object;
		az_active_object_add_listener (object, &weak_reference_event_vec, sizeof (AZObjectEventVector), ref);
	}
}

void
az_weak_reference_clear (AZWeakReference *ref)
{
	if (ref->ptr) {
		az_active_object_remove_listener_by_data (ref->ptr, ref);
		ref->ptr = NULL;
	}
}

AZActiveObject *
az_weak_reference_acquire (AZWeakReference *ref)
{
#ifdef AZ_MT_REFERENCES
	/* The read side of the lifecycle lock keeps the object memory valid while
	 * try_ref does its CAS vs DEAD (docs/mt-reference-locking.md §7) */
	AZActiveObject *object;
	az_lc_read_lock ();
	object = ref->ptr;
	if (object && !az_reference_try_ref (&object->object.reference)) object = NULL;
	az_lc_read_unlock ();
	return object;
#else
	AZActiveObject *object = ref->ptr;
	if (object) az_reference_ref (&object->object.reference);
	return object;
#endif
}

static void
weak_reference_object_dispose (AZActiveObject *object, void *data)
{
	AZWeakReference *ref = (AZWeakReference *) data;
	ref->ptr = NULL;
}

#endif

