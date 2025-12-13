#ifndef __AZ_PRIMITIVES_H__
#define __AZ_PRIMITIVES_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2017
*/

#include <az/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AZ_CONVERT_AUTO 0
#define AZ_CONVERT_CONDITIONAL 1
#define AZ_CONVERT_EXPLICIT 2
#define AZ_CANNOT_CONVERT 3

/* Conversion table, from in rows, to in columns, (AZ_TYPE_POINTER + 1) rows/cols */
extern const unsigned char az_primitive_conversion_table[];

ARIKKEI_INLINE
unsigned int az_primitive_can_convert (unsigned int to, unsigned int from)
{
	return az_primitive_conversion_table[from * (AZ_TYPE_POINTER + 1) + to];
}

/* Conversion results */
#define AZ_CONVERSION_EXACT 0
/* Conversion resulted in rounding of value */
#define AZ_CONVERSION_ROUNDED 1
/* Conversion resulted in clamping of value */
#define AZ_CONVERSION_CLAMPED 2
#define AZ_CONVERSION_FAILED 3

/* Return status, type has to be arithmetic, safe to have to_val == from_val */
unsigned int az_convert_arithmetic_type (unsigned int to_type, AZValue *to_val, unsigned int from_type, const AZValue *from_val);

#ifdef __cplusplus
};
#endif

#endif
