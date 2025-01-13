#ifndef __AZ_INTERFACE_LIST_H__
#define __AZ_INTERFACE_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2020
*/

/*
* An resizable array of interfaces
*/

typedef struct _AZInterfaceList AZInterfaceList;
typedef struct _AZInterfaceListClass AZInterfaceListClass;

#define AZ_TYPE_INTERFACE_LIST (az_interface_list_get_type ())

#include <az/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZInterfaceList {
	unsigned int iface_type;
	unsigned int size;
	unsigned int length;
	AZInterfaceValue* elements;
};

struct _AZInterfaceListClass {
	AZClass klass;
	AZListImplementation list_impl;
};

unsigned int az_interface_list_get_type (void);

void az_interface_list_setup (AZInterfaceList* ifl, unsigned int iface_type);
void az_interface_list_release (AZInterfaceList* ifl);

AZInterfaceList* az_interface_list_new (unsigned int iface_type);
void az_interface_list_delete (AZInterfaceList* ifl);

void az_interface_list_append (AZInterfaceList* ifl, AZImplementation *impl, void *inst);
void az_interface_list_remove (AZInterfaceList* ifl, void *inst);
void az_interface_list_remove_by_index (AZInterfaceList* ifl, unsigned int idx);
void az_interface_list_clear (AZInterfaceList* ifl);

unsigned int az_interface_list_contains (AZInterfaceList* ifl, void *inst);

#ifdef __cplusplus
};
#endif

#endif
