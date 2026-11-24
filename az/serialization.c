#define __AZ_SERIALIZATION_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2017
*/

#include <stdio.h>
#include <string.h>

#include <az/serialization.h>

int64_t
az_serialize_block (unsigned char *d, unsigned int dlen, const void *s, unsigned int slen)
{
	if ((slen <= dlen) && d) memcpy (d, s, slen);
	return slen;
}

int64_t
az_deserialize_block (void *d, unsigned int dlen, const unsigned char *s, unsigned int slen)
{
	if (dlen > slen) return AZ_INSUFFICIENT_SPACE;
	memcpy (d, s, dlen);
	return dlen;
}

int64_t
az_serialize_int (unsigned char *d, unsigned int dlen, const void *inst, unsigned int size)
{
	if ((dlen >= size) && d) {
		unsigned int i;
		for (i = 0; i < size; i++) {
			d[size - 1 - i] = *((unsigned char *) inst + i);
		}
	}
	return size;
}

int64_t
az_deserialize_int (void *value, unsigned int size, const unsigned char *s, unsigned int slen)
{
	unsigned int i;
	if (slen < size) return AZ_INSUFFICIENT_SPACE;
	for (i = 0; i < size; i++) {
		*((unsigned char *) value + i) = s[size - 1 - i];
	}
	return size;
}

int64_t
az_serialize_ints(unsigned char* d, unsigned int dlen, const void* inst, unsigned int size, unsigned int n_values)
{
	if (dlen >= (n_values * size)) {
		uint64_t i;
		for (i = 0; i < n_values; i++) az_serialize_int(d + size * i, size, (const unsigned char *) inst + size * i, size);
	}
	return n_values * size;
}

int64_t
az_deserialize_ints(void* value, unsigned int size, unsigned int n_values, const unsigned char* s, unsigned int slen)
{
	uint64_t i;
	if (slen < n_values * size) return AZ_INSUFFICIENT_SPACE;
	for (i = 0; i < n_values; i++) az_deserialize_int((unsigned char *) value + size * i, size, s + size * i, size);
	return n_values * size;
}
