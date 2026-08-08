#ifndef __AZ_WEAK_OBJECT_LIST_H__
#define __AZ_WEAK_OBJECT_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

/*
 * An resizable array of weakly referenced AZActiveObject either being or implementing certain type
 * Objects are removed from the list automatically on dispose
 */

typedef struct _AZWeakObjectList AZWeakObjectList;
typedef struct _AZWeakObjectListClass AZWeakObjectListClass;

#define AZ_TYPE_WEAK_OBJECT_LIST (az_weak_object_list_get_type ())

#include <az/collections/list.h>
#include <az/classes/active-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZWeakObjectList {
	unsigned int type;
	unsigned int allocated_size;
	AZList list;
	AZActiveObject **objects;
};

struct _AZWeakObjectListClass {
	AZClass klass;
	AZListImplementation list_implementation;
};

unsigned int az_weak_object_list_get_type (void);

void az_weak_object_list_setup (AZWeakObjectList *objl, unsigned int type);
static inline void az_weak_object_list_release (AZWeakObjectList *objl)
{
	az_instance_finalize_by_type (objl, AZ_TYPE_WEAK_OBJECT_LIST);
}

AZWeakObjectList *az_weak_object_list_new (unsigned int type);
void az_weak_object_list_delete (AZWeakObjectList *objl);

void az_weak_object_list_append_object (AZWeakObjectList *objl, AZActiveObject *object);
void az_weak_object_list_insert_object (AZWeakObjectList *objl, AZActiveObject *object, unsigned int pos);
void az_weak_object_list_remove_object (AZWeakObjectList *objl, AZActiveObject *object);
void az_weak_object_list_remove_object_by_index (AZWeakObjectList *objl, unsigned int idx);
void az_weak_object_list_clear (AZWeakObjectList *objl);

unsigned int az_weak_object_list_contains (AZWeakObjectList *objl, AZActiveObject *object);

#ifdef __cplusplus
};
#endif

#endif
