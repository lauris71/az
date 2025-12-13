#ifndef __AZ_REFERENCE_H__
#define __AZ_REFERENCE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2019
*/

/**
 * Use multi-threaded reference counting
 * Single global mutex is created for all reference types
 */

#define AZ_MT_REFERENCES

typedef struct _AZReferenceClass AZReferenceClass;

#include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZReference {
	uint32_t refcount;
};

struct _AZReferenceClass {
	AZClass klass;
	/**
	 * @brief Lifecycle management
	 *
	 * Called before the last reference to instance will be dropped (unref with refcount 1) to
	 * allow object managers to claim ownership of unowned objects.
	 * If it returns 1, instance will be disposed and deleted immediately. Otherwise reference
	 * count is decreased and instance disposed and deleted only there are no remaining references to it.
	 * It is called with mutex unlocked.
	 *
	 * @return 1 if object should be disposed, 0 if someone else aquired new reference
	 * @see dispose
	 * @see az_reference_ref
	 * @see az_reference_unref
	 */
	unsigned int (*drop) (AZReferenceClass *klass, AZReference *ref);
	/**
	 * @brief Shutdown instance
	 *
	 * Dispose should release resources and references hold by this instance.
	 * It is invoked automatically by unref before finalizing instance but may be
	 * called from user code to signal that given instance will not be used anymore by
	 * any remaining reference holders.
	 * Subclasses must ensure that it is safe to call it multiple times.
	 *
	 * @see az_reference_dispose
	 */
	void (*dispose) (AZReferenceClass *klass, AZReference *ref);
};

extern AZReferenceClass AZReferenceKlass;

#ifdef AZ_MT_REFERENCES
void az_reference_ref (AZReference* ref);
void az_reference_unref (AZReferenceClass* klass, AZReference* ref);
#else
ARIKKEI_INLINE void
az_reference_ref (AZReference *ref)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (ref->refcount);
#endif
	ref->refcount += 1;
}

ARIKKEI_INLINE void
az_reference_unref (AZReferenceClass *klass, AZReference *ref)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (ref->refcount);
#endif
	if (ref->refcount == 1) {
		az_reference_drop (klass, ref);
	} else {
		ref->refcount -= 1;
	}
}
#endif

/**
 * @brief Shutdown instance and drop reference
 *
 * Releases resources and references hold by this instance and drops reference
 * unconditionally (not calling AZReferenceClass::drop for last reference)
 */
void az_reference_dispose (AZReferenceClass *klass, AZReference *ref);

#ifdef __cplusplus
};
#endif

#endif
