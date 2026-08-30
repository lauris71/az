#define __AZ_CONVERT_C__

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

AZConversionType
az_type_get_conversion_to (unsigned int type, unsigned int to_type)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	// fixme: Disallowing 0 breaks azo
	arikkei_return_val_if_fail (!type || az_type_is_valid(type), AZ_CANNOT_CONVERT);
	arikkei_return_val_if_fail (az_type_is_valid(to_type), AZ_CANNOT_CONVERT);
#endif
	if (type == AZ_TYPE_NONE) {
		/* NONE can be converted to typed null (i.e. block type with null block pointer) */
		/* fixme: temporarily AUTO, should be EXPLICIT (has to be fixed in Aosora first) */
		return (AZ_TYPE_IS_BLOCK(to_type)) ? AZ_CONVERT_AUTO : AZ_CANNOT_CONVERT;
	}
	/* Everything can be converted to itself, it's supertype and to an interface implemented by it */
	if (az_type_is_assignable_to (type, to_type)) return AZ_CONVERT_AUTO;
	/* Numeric conversions follow the conversion table */
	if (AZ_TYPE_IS_ARITHMETIC(type) && AZ_TYPE_IS_ARITHMETIC(to_type)) {
		return (AZConversionType) az_primitive_can_convert (to_type, type);
	}
	if ((type == AZ_TYPE_UINT64) && (to_type == AZ_TYPE_POINTER)) return AZ_CONVERT_EXPLICIT;
	if ((type == AZ_TYPE_POINTER) && (to_type == AZ_TYPE_UINT64)) return AZ_CONVERT_EXPLICIT;
	if (AZ_TYPE_IS_BLOCK(type) && (to_type == AZ_TYPE_POINTER)) return AZ_CONVERT_EXPLICIT;
	return AZ_CANNOT_CONVERT;
}
