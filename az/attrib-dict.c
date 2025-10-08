#define __AZ_ATTRIB_DICT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2021
*/

#include <arikkei/arikkei-utils.h>

#include <az/attrib-dict.h>
#include <az/packed-value.h>

static void attrib_dict_class_init (AZAttribDictClass *klass);
static void attrib_dict_impl_init (AZAttribDictImplementation *impl);

/* We need three sets of collecion methods */
/* Base */
static unsigned int attrd_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int attrd_get_iterator (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iter);
static unsigned int attrd_iterator_next (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iter);
const AZImplementation *attrd_get_element (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZPackedValue *iterator, AZValue64 *val);
static unsigned int attrd_get_key_type (const AZMapImplementation *map_impl, void *map_inst);
static const AZImplementation *attrd_get_key (const AZMapImplementation *map_impl, void *map_inst, const AZPackedValue *iter, AZValue64 *val);
const AZCollectionImplementation *attrd_get_keys (const AZMapImplementation *map_impl, void *map_inst, void **inst);
const AZImplementation *attrd_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue64 *val);
/* Value list */
static unsigned int attrd_val_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int attrd_val_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int attrd_val_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
/* Key list */
static unsigned int attrd_keys_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int attrd_keys_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst);

static unsigned int attrib_dict_type = 0;
static AZAttribDictClass *attrib_dict_class;

unsigned int
az_attrib_dict_get_type (void)
{
	if (!attrib_dict_type) {
		attrib_dict_class = (AZAttribDictClass *) az_register_interface_type (&attrib_dict_type, (const unsigned char *) "AZAttributeArray", AZ_TYPE_LIST,
			sizeof (AZAttribDictClass), sizeof (AZAttribDictImplementation), 0, AZ_FLAG_ZERO_MEMORY,
			(void (*) (AZClass *)) attrib_dict_class_init,
			(void (*) (AZImplementation *)) attrib_dict_impl_init,
			NULL, NULL);
	}
	return attrib_dict_type;
}

static void
attrib_dict_class_init (AZAttribDictClass *klass)
{
}

static void
attrib_dict_impl_init (AZAttribDictImplementation *impl)
{
	az_implementation_init (&impl->val_list_impl.collection_impl.implementation, AZ_TYPE_LIST);
	az_implementation_init (&impl->key_list_impl.collection_impl.implementation, AZ_TYPE_LIST);
	/* Base */
	impl->map_impl.collection_impl.get_element_type = attrd_get_element_type;
	impl->map_impl.collection_impl.get_iterator = attrd_get_iterator;
	impl->map_impl.collection_impl.iterator_next = attrd_iterator_next;
	impl->map_impl.collection_impl.get_element = attrd_get_element;
	impl->map_impl.get_key_type = attrd_get_key_type;
	impl->map_impl.get_key = attrd_get_key;
	impl->map_impl.get_keys = attrd_get_keys;
	impl->map_impl.lookup = attrd_lookup;
	/* Value list */
	impl->val_list_impl.collection_impl.get_element_type = attrd_val_element_type;
	impl->val_list_impl.collection_impl.get_size = attrd_val_get_size;
	impl->val_list_impl.collection_impl.contains = attrd_val_contains;
	/* Key list */
	impl->key_list_impl.collection_impl.get_element_type = attrd_keys_get_element_type;
	impl->key_list_impl.collection_impl.get_size = attrd_keys_get_size;
}

/* Base */

static unsigned int
attrd_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	return AZ_TYPE_ANY;
}

static unsigned int
attrd_get_iterator (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iter)
{
	az_packed_value_set_unsigned_int (iter, AZ_TYPE_UINT32, 0);
	return 1;
}

static unsigned int
attrd_iterator_next (const AZCollectionImplementation *coll_impl, void *coll_inst, AZPackedValue *iter)
{
	if (iter->v.uint32_v >= az_collection_get_size (coll_impl, coll_inst)) return 0;
	iter->v.uint32_v += 1;
	return 1;
}

const AZImplementation *
attrd_get_element (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZPackedValue *iter, AZValue64 *val)
{
	AZAttribDictImplementation *impl = (AZAttribDictImplementation *) coll_impl;
	return az_list_get_element (&impl->val_list_impl, coll_inst, iter->v.uint32_v, val);
}

static unsigned int
attrd_get_key_type (const AZMapImplementation *map_impl, void *map_inst)
{
	return AZ_TYPE_STRING;
}

static const AZImplementation *
attrd_get_key (const AZMapImplementation *map_impl, void *map_inst, const AZPackedValue *iter, AZValue64 *val)
{
	AZAttribDictImplementation *impl = (AZAttribDictImplementation *) map_impl;
	return az_list_get_element (&impl->key_list_impl, map_inst, iter->v.uint32_v, val);
}

const AZCollectionImplementation *
attrd_get_keys (const AZMapImplementation *map_impl, void *map_inst, void **inst)
{
	AZAttribDictImplementation *impl = (AZAttribDictImplementation *) map_impl;
	*inst = map_inst;
	return &impl->key_list_impl.collection_impl;
}

const AZImplementation *
attrd_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue64 *val)
{
	AZAttribDictImplementation *impl = (AZAttribDictImplementation *) map_impl;
	unsigned int flags;
	if (!key_impl || (AZ_IMPL_TYPE(key_impl) != AZ_TYPE_STRING)) return NULL;
	return impl->lookup (impl, map_inst, (AZString *) key_inst, val, &flags);
}

/* Value list */

static unsigned int
attrd_val_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZAttribDictImplementation *attrd_impl = (AZAttribDictImplementation *) ARIKKEI_BASE_ADDRESS(AZAttribDictImplementation,val_list_impl,coll_impl);
	return attrd_impl->map_impl.collection_impl.get_element_type (&attrd_impl->map_impl.collection_impl, coll_inst);
}

static unsigned int
attrd_val_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZAttribDictImplementation *attrd_impl = (AZAttribDictImplementation *) ARIKKEI_BASE_ADDRESS(AZAttribDictImplementation,val_list_impl,coll_impl);
	return attrd_impl->map_impl.collection_impl.get_size (&attrd_impl->map_impl.collection_impl, coll_inst);
}

static unsigned int
attrd_val_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst)
{
	AZAttribDictImplementation *attrd_impl = (AZAttribDictImplementation *) ARIKKEI_BASE_ADDRESS(AZAttribDictImplementation,val_list_impl,coll_impl);
	return attrd_impl->map_impl.collection_impl.contains (&attrd_impl->map_impl.collection_impl, coll_inst, impl, inst);
}

/* Key list */

static unsigned int
attrd_keys_get_element_type (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	return AZ_TYPE_STRING;
}

static unsigned int
attrd_keys_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZAttribDictImplementation *attrd_impl = (AZAttribDictImplementation *) ARIKKEI_BASE_ADDRESS (AZAttribDictImplementation, key_list_impl, coll_impl);
	return attrd_impl->map_impl.collection_impl.get_size (&attrd_impl->map_impl.collection_impl, coll_inst);
}


const AZImplementation *
az_attrib_dict_lookup (const AZAttribDictImplementation *attrd_impl, void *attrd_inst, const AZString *key, AZValue64 *val, unsigned int *flags)
{
	return attrd_impl->lookup (attrd_impl, attrd_inst, key, val, flags);
}

unsigned int
az_attrib_dict_set (const AZAttribDictImplementation *attrd_impl, void *attrd_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags)
{
	return attrd_impl->set (attrd_impl, attrd_inst, key, impl, inst, flags);
}
