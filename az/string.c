#define __AZ_STRING_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <az/class.h>
#include <az/private.h>
#include <az/serialization.h>
#include <az/string.h>

#include "value.h"

typedef struct _AZStringLookup AZStringLookup;

struct _AZStringLookup {
	unsigned int len;
	const unsigned char *str;
};

static unsigned int
string_hash (const void *data)
{
	AZString *str = (AZString *) data;
	unsigned int hval, i;
	hval = 0;
	for (i = 0; i < str->length; i++) {
		hval = (hval << 5) - hval + str->str[i];
	}
	return hval ^ str->length;
}

static unsigned int
string_equal (const void *l, const void *r)
{
	AZString *lhs = (AZString *) l;
	AZString *rhs = (AZString *) r;
	if (lhs->length != rhs->length) return 0;
	return !strcmp ((const char *) lhs->str, (const char *) rhs->str);
}

static unsigned int
string_data_hash (const void *data)
{
	AZStringLookup *lookup = (AZStringLookup *) data;
	unsigned int hval, i;
	hval = 0;
	for (i = 0; i < lookup->len; i++) {
		hval = (hval << 5) - hval + lookup->str[i];
	}
	return hval ^ lookup->len;
}

static unsigned int
string_data_equal (const void *l, const void *r)
{
	AZStringLookup *lhs = (AZStringLookup *) l;
	AZString *rhs = (AZString *) r;
	if (lhs->len != rhs->length) return 0;
	return !strncmp ((const char *) lhs->str, (const char *) rhs->str, lhs->len);
}

static unsigned int
string_to_string (const AZImplementation *impl, void *instance, unsigned char *buf, unsigned int len)
{
	unsigned int pos = 0;
	if (instance) {
		AZString *str = (AZString *) instance;
		unsigned int slen = (str->length > len) ? len : str->length;
		memcpy (buf + pos, str->str, slen);
		pos += slen;
	}
	if (pos < len) buf[pos] = 0;
	return pos;
}

static unsigned int
serialize_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZString *str = (AZString *) inst;
	if (!str) {
		static const unsigned char b[9] = { 0, 0, 0, 0, 0 };
		return az_serialize_block (d, dlen, b, 5);
	} else {
		if ((5 + str->length) <= dlen) {
			az_serialize_int (d, dlen, &str->length, 4);
			az_serialize_block (d + 4, dlen - 4, str->str, str->length + 1);
		}
		return 5 + str->length;
	}
}

static unsigned int
deserialize_string (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZString **str = &value->string;
	unsigned int len;
	if (slen < 5) {
		*str = NULL;
		return 0;
	}
	az_deserialize_int (&len, 4, s, slen);
	if ((5 + len) > slen) {
		*str = NULL;
		return 0;
	}
	*str = az_string_new_length (s + 4, len);
	return 5 + len;
}

static void
string_dispose (AZReferenceClass *klass, AZReference *ref)
{
	AZString *str = (AZString *) ref;
	arikkei_dict_remove (&AZStringKlass.chr2str, str);
}

AZStringClass AZStringKlass = {
	{{{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_REFERENCE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_STRING},
	&AZReferenceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "string",
	7, sizeof(AZStringClass), 0,
	NULL,
	NULL, NULL,
	serialize_string, deserialize_string, string_to_string,
	NULL, NULL},
	NULL, string_dispose},
	{0}
};

void
az_init_string_class (void)
{
	az_class_new_with_value(&AZStringKlass.reference_class.klass);
	//az_string_class = (AZStringClass *) az_class_new_with_type (AZ_TYPE_STRING, AZ_TYPE_REFERENCE, sizeof (AZStringClass), 0, AZ_FLAG_FINAL, (const uint8_t *) "string");
	//az_string_class->reference_class.klass.serialize = serialize_string;
	//az_string_class->reference_class.klass.deserialize = deserialize_string;
	//az_string_class->reference_class.klass.to_string = string_to_string;
	//az_string_class->reference_class.dispose = string_dispose;
	arikkei_dict_setup_full (&AZStringKlass.chr2str, 701, string_hash, string_equal);
}

AZString *
az_string_new (const unsigned char *str)
{
	if (!str) return NULL;
	return az_string_new_length (str, (unsigned int) strlen ((const char *) str));
}

AZString *
az_string_new_length (const unsigned char *str, unsigned int length)
{
	AZStringLookup lookup;
	AZString *astr;
	lookup.len = length;
	lookup.str = str;
	astr = (AZString *) arikkei_dict_lookup_foreign (&AZStringKlass.chr2str, &lookup, string_data_hash, string_data_equal);
	if (astr) {
		az_string_ref (astr);
	} else {
		astr = (AZString *) malloc (sizeof (AZString) + length);
		az_instance_init (astr, AZ_TYPE_STRING);
		astr->length = length;
		memcpy ((unsigned char *) astr->str, str, length);
		((unsigned char *) astr->str)[length] = 0;
		arikkei_dict_insert (&AZStringKlass.chr2str, astr, astr);
	}
	return astr;
}

AZString *
az_string_lookup (const unsigned char *chars)
{
	if (!chars) return NULL;
	return az_string_lookup_length (chars, (unsigned int) strlen ((const char *) chars));
}

AZString *
az_string_lookup_length (const unsigned char *chars, unsigned int length)
{
	AZStringLookup lookup;
	AZString *astr;
	lookup.len = length;
	lookup.str = chars;
	astr = (AZString *) arikkei_dict_lookup_foreign (&AZStringKlass.chr2str, &lookup, string_data_hash, string_data_equal);
	if (astr) az_string_ref (astr);
	return astr;
}

AZString *
az_string_concat (AZString *lhs, AZString *rhs)
{
	AZString *built, *astr;
	if (!lhs) return rhs;
	if (!rhs) return lhs;
	built = (AZString *) malloc (sizeof (AZString) + lhs->length + rhs->length);
	az_instance_init (built, AZ_TYPE_STRING);
	built->length = lhs->length + rhs->length;
	if (lhs->length) memcpy ((unsigned char *) built->str, lhs->str, lhs->length);
	if (rhs->length) memcpy ((unsigned char *) built->str + lhs->length, rhs->str, rhs->length);
	((unsigned char *) built->str)[lhs->length + rhs->length] = 0;
	astr = (AZString *) arikkei_dict_lookup (&AZStringKlass.chr2str, built);
	if (astr) {
		az_string_unref (built);
	} else {
		astr = built;
		arikkei_dict_insert (&AZStringKlass.chr2str, astr, astr);
	}
	return astr;
}

const unsigned char *
az_string_deserialize_chars (const unsigned char *cdata, unsigned int csize, unsigned int *cpos)
{
	unsigned int slen;
	const unsigned char *str;
	if ((csize - *cpos) < 5) return NULL;
	az_deserialize_int (&slen, 4, cdata + *cpos, csize - *cpos);
	if ((csize - *cpos) < (5 + slen)) return NULL;
	str = cdata + *cpos + 4;
	*cpos += (4 + slen + 1);
	return str;
}
