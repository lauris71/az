#ifndef __AZ_STRING_H__
#define __AZ_STRING_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2018
*/

#include <arikkei/arikkei-dict.h>

#include <az/reference.h>

typedef struct _AZStringClass AZStringClass;

#ifdef __cplusplus
extern "C" {
#endif

struct _AZString {
	AZReference reference;
	unsigned int length;
	const unsigned char str[1];
};

struct _AZStringClass {
	AZReferenceClass reference_class;
	ArikkeiDict chr2str;
};

extern AZStringClass AZStringKlass;

AZString *az_string_new (const unsigned char *str);
AZString *az_string_new_length (const unsigned char *str, unsigned int length);
/* Both create new reference if string exists */
AZString *az_string_lookup (const unsigned char *chars);
AZString *az_string_lookup_length (const unsigned char *chars, unsigned int length);

ARIKKEI_INLINE void
az_string_ref (AZString *astr)
{
	az_reference_ref (&astr->reference);
}

ARIKKEI_INLINE void
az_string_unref (AZString *astr)
{
	az_reference_unref (&AZStringKlass.reference_class, &astr->reference);
}

static inline unsigned int
az_string_equals (const AZString *lhs, const AZString *rhs)
{
	return lhs == rhs;
}

AZString *az_string_concat (AZString *lhs, AZString *rhs);

/* Get serialized string as char array */
const unsigned char *az_string_deserialize_chars (const unsigned char *cdata, unsigned int csize, unsigned int *cpos);

#ifdef __cplusplus
};
#endif

#endif
