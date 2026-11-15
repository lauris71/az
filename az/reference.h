#ifndef __AZ_REFERENCE_H__
#define __AZ_REFERENCE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2019
*/

/**
 * Use multi-threaded reference counting
 * Strong references are atomic; weak->strong promotion and the final memory
 * release are mutually excluded by a lifecycle R/W lock
 * (see docs/mt-reference-locking.md)
 */

typedef struct _AZReferenceClass AZReferenceClass;

#include <arikkei/arikkei-utils.h>

#include <stdatomic.h>

#include <az/class.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZReference {
#ifdef AZ_MT_REFERENCES
	_Atomic uint32_t refcount;
#else
	uint32_t refcount;
#endif
};

/*
 * Refcount word states: 0 = invalid (instances are born at 1),
 * 1..0xfffffffe = strong count, AZ_REFERENCE_DEAD = disposal claimed
 * (unreachable for new strong references and weak->strong promotion)
 */
#define AZ_REFERENCE_DEAD 0xffffffffu

struct _AZReferenceClass {
	AZClass klass;
	/**
	 * @brief Lifecycle management
	 *
	 * Called before the last reference to instance will be dropped (unref with refcount 1) to
	 * allow object managers to claim ownership of unowned objects.
	 * If it returns 1, instance will be disposed and deleted immediately. Otherwise reference
	 * count is decreased and instance disposed and deleted only if there are no remaining references to it.
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

#ifdef AZ_REFERENCE_HOOKS
extern void (*az_reference_ref_hook) (AZReference *);
extern void (*az_reference_unref_hook) (AZReferenceClass *, AZReference *);
#endif

#ifdef AZ_MT_REFERENCES
ARIKKEI_INLINE void
az_reference_ref (AZReference *ref)
{
#ifdef AZ_REFERENCE_HOOKS
	if (az_reference_ref_hook) az_reference_ref_hook(ref);
#endif
	uint32_t n = atomic_load_explicit (&ref->refcount, memory_order_relaxed);
	for (;;) {
#ifdef AZ_SAFETY_CHECKS
		arikkei_return_if_fail (n != 0 && n != AZ_REFERENCE_DEAD);
#else
		if (n == 0 || n == AZ_REFERENCE_DEAD) return;
#endif
		if (atomic_compare_exchange_weak_explicit (&ref->refcount, &n, n + 1, memory_order_relaxed, memory_order_relaxed)) return;
	}
}
#else
ARIKKEI_INLINE void
az_reference_ref (AZReference *ref)
{
#ifdef AZ_REFERENCE_HOOKS
	if (az_reference_ref_hook) az_reference_ref_hook(ref);
#endif
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (ref->refcount);
#endif
	ref->refcount += 1;
}
#endif
/**
 * @brief Try to acquire a strong reference (weak->strong promotion primitive)
 *
 * Succeeds only if the object is alive: the count is brought from n to n+1
 * atomically, so a concurrent final unref either loses to us (its DEAD claim
 * fails) or has already claimed disposal (we fail). Fails on AZ_REFERENCE_DEAD
 * and on 0 (invalid). On success a strong reference is held (acquire ordering).
 *
 * In single-threaded builds this is a plain read + ref: a non-NULL weak pointer
 * always refers to a live object there (weak pointers are cleared synchronously
 * during dispose, and nothing can interleave).
 */
#ifdef AZ_MT_REFERENCES
ARIKKEI_INLINE unsigned int
az_reference_try_ref (AZReference *ref)
{
	uint32_t n = atomic_load_explicit (&ref->refcount, memory_order_relaxed);
	for (;;) {
		if (n == 0 || n == AZ_REFERENCE_DEAD) return 0;
		if (atomic_compare_exchange_weak_explicit (&ref->refcount, &n, n + 1, memory_order_acquire, memory_order_relaxed)) return 1;
	}
}
#else
ARIKKEI_INLINE unsigned int
az_reference_try_ref (AZReference *ref)
{
	if (!ref->refcount) return 0;
	ref->refcount += 1;
	return 1;
}
#endif
/* The last unref goes through the class drop callback and the destruction tail (reference.c) */
void az_reference_unref (AZReferenceClass* klass, AZReference* ref);

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
