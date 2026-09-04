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

void
az_reference_ref (AZReference* ref)
{
	unsigned int refcount;
	AZ_REFERENCE_LOCK ();
	refcount = ref->refcount;
	if (refcount) ref->refcount = refcount + 1;
	AZ_REFERENCE_UNLOCK ();
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (refcount);
#endif
}
#else
#define AZ_REFERENCE_LOCK()
#define AZ_REFERENCE_UNLOCK()
#endif

void
az_reference_unref (AZReferenceClass* klass, AZReference* ref)
{
	unsigned int refcount;
	AZ_REFERENCE_LOCK ();
	refcount = ref->refcount;
	if (refcount > 1) {
		ref->refcount = refcount - 1;
		AZ_REFERENCE_UNLOCK ();
		return;
	}
	AZ_REFERENCE_UNLOCK ();
	if (refcount == 1) {
		/* We are guaranteed to be the only thread holding a reference, so the
		 * (possibly slow) drop callback and disposal run without the global lock */
		az_reference_drop (klass, ref);
	}
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (refcount);
#endif
}

void
az_reference_drop (AZReferenceClass *klass, AZReference *ref)
{
	/* The caller held the only counted reference (refcount was 1 when az_reference_unref
	 * made the decision), but the object may have been resurrected through a class-specific
	 * side channel meanwhile - the drop callback is responsible for detecting this */
	if (klass->drop && !klass->drop (klass, ref)) {
		/* Ownership was claimed (e.g. by a resource cache): release the caller's
		 * reference and dispose only if the claim was already dropped again in
		 * another thread */
		unsigned int last;
		AZ_REFERENCE_LOCK ();
		last = (ref->refcount == 1);
		if (!last) ref->refcount -= 1;
		AZ_REFERENCE_UNLOCK ();
		if (last) {
			if (klass->dispose) klass->dispose (klass, ref);
			az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
		}
		return;
	}
	/* No one took ownership of the object */
	if (klass->dispose) klass->dispose (klass, ref);
	az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
}

void
az_reference_dispose (AZReferenceClass *klass, AZReference *ref)
{
	unsigned int refcount;
	AZ_REFERENCE_LOCK ();
	refcount = ref->refcount;
	AZ_REFERENCE_UNLOCK ();
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (refcount);
#else
	if (!refcount) return;
#endif
	/* We are guaranteed to hold reference so no another thread can auto-dispose by unref */
	if (klass->dispose) klass->dispose (klass, ref);
	AZ_REFERENCE_LOCK ();
	refcount = (ref->refcount -= 1);
	AZ_REFERENCE_UNLOCK ();
	if (!refcount) {
		az_instance_delete(AZ_CLASS_TYPE(&klass->klass), ref);
	}
}

AZ_CLASS_ALIGN AZReferenceClass AZReferenceKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_CONSTRUCT | AZ_FLAG_REFERENCE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_REFERENCE },
		.parent = &AZBlockKlass,
		.name = (const uint8_t *) "reference",
		.alignment = 7,
		.class_size = sizeof(AZReferenceClass),
		.instance_size = sizeof(AZReference),
		/* No instance_init: az_instance_init sets the refcount directly (AZ_FLAG_REFERENCE) */
		.to_string = az_any_to_string
	},
	.drop = NULL,
	.dispose = NULL
};

void
az_init_reference_class (void)
{
	az_class_new_with_value(&AZReferenceKlass.klass);
#ifdef AZ_MT_REFERENCES
	mtx_init (&mutex, mtx_plain);
#endif
}
