#ifndef __AZ_VALUE_ARRAY_H__
#define __AZ_VALUE_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_VALUE_ARRAY az_value_array_get_type ()
#define AZ_TYPE_PACKED_VALUE_ARRAY az_packed_value_array_get_type ()

typedef struct _AZValueArray AZValueArray;
typedef struct _AZValueArrayClass AZValueArrayClass;
typedef struct _AZPackedValueArray AZPackedValueArray;
typedef struct _AZPackedValueArrayClass AZPackedValueArrayClass;

#include <az/classes/list.h>
#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Convenience union for performing pointer arithmetic */

typedef struct _AZValueArrayEntry AZValueArrayEntry;

struct _AZValueArrayEntry {
	const AZImplementation *impl;
	union {
		unsigned char value[8];
		unsigned int val_idx;
	};
};

struct _AZValueArray {
	unsigned int type;
	unsigned int size;
	unsigned int data_size;
	unsigned int length;
	AZValueArrayEntry *values;
	AZValue *data;
};

struct _AZValueArrayClass {
	AZClass klass;
	AZListImplementation list_implementation;
	unsigned int default_size;
};

extern AZValueArrayClass *az_value_array_class;

unsigned int az_value_array_get_type (void);

void az_value_array_set_length (AZValueArray *varray, unsigned int length);
void az_value_array_set_element (AZValueArray *varray, unsigned int idx, const AZImplementation *impl, const AZValue *val);
void az_value_array_transfer_element (AZValueArray *varray, unsigned int idx, const AZImplementation *impl, const AZValue *val);

struct _AZPackedValueArray {
	AZReference reference;
	unsigned int length;
	AZPackedValue *_values;
};

struct _AZPackedValueArrayClass {
	AZReferenceClass reference_klass;
	AZListImplementation list_implementation;
};

unsigned int az_packed_value_array_get_type (void);

AZPackedValueArray *az_packed_value_array_new (unsigned int length);

void az_packed_value_array_set_element (AZPackedValueArray *varray, unsigned int idx, const AZPackedValue *value);

#ifdef __cplusplus
};
#endif

#endif
