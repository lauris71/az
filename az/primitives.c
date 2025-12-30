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

#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-utils.h>

#include <az/base.h>
#include <az/private.h>
#include <az/serialization.h>
#include <az/value.h>
#include <az/extend.h>

#include <az/primitives.h>

/* 0 None */

/* 1 Any */

unsigned int
az_any_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int d_len)
{
	if (AZ_IMPL_TYPE(impl) == AZ_TYPE_ANY) {
		/* Pure Any */
		return arikkei_strncpy (d, d_len, (const unsigned char *) "Any");
	} else {
		/* Subclass that does not implement to_string */
		char c[32];
		unsigned int pos;
		AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
		pos = arikkei_memcpy_str (d, d_len, (const unsigned char *) "Instance of ");
		pos += arikkei_memcpy_str (d + pos, (d_len > pos) ? d_len - pos : 0, klass->name);
		pos += arikkei_memcpy_str (d + pos, (d_len > pos) ? d_len - pos : 0, (const unsigned char *) " (");
		sprintf (c, "%p", inst);
		pos += arikkei_memcpy_str (d + pos, (d_len > pos) ? d_len - pos : 0, (const unsigned char *) c);
		pos += arikkei_strncpy (d + pos, (d_len > pos) ? d_len - pos : 0, (const unsigned char *) ")");
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
	unsigned int is_signed = ((impl->type & 1) != 0);
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

enum {
	ANY_PROP_TYPE,
	ANY_PROP_CLASS,
	ANY_PROP_ARITHMETIC,
	ANY_PROP_INTEGRAL,
	ANY_PROP_SIGNED,
	ANY_NUM_PROPS
};

static unsigned int
any_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	switch (idx) {
		case ANY_PROP_TYPE:
			*prop_impl = &AZUint32Klass.impl;
			prop_val->uint32_v = AZ_IMPL_TYPE(impl);
			break;
		case ANY_PROP_CLASS:
			*prop_impl = &AZClassKlass.impl;
			prop_val->block = AZ_CLASS_FROM_IMPL(impl);
			break;
		case ANY_PROP_ARITHMETIC:
			*prop_impl = &AZBooleanKlass.impl;
			prop_val->boolean_v = ((AZ_CLASS_FROM_IMPL(impl)->impl.flags & AZ_FLAG_ARITHMETIC) != 0);
			break;
		default:
			return 0;
	}
	return 1;
}

static unsigned char zero_val[16] = { 0 };

AZClass AZAnyKlass = {
	{AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_ANY},
	NULL,
	/* Num interfaces, num props, ifaces, props */
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "any",
	0, sizeof(AZClass), 0,
	/* Allocator */
	NULL,
	/* Instance init/finalize */
	NULL, NULL,
	/* Serialize/deserialize/to_string */
	NULL, NULL, az_any_to_string,
	any_get_property, NULL
};

//unsigned int AnyType[] = { AZ_TYPE_ANY };

AZClass AZBooleanKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_BOOLEAN},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "boolean",
	3, sizeof(AZClass), 4,
	NULL,
	NULL, NULL,
	serialize_boolean, deserialize_boolean, boolean_to_string,
	NULL, NULL
};

AZClass AZInt8Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT8},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "int8",
	0, sizeof(AZClass), 1,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZUint8Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT8},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "uint8",
	0, sizeof(AZClass), 1,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZInt16Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT16},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "int16",
	1, sizeof(AZClass), 2,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZUint16Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT16},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "uint16",
	1, sizeof(AZClass), 2,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZInt32Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT32},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "int32",
	3, sizeof(AZClass), 4,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZUint32Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT32},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "uint32",
	3, sizeof(AZClass), 4,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZInt64Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_INT64},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "int64",
	7, sizeof(AZClass), 8,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZUint64Klass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_INTEGRAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_UINT64},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "uint64",
	7, sizeof(AZClass), 8,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, int_to_string_any,
	NULL, NULL
};

AZClass AZFloatKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_FLOAT},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "float",
	3, sizeof(AZClass), 4,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, float_to_string,
	NULL, NULL
};

AZClass AZDoubleKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_DOUBLE},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "double",
	7, sizeof(AZClass), 8,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, double_to_string,
	NULL, NULL
};

AZClass AZComplexFloatKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_COMPLEX_FLOAT},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "complex float",
	7, sizeof(AZClass), 8,
	NULL,
	NULL, NULL,
	serialize_complex_float, deserialize_complex_float, complex_float_to_string,
	NULL, NULL
};

AZClass AZComplexDoubleKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_ARITHMETIC | AZ_FLAG_SIGNED | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_COMPLEX_DOUBLE},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "complex double",
	7, sizeof(AZClass), 16,
	NULL,
	NULL, NULL,
	serialize_complex_double, deserialize_complex_double, complex_double_to_string,
	NULL, NULL
};

AZClass AZPointerKlass = {
	{AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_POINTER},
	&AZAnyKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "pointer",
	7, sizeof(AZClass), 8,
	NULL,
	NULL, NULL,
	serialize_int, deserialize_int, pointer_to_string,
	NULL, NULL
};

static AZClass *primitive_classes[] = {
	&AZBooleanKlass,
	&AZInt8Klass,
	&AZUint8Klass,
	&AZInt16Klass,
	&AZUint16Klass,
	&AZInt32Klass,
	&AZUint32Klass,
	&AZInt64Klass,
	&AZUint64Klass,
	&AZFloatKlass,
	&AZDoubleKlass,
	&AZComplexFloatKlass,
	&AZComplexDoubleKlass,
	&AZPointerKlass
};

#define AZ_NUM_PRIMITIVE_CLASSES (sizeof(primitive_classes) / sizeof(primitive_classes[0]))

void
az_init_primitive_classes (void)
{
	unsigned int i;
	az_class_new_with_value(&AZAnyKlass);
	for (unsigned int i = 0; i < AZ_NUM_PRIMITIVE_CLASSES; i++) {
		az_class_new_with_value(primitive_classes[i]);
	}
}

void
az_post_init_primitive_classes (void)
{
	unsigned int i;
    /* Properties are set in post-init because property types have to be initialized first */
	AZClass *any_class = AZ_CLASS_FROM_TYPE(AZ_TYPE_ANY);
	az_class_set_num_properties (any_class, ANY_NUM_PROPS);
	az_class_define_property (any_class, ANY_PROP_TYPE, (const unsigned char *) "type", AZ_TYPE_UINT32, 1, AZ_FIELD_IMPLEMENTATION, AZ_FIELD_READ_METHOD, AZ_FIELD_WRITE_NONE, 0, NULL, NULL);
	az_class_define_property (any_class, ANY_PROP_CLASS, (const unsigned char *) "class", AZ_TYPE_CLASS, 1, AZ_FIELD_IMPLEMENTATION, AZ_FIELD_READ_METHOD, AZ_FIELD_WRITE_NONE, 0, NULL, NULL);
	az_class_define_property_value(any_class, ANY_PROP_ARITHMETIC, (const uint8_t *) "isArithmetic", AZ_TYPE_BOOLEAN, 1, AZ_FIELD_CLASS, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET(AZClass, impl.flags));
	any_class->props_self[ANY_PROP_ARITHMETIC].mask = AZ_FLAG_ARITHMETIC;
	az_class_define_property_value(any_class, ANY_PROP_INTEGRAL, (const uint8_t *) "isIntegral", AZ_TYPE_BOOLEAN, 1, AZ_FIELD_CLASS, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET(AZClass, impl.flags));
	any_class->props_self[ANY_PROP_INTEGRAL].mask = AZ_FLAG_INTEGRAL;
	az_class_define_property_value(any_class, ANY_PROP_SIGNED, (const uint8_t *) "isSigned", AZ_TYPE_BOOLEAN, 1, AZ_FIELD_CLASS, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET(AZClass, impl.flags));
	any_class->props_self[ANY_PROP_SIGNED].mask = AZ_FLAG_SIGNED;
	for (i = AZ_TYPE_ANY; i <= AZ_TYPE_POINTER; i++) {
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
		AZClass *klass = az_type_get_class (from_type);
		memcpy (to_val, from_val, klass->instance_size);
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


