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
typedef struct _AZAttribDict AZAttribDict;

#include <az/value.h>
#include <az/collections/list.h>
#include <az/collections/map.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* String-value surjective map
*
* Iterator type is uint32
* Key type is string
* It is mainly used as a common base interface for azo named attributes
*/

#define AZ_ATTRIB_ARRAY_IS_FINAL 1

struct _AZAttribDictImplementation {
	AZMapImplementation map_impl;
	const AZImplementation *(*lookup) (const AZAttribDictImplementation *aa_impl, AZAttribDict *aa_inst, const AZString *key, AZValue *val, int size, unsigned int *flags);
	/**
	 * Returns AZ_OK or a negative error code (e.g. AZ_VALUE_IS_FINAL when the
	 * entry is read-only and cannot be replaced). A NULL @c impl removes the entry.
	 */
	int (*set) (const AZAttribDictImplementation *aa_impl, AZAttribDict *aa_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags);
};

struct _AZAttribDictClass {
	AZMapClass map_class;
};

unsigned int az_attrib_dict_get_type (void);

static inline const AZImplementation *
az_attrib_dict_lookup (const AZAttribDictImplementation *attrd_impl, AZAttribDict *attrd_inst, const AZString *key, AZValue *val, unsigned int size, unsigned int *flags)
{
	const AZImplementation *impl = attrd_impl->lookup (attrd_impl, attrd_inst, key, val, size, flags);
	return impl;
}

static inline int
az_attrib_dict_set (const AZAttribDictImplementation *attrd_impl, AZAttribDict *attrd_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags)
{
	return attrd_impl->set (attrd_impl, attrd_inst, key, impl, inst, flags);
}

#ifdef __cplusplus
};
#endif

#endif
