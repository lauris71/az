#define __AZ_CONVERT_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdio.h>

#include <az/class.h>
#include <az/convert.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/types.h>

unsigned int
az_type_is_convertible_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	// fixme: Disallowing 0 breaks azo
	arikkei_return_val_if_fail (!type || az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(test), 0);
#endif
	if (az_type_is_assignable_to (type, test)) return 1;
	/* Only arithmetic types have autoconversion */
	if (AZ_TYPE_IS_ARITHMETIC(type) && AZ_TYPE_IS_ARITHMETIC(test)) {
		if (az_primitive_can_convert(test, type) == AZ_CONVERT_AUTO) return 1;
		fprintf (stderr, "Convert %s -> %s\n", AZ_CLASS_FROM_TYPE(type)->name, AZ_CLASS_FROM_TYPE(test)->name);
		return 1;
	}
	return 0;
}
