#ifndef __AZ_ATTRIB_DICT_H__
#define __AZ_ATTRIB_DICT_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#define AZ_TYPE_ATTRIBUTE_DICT (az_attrib_dict_get_type ())

typedef struct _AZAttribDictImplementation AZAttribDictImplementation;
typedef struct _AZAttribDictClass AZAttribDictClass;

#include <az/collections/list.h>
#include <az/classes/map.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* String-value surjective map
*
* Iterator type is uint32
* Key type is string
*
* Subclasses have to implement
*   [get_type (unless any)]
*   get_size
*   contains
*   
*   val_list.get_element
*
*   key_list.contains
*   key_list.get_element
*
*   lookup
*   set
*
* Iterator is uint32 and can be shared between all interfaces
*/

#define AZ_ATTRIB_ARRAY_IS_FINAL 1

struct _AZAttribDictImplementation {
	AZMapImplementation map_impl;
	AZListImplementation val_list_impl;
	AZListImplementation key_list_impl;
	const AZImplementation *(*lookup) (const AZAttribDictImplementation *aa_impl, void *aa_inst, const AZString *key, AZValue *val, int size, unsigned int *flags);
	unsigned int (*set) (const AZAttribDictImplementation *aa_impl, void *aa_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags);
};

struct _AZAttribDictClass {
	AZMapClass map_class;
};

unsigned int az_attrib_dict_get_type (void);

const AZImplementation *az_attrib_dict_lookup (const AZAttribDictImplementation *aa_impl, void *aa_inst, const AZString *key, AZValue64 *val, unsigned int *flags);
unsigned int az_attrib_dict_set (const AZAttribDictImplementation *aa_impl, void *aa_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags);

#ifdef __cplusplus
};
#endif

#endif
