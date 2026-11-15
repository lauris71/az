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
	az_weak_reference_clear (ref);
	if (object) {
		/* The caller must hold a strong reference to the new object: it
		 * cannot die between the pointer store and the listener
		 * registration (which would leave a dangling weak pointer) */
		ref->ptr = object;
		az_active_object_add_listener (object, &weak_reference_event_vec, sizeof (AZObjectEventVector), ref);
	}
}

void
az_weak_reference_clear (AZWeakReference *ref)
{
	/* The listener can only be removed while the object is alive, so
	 * promote first: remove_listener_by_data on a dying/freed object would
	 * touch its (possibly freed) callback block. If promotion fails the
	 * object is dead and its dispose has already fired our listener. */
	AZActiveObject *object = az_weak_reference_acquire (ref);
	if (object) {
		az_active_object_remove_listener_by_data (object, ref);
		az_object_unref ((AZObject *) object);
	}
	ref->ptr = NULL;
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

