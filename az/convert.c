#define __AZ_CONVERT_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdio.h>
#include <math.h>

#include <az/class.h>
#include <az/convert.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/types.h>
#include <az/value.h>

static unsigned int
convert_int8 (unsigned int to_type, AZValue *to_val, int8_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* A, C, A, C, A, C, A, C, A, A, A, A */
	switch (to_type) {
	case AZ_TYPE_UINT8:
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		to_val->uint8_v = (uint8_t) val;
		break;
	case AZ_TYPE_INT16:
		to_val->int16_v = (int16_t) val;
		break;
	case AZ_TYPE_UINT16:
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		to_val->uint16_v = (uint16_t) val;
		break;
	case AZ_TYPE_INT32:
		to_val->int32_v = (int32_t) val;
		break;
	case AZ_TYPE_UINT32:
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		to_val->uint32_v = (uint32_t) val;
		break;
	case AZ_TYPE_INT64:
		to_val->int64_v = (int64_t) val;
		break;
	case AZ_TYPE_UINT64:
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		to_val->uint64_v = (uint64_t) val;
		break;
	case AZ_TYPE_FLOAT:
		to_val->float_v = (float) val;
		break;
	case AZ_TYPE_DOUBLE:
		to_val->double_v = (double) val;
		break;
	case AZ_TYPE_COMPLEX_FLOAT:
		to_val->cfloat_v.r = (float) val;
		to_val->cfloat_v.i = 0;
		break;
	case AZ_TYPE_COMPLEX_DOUBLE:
		to_val->cdouble_v.r = (double) val;
		to_val->cdouble_v.i = 0;
		break;
	}
	return result;
}

static unsigned int
convert_uint8 (unsigned int to_type, void *to_val, uint8_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, A, A, A, A, A, A, A, A, A, A, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_INT16) {
		*((short *) to_val) = (short) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		*((unsigned short *) to_val) = (unsigned short) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		*((int *) to_val) = (int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		*((long long *) to_val) = (long long) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		*((unsigned long long *) to_val) = (unsigned long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}
static unsigned int
convert_int16 (unsigned int to_type, void *to_val, int16_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, C, A, C, A, C, A, C, A, A, A, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val < INT8_MIN) {
			val = INT8_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned char *) to_val) = (unsigned char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned short *) to_val) = (unsigned short) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		*((int *) to_val) = (int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		*((long long *) to_val) = (long long) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned long long *) to_val) = (unsigned long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

static unsigned int
convert_uint16 (unsigned int to_type, void *to_val, uint16_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, C, C, A, A, A, A, A, A, A, A, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned char *) to_val) = (unsigned char) val;
		return result;
	} else if (to_type == AZ_TYPE_INT16) {
		if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((short *) to_val) = (short) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		*((int *) to_val) = (int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		*((long long *) to_val) = (long long) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		*((unsigned long long *) to_val) = (unsigned long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

static unsigned int
convert_int32 (unsigned int to_type, void *to_val, int32_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, C, C, C, A, C, A, C, C, A, C, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val < INT8_MIN) {
			val = INT8_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned char *) to_val) = (unsigned char) val;
		return result;
	} if (to_type == AZ_TYPE_INT16) {
		if (val < INT16_MIN) {
			val = INT16_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((short *) to_val) = (short) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned short *) to_val) = (unsigned short) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		*((long long *) to_val) = (long long) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned long long *) to_val) = (unsigned long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		if ((val > (1L << 24)) || (val > -(1L << 24))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		if ((val > (1L << 24)) || (val > -(1L << 24))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

static unsigned int
convert_uint32 (unsigned int to_type, void *to_val, uint32_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, C, C, C, C, A, A, A, C, A, C, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((int8_t *) to_val) = (int8_t) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((uint8_t *) to_val) = (uint8_t) val;
		return result;
	} else if (to_type == AZ_TYPE_INT16) {
		if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((int16_t *) to_val) = (int16_t) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((uint16_t *) to_val) = (uint16_t) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val > INT32_MAX) {
			val = INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((int32_t *) to_val) = (int32_t) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		*((int64_t *) to_val) = (int64_t) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		*((uint64_t *) to_val) = (uint64_t) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		if (val > (1UL << 24)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		if (val > (1UL << 24)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

static unsigned int
convert_int64 (unsigned int to_type, void *to_val, int64_t val)
{
	unsigned int result = AZ_CONVERSION_EXACT;
	/* C, C, C, C, C, C, A, C, C, C, C, C */
	if (to_type == AZ_TYPE_INT8) {
		if (val < INT8_MIN) {
			val = INT8_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned char *) to_val) = (unsigned char) val;
		return result;
	} if (to_type == AZ_TYPE_INT16) {
		if (val < INT16_MIN) {
			val = INT16_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((short *) to_val) = (short) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned short *) to_val) = (unsigned short) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val < INT32_MIN) {
			val = INT32_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT32_MAX) {
			val = INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((int *) to_val) = (int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT32_MAX) {
			val = UINT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned long long *) to_val) = (unsigned long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		if ((val > (1LL << 24)) || (val < -(1LL << 24))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		if ((val > (1LL << 53)) || (val < -(1LL << 53))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		if ((val > (1LL << 24)) || (val < -(1LL << 24))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		if ((val > (1LL << 53)) || (val < -(1LL << 53))) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

static unsigned int
convert_uint64 (unsigned int to_type, void *to_val, uint64_t val)
{
	unsigned int result = 0;
	/* C, C, C, C, C, C, C, A, C, C, C, C */
	if (to_type == AZ_TYPE_INT8) {
		if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((char *) to_val) = (char) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned char *) to_val) = (unsigned char) val;
		return result;
	} else if (to_type == AZ_TYPE_INT16) {
		if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((short *) to_val) = (short) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned short *) to_val) = (unsigned short) val;
		return result;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val > INT32_MAX) {
			val = INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((int *) to_val) = (int) val;
		return result;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val > UINT32_MAX) {
			val = UINT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((unsigned int *) to_val) = (unsigned int) val;
		return result;
	} else if (to_type == AZ_TYPE_INT64) {
		if (val > INT64_MAX) {
			val = INT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		}
		*((long long *) to_val) = (long long) val;
		return result;
	} else if (to_type == AZ_TYPE_FLOAT) {
		if (val > (1ULL << 24)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		if (val > (1ULL << 53)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		if (val > (1ULL << 24)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		if (val > (1ULL << 53)) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

#define MIN_FLOAT_GT_I32_MAX 2147483648.0f
#define MIN_FLOAT_GT_U32_MAX 4294967296.0f

/* Float has 24-bit mantissa. ULP at 2^63 is 2^40 (1099511627776). */
#define MIN_FLOAT_GT_I64_MAX 9223372036854775808.0f     /* 2^63, (float)INT64_MAX rounds to this */
#define MIN_FLOAT_GT_U64_MAX 18446744073709551616.0f    /* 2^64, (float)UINT64_MAX rounds to this */

/* Double has 53-bit mantissa. ULP at 2^63 is 2^11 (2048). */
#define MIN_DBL_GT_I64_MAX 9223372036854775808.0      /* 2^63, (double)INT64_MAX rounds to this */
#define MIN_DBL_GT_U64_MAX 18446744073709551616.0     /* 2^64, (double)UINT64_MAX rounds to this */

static unsigned int
convert_float (unsigned int to_type, AZValue *to_val, float val)
{
	unsigned int result = 0;
	/* C, C, C, C, C, C, C, C, A, A, A, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val < INT8_MIN) {
			val = INT8_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->int8_v = (int8_t) val;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->uint8_v = (uint8_t) val;
	} if (to_type == AZ_TYPE_INT16) {
		if (val < INT16_MIN) {
			val = INT16_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->int16_v = (int16_t) val;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->uint16_v = (uint16_t) val;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val < INT32_MIN) {
			to_val->int32_v = INT32_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_FLOAT_GT_I32_MAX) {
			to_val->int32_v = INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->int32_v = (int32_t) val;
		} else {
			to_val->int32_v = (int32_t) val;
		}
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			to_val->uint32_v = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_FLOAT_GT_U32_MAX) {
			to_val->uint32_v = UINT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->uint32_v = (uint32_t) val;
		} else {
		to_val->uint32_v = (uint32_t) val;
		}
	} else if (to_type == AZ_TYPE_INT64) {
		if (val < INT64_MIN) {
			to_val->int64_v = INT64_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_FLOAT_GT_I64_MAX) {
			to_val->int64_v = INT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->int64_v = (int64_t) val;
		} else {
			to_val->int64_v = (int64_t) val;
		}
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			to_val->uint64_v = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_FLOAT_GT_U64_MAX) {
			to_val->uint64_v = UINT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->uint64_v = (uint64_t) val;
		} else {
			to_val->uint64_v = (uint64_t) val;
		}
	} else if (to_type == AZ_TYPE_DOUBLE) {
		to_val->double_v = val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		to_val->cfloat_v = (AZComplexFloat) {val, 0};
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		to_val->cdouble_v = (AZComplexDouble) {val, 0};
	}
	return result;
}

static unsigned int
convert_double (unsigned int to_type, AZValue *to_val, double val)
{
	unsigned int result = 0;
	/* C, C, C, C, C, C, C, C, C, A, C, A */
	if (to_type == AZ_TYPE_INT8) {
		if (val < INT8_MIN) {
			val = INT8_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT8_MAX) {
			val = INT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->int8_v = (int8_t) val;
	} else if (to_type == AZ_TYPE_UINT8) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT8_MAX) {
			val = UINT8_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->uint8_v = (uint8_t) val;
	} if (to_type == AZ_TYPE_INT16) {
		if (val < INT16_MIN) {
			val = INT16_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT16_MAX) {
			val = INT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->int16_v = (int16_t) val;
	} else if (to_type == AZ_TYPE_UINT16) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT16_MAX) {
			val = UINT16_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->uint16_v = (uint16_t) val;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val < INT32_MIN) {
			val = INT32_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT32_MAX) {
			val = INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->int32_v = (int32_t) val;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT32_MAX) {
			val = UINT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->uint32_v = (uint32_t) val;
	} else if (to_type == AZ_TYPE_INT64) {
		if (val < INT64_MIN) {
			to_val->int64_v = INT64_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_DBL_GT_I64_MAX) {
			to_val->int64_v = INT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->int64_v = (int64_t) val;
		} else {
			to_val->int64_v = (int64_t) val;
		}
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			to_val->uint64_v = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val >= MIN_DBL_GT_U64_MAX) {
			to_val->uint64_v = UINT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
			to_val->uint64_v = (uint64_t) val;
		} else {
			to_val->uint64_v = (uint64_t) val;
		}
	} else if (to_type == AZ_TYPE_FLOAT) {
		float fval = (float) val;
		if (isfinite (val) && !isfinite (fval)) {
			result = AZ_CONVERSION_CLAMPED;
		} else if (val != (double) fval) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->float_v = fval;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		to_val->double_v = val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		float fval = (float) val;
		if (isfinite (val) && !isfinite (fval)) {
			result = AZ_CONVERSION_CLAMPED;
		} else if (val != (double) fval) {
			result = AZ_CONVERSION_ROUNDED;
		}
		to_val->cfloat_v.r = fval;
		to_val->cfloat_v.i = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		to_val->cdouble_v.r = val;
		to_val->cdouble_v.i = 0;
	}
	return result;
}

AZConversionResult
az_convert_arithmetic_type (unsigned int to_type, AZValue *to_val, unsigned int from_type, const AZValue *from_val)
{
	unsigned int result = 0;
	arikkei_return_val_if_fail (AZ_TYPE_IS_PRIMITIVE (to_type), AZ_CONVERSION_FAILED);
	arikkei_return_val_if_fail (AZ_TYPE_IS_PRIMITIVE (from_type), AZ_CONVERSION_FAILED);
	// fixme: This has to be handled somewhere else
	if (to_type == from_type) {
		AZClass *klass = az_type_get_class (from_type);
		memcpy (to_val, from_val, klass->instance_size);
		return 0;
	}
	/* Booleans and pointers do not have automatic conversions */
	arikkei_return_val_if_fail (AZ_TYPE_IS_ARITHMETIC (to_type), AZ_CONVERSION_FAILED);
	arikkei_return_val_if_fail (AZ_TYPE_IS_ARITHMETIC (from_type), AZ_CONVERSION_FAILED);

	if (from_type == AZ_TYPE_INT8) {
		/* A, C, A, C, A, C, A, C, A, A, A, A */
		return convert_int8 (to_type, to_val, *((int8_t *) from_val));
	} else if (from_type == AZ_TYPE_UINT8) {
		/* C, A, A, A, A, A, A, A, A, A, A, A */
		return convert_uint8 (to_type, to_val, *((uint8_t *) from_val));
	} else if (from_type == AZ_TYPE_INT16) {
		/* C, C, A, C, A, C, A, C, A, A, A, A */
		return convert_int16 (to_type, to_val, *((int16_t *) from_val));
	} else if (from_type == AZ_TYPE_UINT16) {
		/* C, C, C, A, A, A, A, A, A, A, A, A */
		return convert_uint16 (to_type, to_val, *((uint16_t *) from_val));
	} else if (from_type == AZ_TYPE_INT32) {
		/* C, C, C, C, A, C, A, C, C, A, C, A */
		return convert_int32 (to_type, to_val, *((int32_t *) from_val));
	} else if (from_type == AZ_TYPE_UINT32) {
		/* C, C, C, C, C, A, A, A, C, A, C, A */
		return convert_uint32 (to_type, to_val, *((uint32_t *) from_val));
	} else if (from_type == AZ_TYPE_INT64) {
		/* C, C, C, C, C, C, A, C, C, C, C, C */
		return convert_int64 (to_type, to_val, *((int64_t *) from_val));
	} else if (from_type == AZ_TYPE_UINT64) {
		/* C, C, C, C, C, C, C, A, C, C, C, C */
		return convert_uint64 (to_type, to_val, *((uint64_t *) from_val));
	} else if (from_type == AZ_TYPE_FLOAT) {
		/* C, C, C, C, C, C, C, C, A, A, A, A */
		return convert_float (to_type, to_val, *((float *) from_val));
	} else if (from_type == AZ_TYPE_DOUBLE) {
		/* C, C, C, C, C, C, C, C, C, A, C, A */
		return convert_double (to_type, to_val, *((double *) from_val));
	} else if (from_type == AZ_TYPE_COMPLEX_FLOAT) {
		/* C, C, C, C, C, C, C, C, C, C, A, A */
		float r = *((float *) from_val);
		float i = *((float *) from_val + 1);
		if (AZ_TYPE_INDEX(to_type) < AZ_TYPE_IDX_COMPLEX_FLOAT) {
			result = convert_float (to_type, to_val, r);
			if (i != 0) result = AZ_CONVERSION_CLAMPED;
		} else {
			*((double *) to_val) = (double) r;
			*((double *) to_val + 1) = (double) i;
		}
		return result;
	} else if (from_type == AZ_TYPE_COMPLEX_DOUBLE) {
		/* C, C, C, C, C, C, C, C, C, C, C, A */
		if (AZ_TYPE_INDEX(to_type) < AZ_TYPE_IDX_COMPLEX_FLOAT) {
			result = convert_double (to_type, to_val, from_val->cdouble_v.r);
			if (from_val->cdouble_v.i != 0) result = AZ_CONVERSION_CLAMPED;
		} else {
			for (unsigned int i = 0; i < 2; i++) {
				float fval = (float) from_val->cdouble_v.c[i];
				if (isfinite (from_val->cdouble_v.c[i]) && !isfinite (fval)) {
					result = AZ_CONVERSION_CLAMPED;
				} else if (from_val->cdouble_v.c[i] != (double) fval) {
					result = AZ_CONVERSION_ROUNDED;
				}
				to_val->cfloat_v.c[i] = fval;
			}
		}
		return result;
	}
	return AZ_CONVERSION_FAILED;
}

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
