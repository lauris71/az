#ifndef __AZ_WEAK_REFERENCE_H__
#define __AZ_WEAK_REFERENCE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2019
*/

/*
 * Zero reference can be safely used and destroyed without init/finalize cascade
 */

#define AZ_TYPE_WEAK_REFERENCE az_weak_reference_get_type ()

typedef struct _AZWeakReference AZWeakReference;
typedef struct _AZWeakReferenceClass AZWeakReferenceClass;

#include <az/classes/active-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZWeakReference {
	AZActiveObject *object;
};

struct _AZWeakReferenceClass {
	AZClass klass;
};

unsigned int az_weak_reference_get_type (void);

void az_weak_reference_set (AZWeakReference *ref, AZActiveObject *object);
void az_weak_reference_clear (AZWeakReference *ref);

#ifdef __cplusplus
};
#endif

#endif

