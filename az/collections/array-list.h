#ifndef __AZ_ARRAY_LIST_H__
#define __AZ_ARRAY_LIST_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2025
*/

/*
 * A resizable array that keeps elements in [impl,value] tuples
 * Oversized values and interfaces are automatically boxed
 * Containment is defined by value equality (taking boxing into account)
 */

#define AZ_TYPE_ARRAY_LIST az_array_list_get_type ()

typedef struct _AZArrayList AZArrayList;
typedef struct _AZArrayListClass AZArrayListClass;

#include <az/collections/list.h>
#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AZArrayListEntry AZArrayListEntry;

struct _AZArrayListEntry {
	const AZImplementation *impl;
    uint8_t val[8];
};

struct _AZArrayList {
    unsigned int type;
	unsigned int length;
	unsigned int val_size;
	unsigned int data_size;
	void *data;
};

struct _AZArrayListClass {
	AZClass klass;
	AZListImplementation list_impl;
    unsigned int default_size;
};

extern AZArrayListClass *AZArrayListKlass;

unsigned int az_array_list_get_type (void);

AZArrayList *az_array_list_new(unsigned int el_type, unsigned int val_size);

static inline void
az_array_list_delete(AZArrayList *alist)
{
	az_instance_delete(AZ_TYPE_ARRAY_LIST, alist);
}

void az_array_list_clear(AZArrayList *alist);
unsigned int az_array_list_append(AZArrayList *alist, const AZImplementation *impl, void *inst);
unsigned int az_array_list_insert(AZArrayList *alist, unsigned int idx, const AZImplementation *impl, void *inst);

static inline AZArrayListEntry *
az_array_list_get_entry(AZArrayList *alist, unsigned int idx)
{
	return (AZArrayListEntry *) ((char *) alist->data + idx * (sizeof(AZArrayListEntry) + alist->val_size - 8));
}

#ifdef __cplusplus
};
#endif

#endif
