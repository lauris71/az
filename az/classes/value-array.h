#ifndef __AZ_VALUE_ARRAY_H__
#define __AZ_VALUE_ARRAY_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_VALUE_ARRAY az_value_array_get_type ()

typedef struct _AZValueArray AZValueArray;
typedef struct _AZValueArrayClass AZValueArrayClass;

#include <az/collections/list.h>
#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AZValueArrayEntry AZValueArrayEntry;

struct _AZValueArrayEntry {
	const AZImplementation *impl;
	union {
		unsigned char value[8];
		unsigned int val_idx;
	};
};

/**
 * @brief A compact dynamic array of values of any type.
 * 
 * It keeps a list of entries that either:
 * - Store the value inline if the value size <= 8 bytes
 * - Reference a value in data pool if value size > 8 bytes
 * 
 * The data buffer is composed of AZValue 'slots', each large value taking one or more slots.
 */
struct _AZValueArray {
	unsigned int type;
	unsigned int size;
	unsigned int data_size;
	AZList list;
	AZValueArrayEntry *values;
	AZValue *data;
};

struct _AZValueArrayClass {
	AZClass klass;
	AZListImplementation list_impl;
	unsigned int default_size;
};

extern AZValueArrayClass *az_value_array_class;

unsigned int az_value_array_get_type (void);

void az_value_array_set_length (AZValueArray *varray, unsigned int length);
const AZImplementation *az_value_array_get_element(AZValueArray *varray, unsigned int idx, AZValue *val, unsigned int size);
/**
 * @brief Set an array element.
 * 
 * @param varray The value array to modify.
 * @param idx The index of the element to set.
 * @param impl The implementation type of the value.
 * @param inst Pointer to the instance data.
 */
void az_value_array_set_element(AZValueArray *varray, unsigned int idx, const AZImplementation *impl, void *inst);
/**
 * @brief Copy an array element from a value.
 * 
 * If the value is boxed, unbox automatically before copying.
 * 
 * @param varray The value array to modify.
 * @param idx The index of the element to set.
 * @param impl The implementation type of the value.
 * @param val Pointer to the value.
 */
void az_value_array_set_element_from_val(AZValueArray *varray, unsigned int idx, const AZImplementation *impl, const AZValue *val);
/**
 * @brief Move an array element from a value.
 * 
 * Transfers ownership of the value from the source to the array,
 * leaving the source value in an uninitalized state.
 * If the value is boxed, unbox automatically before transferring.
 * 
 * @param varray The value array to modify.
 * @param idx The index of the element to set.
 * @param impl The implementation type of the value.
 * @param val Pointer to the value.
 */
void az_value_array_transfer_element (AZValueArray *varray, unsigned int idx, const AZImplementation *impl, const AZValue *val);

#ifdef __cplusplus
};
#endif

#endif
