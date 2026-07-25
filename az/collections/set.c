#define __AZ_SET_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdlib.h>

#include <az/base.h>
#include <az/types.h>

#include "set.h"

static unsigned int set_type = 0;
static AZSetClass *set_class;

unsigned int
az_set_get_type (void)
{
	unsigned int t = AZ_TYPE_READ(set_type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!set_type) {
		set_class = (AZSetClass *) az_register_interface_type (&set_type, (const unsigned char *) "AZSet", AZ_TYPE_COLLECTION,
			sizeof(AZSetClass), sizeof(AZSetImplementation), sizeof(AZSet), AZ_FLAG_ABSTRACT,
			0, 0,
			NULL, NULL, NULL, NULL);
	}
	t = set_type;
	AZ_TYPES_UNLOCK();
	return t;
}
