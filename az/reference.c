#define __AZ_REFERENCE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/class.h>
#include <az/instance.h>
#include <az/private.h>
#include <az/reference.h>

#ifdef AZ_MT_REFERENCES
#include <arikkei/arikkei-threads.h>
#endif

#ifdef AZ_MT_REFERENCES
mtx_t mutex;

#define AZ_REFERENCE_LOCK() mtx_lock (&mutex)
#define AZ_REFERENCE_UNLOCK() mtx_unlock (&mutex)

static void az_reference_drop (AZReferenceClass *klass, AZReference *ref);

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
		az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
	} else {
		/* Someone took ownership but may have dropped it in another thread */
		AZ_REFERENCE_LOCK ();
		if (ref->refcount == 1) {
			AZ_REFERENCE_UNLOCK ();
			if (klass->dispose) klass->dispose (klass, ref);
			az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
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
		az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
	} else {
		AZ_REFERENCE_UNLOCK ();
	}
}

static void
reference_instance_init (AZReferenceClass *klass, void *instance)
{
	((AZReference *) instance)->refcount = 1;
}

AZReferenceClass AZReferenceKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_REFERENCE},
	&AZBlockKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "reference",
	7, sizeof(AZReferenceClass), 0,
	NULL,
	(void (*) (const AZImplementation *, void *)) reference_instance_init, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL},
	NULL, NULL
};

void
az_init_reference_class (void)
{
	az_class_new_with_value(&AZReferenceKlass.klass);
#ifdef AZ_MT_REFERENCES
	mtx_init (&mutex, mtx_plain);
#endif
}
