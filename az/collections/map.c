#define __AZ_MAP_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>

#include <az/field.h>
#include <az/interface.h>
#include <az/packed-value.h>

#include "map.h"

static void map_implementation_init(AZMapImplementation *impl);

unsigned int keyset_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst);
unsigned int keyset_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *val_impl, const void *val_inst);
const AZImplementation *keyset_get_iterator (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
const AZImplementation *keyset_iterator_next (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter);
const AZImplementation *keyset_get_element (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZValue *iter, AZValue *val, unsigned int size);

static unsigned int map_type = 0;
static AZMapClass *map_class;

unsigned int
az_map_get_type (void)
{
	if (!map_type) {
		map_class = (AZMapClass *) az_register_interface_type (&map_type, (const unsigned char *) "AZMap", AZ_TYPE_COLLECTION,
			sizeof(AZMapClass), sizeof(AZMapImplementation), sizeof(AZMap), AZ_FLAG_ZERO_MEMORY,
			0, 0,
			NULL,
			(void (*) (AZImplementation *)) map_implementation_init,
			NULL, NULL);
	}
	return map_type;
}

static void
map_implementation_init(AZMapImplementation *impl)
{
	az_implementation_init_by_type((AZImplementation *) &impl->keyset_impl, AZ_TYPE_SET);
	impl->keyset_impl.collection_impl.get_element_type = keyset_get_element_type;
	impl->keyset_impl.collection_impl.contains = keyset_contains;
	impl->keyset_impl.collection_impl.get_iterator = keyset_get_iterator;
	impl->keyset_impl.collection_impl.iterator_next = keyset_iterator_next;
	impl->keyset_impl.collection_impl.get_element = keyset_get_element;
}

unsigned int
keyset_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZMapImplementation *map_impl = (AZMapImplementation *) ARIKKEI_BASE_ADDRESS(AZMapImplementation,keyset_impl,coll_impl);
	return map_impl->get_key_type(map_impl, (AZMap *) coll_inst);
}

unsigned int
keyset_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *val_impl, const void *val_inst)
{
	AZMapImplementation *map_impl = (AZMapImplementation *) ARIKKEI_BASE_ADDRESS(AZMapImplementation,keyset_impl,coll_impl);
	return map_impl->contains_key(map_impl, (AZMap *) coll_inst, val_impl, val_inst);
}

const AZImplementation *
keyset_get_iterator (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	AZMapImplementation *map_impl = (AZMapImplementation *) ARIKKEI_BASE_ADDRESS(AZMapImplementation,keyset_impl,coll_impl);
	return map_impl->collection_impl.get_iterator(&map_impl->collection_impl, coll_inst, iter);
}

const AZImplementation *
keyset_iterator_next (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, AZValue *iter)
{
	AZMapImplementation *map_impl = (AZMapImplementation *) ARIKKEI_BASE_ADDRESS(AZMapImplementation,keyset_impl,coll_impl);
	return map_impl->collection_impl.iterator_next(&map_impl->collection_impl, coll_inst, iter);
}

const AZImplementation *
keyset_get_element (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZValue *iter, AZValue *val, unsigned int size)
{
	AZMapImplementation *map_impl = (AZMapImplementation *) ARIKKEI_BASE_ADDRESS(AZMapImplementation,keyset_impl,coll_impl);
	return map_impl->get_key(map_impl, (AZMap *) coll_inst, iter, val, size);
}
