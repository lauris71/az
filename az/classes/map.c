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

/* AZInterface implementation */
static void map_class_init (AZMapClass *klass);
static void map_implementation_init (AZMapImplementation *impl);

static unsigned int map_type = 0;
static AZMapClass *map_class;

unsigned int
az_map_get_type (void)
{
	if (!map_type) {
		map_class = (AZMapClass *) az_register_interface_type (&map_type, (const unsigned char *) "AZMap", AZ_TYPE_COLLECTION,
			sizeof (AZMapClass), sizeof (AZMapImplementation), 0, AZ_FLAG_ZERO_MEMORY,
			(void (*) (AZClass *)) map_class_init,
			(void (*) (AZImplementation *)) map_implementation_init,
			NULL, NULL);
	}
	return map_type;
}

static void
map_class_init (AZMapClass *klass)
{
}

static void
map_implementation_init (AZMapImplementation *impl)
{
}

unsigned int
az_map_get_key_type (const AZMapImplementation *map_impl, void *map_inst)
{
	return map_impl->get_key_type (map_impl, map_inst);
}

const AZImplementation *
az_map_get_key (const AZMapImplementation *map_impl, void *map_inst, const AZPackedValue *iter, AZValue64 *val)
{
	return map_impl->get_key (map_impl, map_inst, iter, val);
}
const AZCollectionImplementation *
az_map_get_keys (const AZMapImplementation *map_impl, void *map_inst, void **inst)
{
	return map_impl->get_keys (map_impl, map_inst, inst);
}

const AZImplementation *
az_map_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue64 *val)
{
	return map_impl->lookup (map_impl, map_inst, key_impl, key_inst, val);
}
