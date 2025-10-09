#define __AZ_PRIMITIVES_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <az/class.h>
#include <az/field.h>
#include <az/private.h>
#include <az/serialization.h>
#include <az/value.h>
#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-utils.h>

#include <az/primitives.h>

/* 0 None */

/* 1 Any */

static unsigned int
any_to_string (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen)
{
	if (AZ_IMPL_TYPE(impl) == AZ_TYPE_ANY) {
		/* Pure Any */
		return arikkei_strncpy (d, dlen, (const unsigned char *) "Any");
	} else {
		/* Subclass that does not implement to_string */
		char c[32];
		unsigned int pos;
		AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
		pos = arikkei_memcpy_str (d, dlen, (const unsigned char *) "Instance of ");
		pos += arikkei_memcpy_str (d + pos, (dlen > pos) ? dlen - pos : 0, klass->name);
		pos += arikkei_memcpy_str (d + pos, (dlen > pos) ? dlen - pos : 0, (const unsigned char *) " (");
		sprintf (c, "%p", instance);
		pos += arikkei_memcpy_str (d + pos, (dlen > pos) ? dlen - pos : 0, (const unsigned char *) c);
		pos += arikkei_strncpy (d + pos, (dlen > pos) ? dlen - pos : 0, (const unsigned char *) ")");
		return pos;
	}
}

/* 2 Boolean */

static unsigned int
serialize_boolean (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	unsigned char v = (*((unsigned int *) inst) != 0);
	return az_serialize_int (d, dlen, &v, 1);
}

static unsigned int
deserialize_boolean (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	if (!slen) return 0;
        value->uint32_v = *s;
	return 1;
}

static unsigned int
boolean_to_string (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen)
{
	return arikkei_strncpy (d, dlen, (*((unsigned int *) instance)) ? (const unsigned char *) "True" : (const unsigned char *) "False");
}

/* 3 Int8 */

static unsigned int
serialize_int (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	return az_serialize_int (d, dlen, inst, klass->instance_size);
}

static unsigned int
deserialize_int (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	return az_deserialize_int (value, klass->instance_size, s, slen);
}

static unsigned int
copy_int_to_buffer (unsigned char *d, unsigned int dlen, unsigned long long value, unsigned int sign)
{
	unsigned char c[32];
	unsigned int clen = 0, len = 0;
	c[clen++] = '0' + value % 10;
	value /= 10;
	while (value) {
		c[clen++] = '0' + value % 10;
		value /= 10;
	}
	if (sign) {
		if (d && (len < dlen)) d[len++] = '-';
	}
	if (d) {
		unsigned int i;
		for (i = 0; i < clen; i++) {
			if (len < dlen) d[len++] = c[clen - 1 - i];
		}
		if (len < dlen) d[len++] = 0;
	}
	return clen + sign + 1;
}

static unsigned int
int_to_string_any (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen)
{
	AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
	unsigned int size = klass->instance_size;
	unsigned int is_signed = ((klass->impl._type & 1) != 0);
	unsigned long long value = 0;
	unsigned int sign = 0;
	if (size == 1) {
		value = *((unsigned char *) instance);
	} else if (size == 2) {
		value = *((unsigned short *) instance);
	}
	memcpy (&value, instance, size);
	if (is_signed) {
		if (value & (1ULL << ((8 * size) - 1))) {
			sign = 1;
			value = (1ULL << (8 * size)) - value;
		}
	}
	return copy_int_to_buffer (d, dlen, value, sign);
}

/* 4 Uint8 */

/* 5 Int16 */

/* 6 Uint16 */

/* 7 Int32 */

/* 8 Uint32 */

/* 9 Int64 */

/* 10 Uint64 */

/* 11 Float */

static unsigned int
float_to_string (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen)
{
	unsigned char c[32];
	unsigned int clen = arikkei_dtoa_exp (c, 32, *((float *) instance), 5, -5, 5);
	if (d) {
		memcpy (d, c, (clen <= dlen) ? clen : dlen);
		if (clen < dlen) d[clen] = 0;
	}
	return clen + 1;
}

/* 12 Double */

static unsigned int
double_to_string (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen)
{
	unsigned char c[32];
	unsigned int clen = arikkei_dtoa_exp (c, 32, *((double *) instance), 8, -5, 5);
	if (d) {
		memcpy (d, c, (clen <= dlen) ? clen : dlen);
		if (clen < dlen) d[clen] = 0;
	}
	return clen + 1;
}

/* 13 Complex float */

static unsigned int
complex_float_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	float *v = (float *) inst;
	unsigned char c[64];
	unsigned int clen = arikkei_dtoa_exp (c, 32, v[0], 5, -5, 5);
	if (v[1] >= 0) c[clen++] = '+';
	clen += arikkei_dtoa_exp (c, 32, v[1], 5, -5, 5);
	c[clen++] = 'i';
	if (buf) {
		memcpy (buf, c, clen);
		if (clen < len) buf[clen] = 0;
	}
	return clen + 1;
}

static unsigned int
serialize_complex_float (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	if (d && (dlen >= 8)) {
		az_serialize_int (d, dlen, inst, 4);
		az_serialize_int (d + 4, dlen - 4, (float *) inst + 1, 4);
	}
	return 8;
}

static unsigned int
deserialize_complex_float (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	if (slen < 8) return 0;
	az_deserialize_int (value, 4, s, slen);
	az_deserialize_int ((float *) value + 1, 4, s + 4, slen - 4);
	return 8;
}

/* 14 Complex double */

static unsigned int
complex_double_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	double *v = (double *) inst;
	unsigned char c[64];
	unsigned int clen = arikkei_dtoa_exp (c, 32, v[0], 8, -5, 5);
	if (v[1] >= 0) c[clen++] = '+';
	clen += arikkei_dtoa_exp (&c[clen], 32, v[1], 8, -5, 5);
	c[clen++] = 'i';
	if (buf) {
		memcpy (buf, c, clen);
		if (clen < len) buf[clen] = 0;
	}
	return clen + 1;
}

static unsigned int
serialize_complex_double (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	if (d && (dlen >= 16)) {
		az_serialize_int (d, dlen, inst, 8);
		az_serialize_int (d + 8, dlen - 8, (double *) inst + 1, 8);
	}
	return 16;
}

static unsigned int
deserialize_complex_double (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	if (slen < 16) return 0;
	az_deserialize_int (value, 8, s, slen);
	az_deserialize_int ((double *) value + 1, 8, s + 8, slen - 8);
	return 16;
}

/* 15 Pointer */

static unsigned int
pointer_to_string (const AZImplementation* impl, void *instance, unsigned char *buf, unsigned int len)
{
	unsigned int i;
	const char *t;
	char b[32];
	if (instance) {
		static const char *c = "0123456789abcdef";
		unsigned long long v = (unsigned long long) instance;
		unsigned int l = 2 * sizeof (void *);
		for (i = 0; i < l; i++) {
			b[l - 1 - i] = c[v & 0xf];
			v = v >> 4;
		}
		b[i] = 0;
		t = b;
	} else {
		t = "null";
	}
	return arikkei_strncpy (buf, len, (const unsigned char *) t);
}

struct _PrimitiveDef {
	unsigned int type;
	unsigned int parent;
	unsigned int is_abstract;
	unsigned int is_final;
	unsigned int is_value;
	unsigned int instance_size;
	unsigned int alignment;
	const char *name;
	unsigned int (*serialize) (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
	unsigned int (*deserialize) (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);
	unsigned int (*to_string) (const AZImplementation* impl, void *instance, unsigned char *buf, unsigned int len);
};

static unsigned int
any_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	*prop_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_CLASS);
	prop_val->block = AZ_CLASS_FROM_IMPL(impl);
	return 1;
}

static unsigned char zero_val[16] = { 0 };

static AZClass AnyClass = {
	{AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_ANY},
	NULL,
	/* Interfaces */
	0, 0, {0},
	/* Properties */
	0, NULL,
	(const uint8_t *) "any",
	0, sizeof(AZClass), 0,
	/* Default value */
	NULL,
	/* Allocator */
	NULL,
	NULL, NULL, NULL,
	NULL, NULL, any_to_string,
	any_get_property, NULL
};

unsigned int AnyType[] = { AZ_TYPE_ANY };

static AZClass primitive_classes[] = {
	{
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BOOLEAN},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "boolean",
		3, sizeof(AZClass), 4,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_boolean, deserialize_boolean, boolean_to_string,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT8},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "int8",
		0, sizeof(AZClass), 1,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT8},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "uint8",
		0, sizeof(AZClass), 1,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT16},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "int16",
		1, sizeof(AZClass), 2,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT16},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "uint16",
		1, sizeof(AZClass), 2,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT32},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "int32",
		3, sizeof(AZClass), 4,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT32},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "uint32",
		3, sizeof(AZClass), 4,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT64},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "int64",
		7, sizeof(AZClass), 8,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT64},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "uint64",
		7, sizeof(AZClass), 8,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, int_to_string_any,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_FLOAT},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "float",
		3, sizeof(AZClass), 4,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, float_to_string,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_DOUBLE},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "double",
		7, sizeof(AZClass), 8,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_int, deserialize_int, double_to_string,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_COMPLEX_FLOAT},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "complex float",
		7, sizeof(AZClass), 8,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_complex_float, deserialize_complex_float, complex_float_to_string,
		NULL, NULL
	}, {
		{AZ_FLAG_VALUE | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_COMPLEX_DOUBLE},
		&AnyClass,
		0, 0, {0},
		0, NULL,
		(const uint8_t *) "complex double",
		7, sizeof(AZClass), 16,
		&zero_val,
		NULL,
		NULL, NULL, NULL,
		serialize_complex_double, deserialize_complex_double, complex_double_to_string,
		NULL, NULL
	}
};

struct _PrimitiveDef defs[] = {
	{ AZ_TYPE_NONE, AZ_TYPE_NONE, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL },
	{ AZ_TYPE_ANY, AZ_TYPE_NONE, 1, 0, 0, 0, 0, "any", NULL, NULL, any_to_string },
	{ AZ_TYPE_BOOLEAN, AZ_TYPE_ANY, 0, 1, 1, 4, 3, "boolean", serialize_boolean, deserialize_boolean, boolean_to_string },
	{ AZ_TYPE_INT8, AZ_TYPE_ANY, 0, 1, 1, 1, 0, "int8", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_UINT8, AZ_TYPE_ANY, 0, 1, 1, 1, 0, "uint8", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_INT16, AZ_TYPE_ANY, 0, 1, 1, 2, 1, "int16", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_UINT16, AZ_TYPE_ANY, 0, 1, 1, 2, 1, "uint16", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_INT32, AZ_TYPE_ANY, 0, 1, 1, 4, 3, "int32", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_UINT32, AZ_TYPE_ANY, 0, 1, 1, 4, 3, "uint32", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_INT64, AZ_TYPE_ANY, 0, 1, 1, 8, 7, "int64", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_UINT64, AZ_TYPE_ANY, 0, 1, 1, 8, 7, "uint64", serialize_int, deserialize_int, int_to_string_any },
	{ AZ_TYPE_FLOAT, AZ_TYPE_ANY, 0, 1, 1, 4, 3, "float", serialize_int, deserialize_int, float_to_string },
	{ AZ_TYPE_DOUBLE, AZ_TYPE_ANY, 0, 1, 1, 8, 7, "double", serialize_int, deserialize_int, double_to_string },
	{ AZ_TYPE_COMPLEX_FLOAT, AZ_TYPE_ANY, 0, 1, 1, 8, 7, "complex float", serialize_complex_float, deserialize_complex_float, complex_float_to_string },
	{ AZ_TYPE_COMPLEX_DOUBLE, AZ_TYPE_ANY, 0, 1, 1, 16, 7, "complex double", serialize_complex_double, deserialize_complex_double, complex_double_to_string },
	{ AZ_TYPE_POINTER, AZ_TYPE_ANY, 0, 0, 1, 8, 7, "pointer", serialize_int, deserialize_int, pointer_to_string },
	{ AZ_TYPE_STRUCT, AZ_TYPE_ANY, 1, 0, 1, 0, 3, "struct", NULL, NULL, NULL },
	{ AZ_TYPE_BLOCK, AZ_TYPE_ANY, 1, 0, 0, 0, 7, "block", NULL, NULL, NULL }
};


void
az_init_primitive_classes (void)
{
	unsigned int i;
	az_class_new_with_value(&AnyClass);
	for (i = AZ_TYPE_BOOLEAN; i <= AZ_TYPE_COMPLEX_DOUBLE; i++) {
		az_class_new_with_value(&primitive_classes[i - AZ_TYPE_BOOLEAN]);
	}
	for (i = AZ_TYPE_POINTER; i <= AZ_TYPE_BLOCK; i++) {
		unsigned int flags = 0;
		assert (defs[i].type == i);
		if (defs[i].is_abstract) flags |= AZ_FLAG_ABSTRACT;
		if (defs[i].is_final) flags |= AZ_FLAG_FINAL;
		if (defs[i].is_value) flags |= AZ_FLAG_VALUE;
		if (i == AZ_TYPE_BLOCK) flags |= AZ_FLAG_BLOCK;
		AZClass *klass = az_class_new_with_type (defs[i].type, defs[i].parent, sizeof (AZClass), defs[i].instance_size, flags, (const uint8_t *) defs[i].name);
		klass->alignment = defs[i].alignment;
		klass->serialize = defs[i].serialize;
		klass->deserialize = defs[i].deserialize;
		if (defs[i].to_string) klass->to_string = defs[i].to_string;
	}
}

void
az_post_init_primitive_classes (void)
{
	unsigned int i;
    /* Properties are set in post-init because property types have to be initialized first */
	AZClass *any_class = AZ_CLASS_FROM_TYPE(AZ_TYPE_ANY);
	az_class_set_num_properties (any_class, 2);
	az_class_define_property (any_class, 0, (const unsigned char *) "type", AZ_TYPE_UINT32, 1, AZ_FIELD_IMPLEMENTATION, AZ_FIELD_READ_METHOD, AZ_FIELD_WRITE_NONE, 0, NULL, NULL);
	az_class_define_property (any_class, 1, (const unsigned char *) "class", AZ_TYPE_CLASS, 1, AZ_FIELD_IMPLEMENTATION, AZ_FIELD_READ_METHOD, AZ_FIELD_WRITE_NONE, 0, NULL, NULL);
	for (i = AZ_TYPE_ANY; i <= AZ_TYPE_BLOCK; i++) {
		az_class_post_init (AZ_CLASS_FROM_TYPE(i));
	}
}

#define A 0
#define C 1
#define E 2
#define N 3

const unsigned char az_primitive_conversion_table[] = {
	/* From none */
	N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N,
	/* From any */
	N, A, N, N, N, N, N, N, N, N, N, N, N, N, N, N,
	/* Boolean */
	N, A, A, E, E, E, E, E, E, E, E, E, E, E, E, N,
	/* Int8 */
	N, A, E, A, C, A, C, A, C, A, C, A, A, A, A, N,
	/* UInt8 */
	N, A, E, C, A, A, A, A, A, A, A, A, A, A, A, N,
	/* Int16 */
	N, A, E, C, C, A, C, A, C, A, C, A, A, A, A, N,
	/* UInt16 */
	N, A, E, C, C, C, A, A, A, A, A, A, A, A, A, N,
	/* Int32 */
	N, A, E, C, C, C, C, A, C, A, C, C, A, C, A, N,
	/* UInt32 */
	N, A, E, C, C, C, C, C, A, A, A, C, A, C, A, N,
	/* Int64 */
	N, A, E, C, C, C, C, C, C, A, C, C, C, C, C, N,
	/* UInt64 */
	N, A, E, C, C, C, C, C, C, C, A, C, C, C, C, N,
	/* Float */
	N, A, E, C, C, C, C, C, C, C, C, A, A, A, A, N,
	/* Double */
	N, A, E, C, C, C, C, C, C, C, C, C, A, C, A, N,
	/* Complex float */
	N, A, E, C, C, C, C, C, C, C, C, C, C, A, A, N,
	/* Complex double */
	N, A, E, C, C, C, C, C, C, C, C, C, C, C, A, N,
	/* Pointer */
	N, A, E, E, E, E, E, E, E, E, E, E, E, E, E, A
};

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

static unsigned int
convert_float (unsigned int to_type, void *to_val, float val)
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
		*((char *) to_val) = (char) val;
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
		*((unsigned char *) to_val) = (unsigned char) val;
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
		*((short *) to_val) = (short) val;
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
		*((unsigned short *) to_val) = (unsigned short) val;
	} else if (to_type == AZ_TYPE_INT32) {
		if (val < INT32_MIN) {
			val = INT32_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT32_MAX) {
			val = (float) INT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((int *) to_val) = (int) val;
	} else if (to_type == AZ_TYPE_UINT32) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT32_MAX) {
			val = (float) UINT32_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((unsigned int *) to_val) = (unsigned int) val;
	} else if (to_type == AZ_TYPE_INT64) {
		if (val < INT64_MIN) {
			val = (float) INT64_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT64_MAX) {
			val = (float) INT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((int *) to_val) = (int) val;
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT64_MAX) {
			val = (float) UINT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floorf (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((unsigned long long *) to_val) = (unsigned long long) val;
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
convert_double (unsigned int to_type, void *to_val, double val)
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
		*((char *) to_val) = (char) val;
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
		*((unsigned char *) to_val) = (unsigned char) val;
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
		*((short *) to_val) = (short) val;
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
		*((unsigned short *) to_val) = (unsigned short) val;
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
		*((int *) to_val) = (int) val;
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
		*((unsigned int *) to_val) = (unsigned int) val;
	} else if (to_type == AZ_TYPE_INT64) {
		if (val < INT64_MIN) {
			val = (double) INT64_MIN;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > INT64_MAX) {
			val = (double) INT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((int *) to_val) = (int) val;
	} else if (to_type == AZ_TYPE_UINT64) {
		if (val < 0) {
			val = 0;
			result = AZ_CONVERSION_CLAMPED;
		} else if (val > UINT64_MAX) {
			val = (double) UINT64_MAX;
			result = AZ_CONVERSION_CLAMPED;
		} else if (floor (val) != val) {
			result = AZ_CONVERSION_ROUNDED;
		}
		*((unsigned long long *) to_val) = (unsigned long long) val;
	} else if (to_type == AZ_TYPE_FLOAT) {
		/* fixme: Check ranges and precision */
		*((float *) to_val) = (float) val;
	} else if (to_type == AZ_TYPE_DOUBLE) {
		*((double *) to_val) = (double) val;
	} else if (to_type == AZ_TYPE_COMPLEX_FLOAT) {
		/* fixme: Check ranges and precision */
		*((float *) to_val) = (float) val;
		*((float *) to_val + 1) = 0;
	} else if (to_type == AZ_TYPE_COMPLEX_DOUBLE) {
		*((double *) to_val) = (double) val;
		*((double *) to_val + 1) = 0;
	}
	return result;
}

unsigned int
az_convert_arithmetic_type (unsigned int to_type, AZValue *to_val, unsigned int from_type, const AZValue *from_val)
{
	unsigned int result = 0;
	arikkei_return_val_if_fail (AZ_TYPE_IS_PRIMITIVE (to_type), 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_PRIMITIVE (from_type), 0);
	if (to_type == from_type) {
		if (to_val != from_val) {
			AZClass *klass = az_type_get_class (from_type);
			memcpy (to_val, from_val, klass->instance_size);
		}
		return 0;
	}
	/* Booleans and pointers do not have automatic conversions */
	arikkei_return_val_if_fail (AZ_TYPE_IS_ARITHMETIC (to_type), 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_ARITHMETIC (from_type), 0);

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
		if (to_type < AZ_TYPE_COMPLEX_FLOAT) {
			result = convert_float (to_type, to_val, r);
			if (i != 0) result = AZ_CONVERSION_CLAMPED;
		} else {
			*((double *) to_val) = (double) r;
			*((double *) to_val + 1) = (double) i;
		}
		return result;
	} else if (from_type == AZ_TYPE_COMPLEX_DOUBLE) {
		/* C, C, C, C, C, C, C, C, C, C, C, A */
		double r = *((double *) from_val);
		double i = *((double *) from_val + 1);
		if (to_type < AZ_TYPE_COMPLEX_FLOAT) {
			result = convert_double (to_type, to_val, r);
			if (i != 0) result = AZ_CONVERSION_CLAMPED;
		} else {
			/* fixme: Check ranges and precision */
			*((float *) to_val) = (float) r;
			*((float *) to_val + 1) = (float) i;
		}
		return result;
	}
	return 0;
}


