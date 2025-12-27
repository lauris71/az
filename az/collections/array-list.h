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

#include <az/classes/list.h>
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
	unsigned int size;
	unsigned int length;
	unsigned int val_size;
	AZArrayListEntry *values;
};

struct _AZArrayListClass {
	AZClass klass;
	AZListImplementation list_implementation;
    unsigned int default_size;
};

extern AZArrayListClass *AZArrayListKlass;

unsigned int az_array_list_get_type (void);

AZArrayList *az_array_list_new(unsigned int el_type, unsigned int val_size);

unsigned int az_array_list_append(AZArrayList *alist, const AZImplementation *impl, void *inst);

#ifdef __cplusplus
};
#endif

#endif
