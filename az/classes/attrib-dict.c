#define __AZ_ATTRIB_DICT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/packed-value.h>

#include "attrib-dict.h"

static void attrib_dict_class_init (AZAttribDictClass *klass);
static void attrib_dict_impl_init (AZAttribDictImplementation *impl);

/* We need three sets of collecion methods */
/* Base */
static unsigned int attrd_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
static const AZImplementation *attrd_get_iterator (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
static const AZImplementation *attrd_iterator_next (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
static unsigned int attrd_get_key_type (const AZMapImplementation *map_impl, AZMap *map_inst);
const AZImplementation *attrd_lookup (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size);

static unsigned int attrib_dict_type = 0;
static AZAttribDictClass *attrib_dict_class;

unsigned int
az_attrib_dict_get_type (void)
{
	if (attrib_dict_type) return attrib_dict_type;
	AZ_TYPES_LOCK();
	if (!attrib_dict_type) {
		attrib_dict_class = (AZAttribDictClass *) az_register_interface_type (&attrib_dict_type, (const unsigned char *) "AZAttributeArray", AZ_TYPE_LIST,
			sizeof (AZAttribDictClass), sizeof (AZAttribDictImplementation), sizeof(AZAttribDict), AZ_FLAG_ZERO_MEMORY,
			0, 0,
			NULL,
			(void (*) (AZImplementation *)) attrib_dict_impl_init,
			NULL, NULL);
	}
	AZ_TYPES_UNLOCK();
	return attrib_dict_type;
}

static void
attrib_dict_impl_init (AZAttribDictImplementation *impl)
{
	impl->map_impl.collection_impl.get_element_type = attrd_get_element_type;
	impl->map_impl.collection_impl.get_iterator = attrd_get_iterator;
	impl->map_impl.collection_impl.iterator_next = attrd_iterator_next;
	impl->map_impl.get_key_type = attrd_get_key_type;
	impl->map_impl.lookup = attrd_lookup;
}

/* Base */

static unsigned int
attrd_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	return AZ_TYPE_ANY;
}

static const AZImplementation *
attrd_get_iterator (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	if (!az_collection_get_size (coll_impl, coll_inst)) return NULL;
	iter->uint32_v = 0;
	return &AZUint32Klass.impl;
}

static const AZImplementation *
attrd_iterator_next (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	iter->uint32_v += 1;
	return (iter->uint32_v < az_collection_get_size (coll_impl, coll_inst)) ? &AZUint32Klass.impl : NULL;
}

static unsigned int
attrd_get_key_type (const AZMapImplementation *map_impl, AZMap *map_inst)
{
	return AZ_TYPE_STRING;
}

const AZImplementation *
attrd_lookup (const AZMapImplementation *map_impl, AZMap *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size)
{
	AZAttribDictImplementation *impl = (AZAttribDictImplementation *) map_impl;
	unsigned int flags;
	if (!key_impl || (AZ_IMPL_TYPE(key_impl) != AZ_TYPE_STRING)) return NULL;
	return impl->lookup (impl, (AZAttribDict *) map_inst, (AZString *) key_inst, val, size, &flags);
}
