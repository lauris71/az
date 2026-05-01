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
