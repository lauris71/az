#ifndef __AZ_COMPLEX_H__
#define __AZ_COMPLEX_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2018
*/

typedef struct _AZComplexFloat AZComplexFloat;
typedef struct _AZComplexDouble AZComplexDouble;

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

/* fixme: Implement value-based methods that can be #defined to complex operators */

ARIKKEI_INLINE AZComplexFloat *
az_complexf_set_ri (AZComplexFloat* d, float r, float i)
{
	d->r = r;
	d->i = i;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_conj (AZComplexFloat *d, const AZComplexFloat *v)
{
	d->r = v->r;
	d->i = -v->i;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_add (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r + rhs->r;
	d->i = lhs->i + rhs->i;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_sub (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r - rhs->r;
	d->i = lhs->i - rhs->i;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul (AZComplexFloat* d, const AZComplexFloat* lhs, const AZComplexFloat* rhs)
{
	d->r = lhs->r * rhs->r - lhs->i * rhs->i;
	d->i = lhs->r * rhs->i + lhs->i * rhs->r;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul_r (AZComplexFloat* d, const AZComplexFloat* lhs, float rhs)
{
	d->r = lhs->r * rhs;
	d->i = lhs->i * rhs;
	return d;
}

ARIKKEI_INLINE AZComplexFloat*
az_complexf_mul_ri (AZComplexFloat* d, const AZComplexFloat* lhs, float r, float i)
{
	d->r = lhs->r * r - lhs->i * i;
	d->i = lhs->r * i + lhs->i * r;
	return d;
}

ARIKKEI_INLINE AZComplexFloat *
az_complexf_div (AZComplexFloat *d, const AZComplexFloat *lhs, const AZComplexFloat *rhs)
{
	d->r = (lhs->r * rhs->r + lhs->i * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	d->i = (lhs->i * rhs->r - lhs->r * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	return d;
}

/* Complex double */

ARIKKEI_INLINE AZComplexDouble *
az_complexd_set_ri (AZComplexDouble *d, double r, double i)
{
	d->r = r;
	d->i = i;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_conj (AZComplexDouble *d, const AZComplexDouble *v)
{
	d->r = v->r;
	d->i = -v->i;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_add (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r + rhs->r;
	d->i = lhs->i + rhs->i;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_sub (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r - rhs->r;
	d->i = lhs->i - rhs->i;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = lhs->r * rhs->r - lhs->i * rhs->i;
	d->i = lhs->r * rhs->i + lhs->i * rhs->r;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul_r (AZComplexDouble *d, const AZComplexDouble *lhs, double rhs)
{
	d->r = lhs->r * rhs;
	d->i = lhs->i * rhs;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_mul_ri (AZComplexDouble *d, const AZComplexDouble *lhs, double r, double i)
{
	d->r = lhs->r * r - lhs->i * i;
	d->i = lhs->r * i + lhs->i * r;
	return d;
}

ARIKKEI_INLINE AZComplexDouble *
az_complexd_div (AZComplexDouble *d, const AZComplexDouble *lhs, const AZComplexDouble *rhs)
{
	d->r = (lhs->r * rhs->r + lhs->i * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	d->i = (lhs->i * rhs->r - lhs->r * rhs->i) / (rhs->r * rhs->r + rhs->i * rhs->i);
	return d;
}

#ifdef __cplusplus
};
#endif

#endif

