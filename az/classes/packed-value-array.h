#ifndef __AZ_PACKED_VALUE_ARRAY_H__
#define __AZ_PACKED_VALUE_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_PACKED_VALUE_ARRAY az_packed_value_array_get_type ()

typedef struct _AZPackedValueArray AZPackedValueArray;
typedef struct _AZPackedValueArrayClass AZPackedValueArrayClass;

#include <az/collections/list.h>
#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZPackedValueArray {
	AZReference reference;
	AZList list;
	AZPackedValue *_values;
};

struct _AZPackedValueArrayClass {
	AZReferenceClass reference_klass;
	AZListImplementation list_impl;
};

unsigned int az_packed_value_array_get_type (void);

AZPackedValueArray *az_packed_value_array_new (unsigned int length);

void az_packed_value_array_set_element (AZPackedValueArray *varray, unsigned int idx, const AZPackedValue *value);

#ifdef __cplusplus
};
#endif

#endif
