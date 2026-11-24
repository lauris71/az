#ifndef __AZ_ACTIVE_OBJECT_H__
#define __AZ_ACTIVE_OBJECT_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#define AZ_TYPE_ACTIVE_OBJECT az_active_object_get_type ()
#define AZ_ACTIVE_OBJECT(o) (AZ_CHECK_INSTANCE_CAST ((o), AZ_TYPE_ACTIVE_OBJECT, AZActiveObject))
#define AZ_IS_ACTIVE_OBJECT(o) (AZ_CHECK_INSTANCE_TYPE ((o), AZ_TYPE_ACTIVE_OBJECT))

typedef struct _AZActiveObject AZActiveObject;
typedef struct _AZActiveObjectClass AZActiveObjectClass;

#include <az/classes/attrib-dict.h>
#include <az/object.h>
#include <az/packed-value.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AZObjectAttribute AZObjectAttribute;
typedef struct _AZObjectAttributeArray AZObjectAttributeArray;
typedef struct _AZObjectListener AZObjectListener;
typedef struct _AZObjectCallbackBlock AZObjectCallbackBlock;
typedef struct _AZObjectEventVector AZObjectEventVector;

struct _AZObjectAttribute {
	AZString *key;
	AZPackedValue value;
};

struct _AZObjectAttributeArray {
	unsigned int size;
	unsigned int _length;
	AZObjectAttribute attribs[1];
};

struct _AZObjectEventVector {
	void (*dispose) (AZActiveObject *object, void *data);
};

struct _AZObjectListener {
	const AZObjectEventVector *vector;
	unsigned int size;
	void *data;
};

struct _AZObjectCallbackBlock {
	unsigned int size;
	unsigned int length;
	AZObjectListener listeners[1];
};

struct _AZActiveObject {
	AZObject object;
	AZObjectCallbackBlock *callbacks;
	AZObjectAttributeArray *attributes;
};

struct _AZActiveObjectClass {
	AZObjectClass object_class;
	AZAttribDictImplementation aa_impl;
};

unsigned int az_active_object_get_type (void);

/**
 * @brief Get an attribute value
 *
 * Frontend to az_attrib_dict_lookup.
 *
 * @param aobj the active object
 * @param key the attribute key
 * @param val the destination value (uninitialized)
 * @param val_size the size of the destination value storage
 * @return the implementation of the stored value or NULL if there is no such attribute
 */
const AZImplementation *az_active_object_get_attribute (AZActiveObject *aobj, AZString *key, AZValue *val, unsigned int val_size);
/** Returns AZ_OK or a negative error code */
int az_active_object_set_attribute (AZActiveObject *aobj, AZString *key, const AZImplementation *impl, const AZValue *val);
/** Returns AZ_OK if the attribute was removed, AZ_NOT_FOUND if there is no such attribute */
int az_active_object_clear_attribute (AZActiveObject *aobj, AZString *key);

void az_active_object_add_listener (AZActiveObject *aobj, const AZObjectEventVector *vector, unsigned int size, void *data);
void az_active_object_remove_listener_by_data (AZActiveObject *aobj, void *data);

/* Helpers */
int az_active_object_set_attribute_i32 (AZActiveObject *aobj, const unsigned char *key, int value);
int az_active_object_set_attribute_object (AZActiveObject *aobj, const unsigned char *key, AZObject *value);

#ifdef __cplusplus
};
#endif

#endif

