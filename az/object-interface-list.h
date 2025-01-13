#ifndef __AZ_OBJECT_INTERFACE_LIST_H__
#define __AZ_OBJECT_INTERFACE_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2018
*/

/*
* An resizable array of AZObjects implementing certain type
*/

typedef struct _AZObjectInterfaceList AZObjectInterfaceList;
typedef struct _AZObjectInterfaceListClass AZObjectInterfaceListClass;
typedef struct _AZObjectInterfaceListElement AZObjectInterfaceListElement;

#define AZ_TYPE_OBJECT_INTERFACE_LIST (az_object_interface_list_get_type ())

#include <az/list.h>
#include <az/object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZObjectInterfaceListElement {
	const AZImplementation *impl;
	void *inst;
	AZObject *obj;
};

struct _AZObjectInterfaceList {
	unsigned int object_type;
	unsigned int interface_type;
	unsigned int size;
	unsigned int length;
	AZObjectInterfaceListElement *elements;
};

struct _AZObjectInterfaceListClass {
	AZClass klass;
	AZListImplementation list_implementation;
};

unsigned int az_object_interface_list_get_type (void);

void az_object_interface_list_setup (AZObjectInterfaceList *objifl, unsigned int object_type, unsigned int interface_type);
void az_object_interface_list_release (AZObjectInterfaceList *objifl);

AZObjectInterfaceList *az_object_interface_list_new (unsigned int object_type, unsigned int interface_type);
void az_object_interface_list_delete (AZObjectInterfaceList *objifl);

void az_object_interface_list_append_object (AZObjectInterfaceList *objifl, AZObject *object);
void az_object_interface_list_remove_object (AZObjectInterfaceList *objifl, AZObject *object);
void az_object_interface_list_remove_object_by_index (AZObjectInterfaceList *objifl, unsigned int idx);
void az_object_interface_list_clear (AZObjectInterfaceList *objifl);

unsigned int az_object_interface_list_contains (AZObjectInterfaceList *objifl, AZObject *object);

#ifdef __cplusplus
};
#endif

#endif
