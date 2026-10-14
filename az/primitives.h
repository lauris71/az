#ifndef __AZ_PRIMITIVES_H__
#define __AZ_PRIMITIVES_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2017
*/

#include <az/types.h>
#include <az/convert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Conversion table, from (rows), to (columns), (AZ_TYPE_POINTER + 1) rows/cols */
extern const unsigned char az_primitive_conversion_table[];

ARIKKEI_INLINE
AZConversionType az_primitive_can_convert (unsigned int to, unsigned int from)
{
	return az_primitive_conversion_table[AZ_TYPE_INDEX(from) * (AZ_TYPE_IDX_POINTER + 1) + AZ_TYPE_INDEX(to)];
}

#ifdef __cplusplus
};
#endif

#endif
