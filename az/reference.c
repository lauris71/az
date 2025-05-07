#define __AZ_REFERENCE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/private.h>
#include <az/reference.h>

#ifdef AZ_MT_REFERENCES
#include <arikkei/arikkei-threads.h>
#endif

#ifdef AZ_MT_REFERENCES
mtx_t mutex;

#define AZ_REFERENCE_LOCK() mtx_lock (&mutex)
#define AZ_REFERENCE_UNLOCK() mtx_unlock (&mutex)

void
az_reference_ref (AZReference* ref)
{
	AZ_REFERENCE_LOCK ();
#ifdef AZ_SAFETY_CHECKS
	if (!ref->refcount) {
		AZ_REFERENCE_UNLOCK ();
		return;
	}
#endif
	ref->refcount += 1;
	AZ_REFERENCE_UNLOCK ();
}

void
az_reference_unref (AZReferenceClass* klass, AZReference* ref)
{
	AZ_REFERENCE_LOCK ();
#ifdef AZ_SAFETY_CHECKS
	if (!ref->refcount) {
		AZ_REFERENCE_UNLOCK ();
		return;
	}
#endif
	if (ref->refcount == 1) {
		AZ_REFERENCE_UNLOCK ();
		az_reference_drop (klass, ref);
	} else {
		ref->refcount -= 1;
		AZ_REFERENCE_UNLOCK ();
	}
}
#else
#define AZ_REFERENCE_LOCK()
#define AZ_REFERENCE_UNLOCK()
#endif

void
az_reference_drop (AZReferenceClass *klass, AZReference *ref)
{
#ifdef AZ_SAFETY_CHECKS
	AZ_REFERENCE_LOCK ();
	if (ref->refcount != 1) {
		AZ_REFERENCE_UNLOCK ();
		return;
	}
	AZ_REFERENCE_UNLOCK ();
#endif
	/* We are guaranteed to hold the only reference to this object */
	if (!klass->drop || klass->drop (klass, ref)) {
		/* No one took ownership of the object */
		if (klass->dispose) klass->dispose (klass, ref);
		az_instance_delete (klass->klass.implementation.type, ref);
	} else {
		/* Someone took ownership but may have dropped it in another thread */
		AZ_REFERENCE_LOCK ();
		//ref->refcount -= 1;
		//if (!ref->refcount) {
		if (ref->refcount == 1) {
			AZ_REFERENCE_UNLOCK ();
			if (klass->dispose) klass->dispose (klass, ref);
			az_instance_delete (klass->klass.implementation.type, ref);
		} else {
            ref->refcount -= 1;
			AZ_REFERENCE_UNLOCK ();
		}
	}
}

void
az_reference_dispose (AZReferenceClass *klass, AZReference *ref)
{
#ifdef AZ_SAFETY_CHECKS
	AZ_REFERENCE_LOCK ();
	if (ref->refcount == 0) {
		AZ_REFERENCE_UNLOCK ();
		return;
	}
	AZ_REFERENCE_UNLOCK ();
#endif
	/* We are guaranteed to hold reference so no another threas can auto-dispose by unref */
	if (klass->dispose) klass->dispose (klass, ref);
	AZ_REFERENCE_LOCK ();
	ref->refcount -= 1;
	if (!ref->refcount) {
		AZ_REFERENCE_UNLOCK ();
		az_instance_delete (klass->klass.implementation.type, ref);
	} else {
		AZ_REFERENCE_UNLOCK ();
	}
}

static AZReferenceClass *reference_class = NULL;

static void
reference_instance_init (AZReferenceClass *klass, void *instance)
{
	((AZReference *) instance)->refcount = 1;
}

void
az_init_reference_class (void)
{
	reference_class = (AZReferenceClass *) az_class_new_with_type (AZ_TYPE_REFERENCE, AZ_TYPE_BLOCK, sizeof (AZReferenceClass), sizeof (AZReference), AZ_FLAG_ABSTRACT | AZ_FLAG_REFERENCE, (const uint8_t *) "reference");
	reference_class->klass.instance_init = (void (*) (const AZImplementation *, void *)) reference_instance_init;
#ifdef AZ_MT_REFERENCES
	mtx_init (&mutex, mtx_plain);
#endif
}
