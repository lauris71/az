#ifndef __AZ_VALUE_ARRAY_REF_H__
#define __AZ_VALUE_ARRAY_REF_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2020
*/

#define AZ_TYPE_VALUE_ARRAY_REF az_value_array_ref_get_type ()

typedef struct _AZValueArrayRef AZValueArrayRef;
typedef struct _AZValueArrayRefClass AZValueArrayRefClass;

#include <az/reference.h>
#include <az/classes/value-array.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZValueArrayRef {
	AZReference reference;
	AZValueArray varray;
};

struct _AZValueArrayRefClass {
	AZReferenceClass reference_klass;
	AZListImplementation list_implementation;
};

unsigned int az_value_array_ref_get_type (void);

AZValueArrayRef *az_value_array_ref_new (unsigned int length);

void az_value_array_ref_set_element (AZValueArrayRef *varef, unsigned int idx, const AZImplementation *impl, AZValue* value);

#ifdef __cplusplus
};
#endif

#endif
