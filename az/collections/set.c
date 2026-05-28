#define __AZ_SET_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <stdlib.h>

#include <az/base.h>

#include "set.h"

static unsigned int set_type = 0;
static AZSetClass *set_class;

unsigned int
az_set_get_type (void)
{
	if (!set_type) {
		set_class = (AZSetClass *) az_register_interface_type (&set_type, (const unsigned char *) "AZSet", AZ_TYPE_COLLECTION,
			sizeof(AZSetClass), sizeof(AZSetImplementation), sizeof(AZSet), AZ_FLAG_ABSTRACT,
			0, 0,
			NULL, NULL, NULL, NULL);
	}
	return set_type;
}
