#ifndef __AZ_WEAK_REFERENCE_H__
#define __AZ_WEAK_REFERENCE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

/**
 * @brief Weak reference to an active object
 * 
 * Stores a pointer to an AZActivbeObject without claiming reference. The pointer is cleared
 * when the object is destroyed (in dispose handler).
 * 
 * Zero reference can be safely used and destroyed without init/finalize cascade
 */

#define AZ_TYPE_WEAK_REFERENCE az_weak_reference_get_type ()

typedef struct _AZWeakReference AZWeakReference;

#include <az/classes/active-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZWeakReference {
	AZActiveObject *ptr;
};

void az_weak_reference_set (AZWeakReference *ref, AZActiveObject *object);
void az_weak_reference_clear (AZWeakReference *ref);
/**
 * @brief Acquire a strong reference to the weakly referenced object
 *
 * Weak->strong promotion (docs/mt-reference-locking.md §7): atomically acquires
 * a counted reference if the object is still alive. This is the only safe way
 * to extract the pointer in multi-threaded builds; do not read ref->ptr directly.
 *
 * @return the object with an owned strong reference (unref when done),
 * or NULL if the object has died
 */
AZActiveObject *az_weak_reference_acquire (AZWeakReference *ref);

#ifdef __cplusplus
};
#endif

#endif

