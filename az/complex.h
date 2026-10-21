#ifndef __AZ_COMPLEX_H__
#define __AZ_COMPLEX_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2018
*/

typedef struct _AZComplexFloat AZComplexFloat;
typedef struct _AZComplexDouble AZComplexDouble;

#include <math.h>

#include <arikkei/arikkei-utils.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _AZComplexFloat {
	union {
		struct {
			float r, i;
		};
		float c[2];
	};
};

struct _AZComplexDouble {
	union {
		struct {
			double r, i;
		};
		double c[2];
	};
};

/*
* Complex infinity is a single value - +inf and -inf are not distinguished.
* Whenever either component is infinite the value is canonicalized to
* (inf, 0), so infinities have one bit-identical representation.
* NaNs take precedence: a value with any NaN component is left untouched.
*/

ARIKKEI_INLINE AZComplexFloat *
az_complexf_canonicalize (AZComplexFloat *d)
{
	if (!isnan (d->r) && !isnan (d->i) && (isinf (d->r) || isinf (d->i))) {
		d->r = INFINITY;
		d->i = 0.0f;
	}
	return d;
}

/* Both components finite */
ARIKKEI_INLINE int
az_complexf_isfinite (const AZComplexFloat *d)
{
	return isfinite (d->r) && isfinite (d->i);
}

/* Exactly the canonical infinity (inf, 0) */
ARIKKEI_INLINE int
az_complexf_isinf (const AZComplexFloat *d)
{
	return isinf (d->r) && (d->i == 0.0f);
}

/* Any other non-finite value, including unnormalized infinities */
ARIKKEI_INLINE int
az_complexf_isnan (const AZComplexFloat *d)
{
	return !az_complexf_isfinite (d) && !az_complexf_isinf (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_canonicalize (AZComplexDouble *d)
{
	if (!isnan (d->r) && !isnan (d->i) && (isinf (d->r) || isinf (d->i))) {
		d->r = INFINITY;
		d->i = 0.0;
	}
	return d;
}

/* Both components finite */
ARIKKEI_INLINE int
az_complexd_isfinite (const AZComplexDouble *d)
{
	return isfinite (d->r) && isfinite (d->i);
}

/* Exactly the canonical infinity (inf, 0) */
ARIKKEI_INLINE int
az_complexd_isinf (const AZComplexDouble *d)
{
	return isinf (d->r) && (d->i == 0.0);
}

/* Any other non-finite value, including unnormalized infinities */
ARIKKEI_INLINE int
az_complexd_isnan (const AZComplexDouble *d)
{
	return !az_complexd_isfinite (d) && !az_complexd_isinf (d);
}

/* fixme: Implement value-based methods that can be #defined to complex operators */

ARIKKEI_INLINE AZComplexFloat *
az_complexf_set_ri (AZComplexFloat *d, float r, float i)
{
	d->r = r;
	d->i = i;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_conj (AZComplexFloat *d, const AZComplexFloat *v)
{
	d->r = v->r;
	d->i = -v->i;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_add (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r + rhs->r;
	d->i = lhs->i + rhs->i;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_sub (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r - rhs->r;
	d->i = lhs->i - rhs->i;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r * rhs->r - lhs->i * rhs->i;
	d->i = lhs->r * rhs->i + lhs->i * rhs->r;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul_r (AZComplexFloat* d, const AZComplexFloat* lhs, float rhs)
{
	d->r = lhs->r * rhs;
	d->i = lhs->i * rhs;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul_ri (AZComplexFloat* d, const AZComplexFloat* lhs, float r, float i)
{
	d->r = lhs->r * r - lhs->i * i;
	d->i = lhs->r * i + lhs->i * r;
	return az_complexf_canonicalize (d);
}

ARIKKEI_INLINE AZComplexFloat *
az_complexf_div (AZComplexFloat *d, const AZComplexFloat *lhs, const AZComplexFloat *rhs)
{
	d->r = (lhs->r * rhs->r + lhs->i * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	d->i = (lhs->i * rhs->r - lhs->r * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	return az_complexf_canonicalize (d);
}

/* Complex double */

ARIKKEI_INLINE AZComplexDouble *
az_complexd_set_ri (AZComplexDouble *d, double r, double i)
{
	d->r = r;
	d->i = i;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_conj (AZComplexDouble *d, const AZComplexDouble *v)
{
	d->r = v->r;
	d->i = -v->i;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_add (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r + rhs->r;
	d->i = lhs->i + rhs->i;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_sub (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r - rhs->r;
	d->i = lhs->i - rhs->i;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r * rhs->r - lhs->i * rhs->i;
	d->i = lhs->r * rhs->i + lhs->i * rhs->r;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul_r (AZComplexDouble *d, const AZComplexDouble *lhs, double rhs)
{
	d->r = lhs->r * rhs;
	d->i = lhs->i * rhs;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul_ri (AZComplexDouble *d, const AZComplexDouble *lhs, double r, double i)
{
	d->r = lhs->r * r - lhs->i * i;
	d->i = lhs->r * i + lhs->i * r;
	return az_complexd_canonicalize (d);
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_div (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = (lhs->r * rhs->r + lhs->i * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	d->i = (lhs->i * rhs->r - lhs->r * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	return az_complexd_canonicalize (d);
}

#ifdef __cplusplus
};
#endif

#endif
