#ifndef __AZ_SERIALIZATION_H__
#define __AZ_SERIALIZATION_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2017
*/

#include <az/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* All serialization methods accept NULL as destination */
/* Serialize returns the total number of bytes that would have been written (regardless of destination length) */
unsigned int az_serialize_block (unsigned char *d, unsigned int dlen, const void *s, unsigned int slen);
/* Deserialize returns the number of bytes consumed or a negative error code (AZ_INSUFFICIENT_SPACE) */
int64_t az_deserialize_block (void *d, unsigned int dlen, const unsigned char *s, unsigned int slen);

/* Size is in bytes */
unsigned int az_serialize_int (unsigned char *d, unsigned int dlen, const void *inst, unsigned int size);
int64_t az_deserialize_int (void *value, unsigned int size, const unsigned char *s, unsigned int slen);
unsigned int az_serialize_ints (unsigned char *d, unsigned int dlen, const void *inst, unsigned int size, unsigned int n_values);
int64_t az_deserialize_ints (void *value, unsigned int size, unsigned int n_values, const unsigned char *s, unsigned int slen);

#define az_serialize_float(d,dlen,inst) az_serialize_int (d, dlen, inst, 4)
#define az_serialize_double(d,dlen,inst) az_serialize_int (d, dlen, inst, 8)
#define az_deserialize_float(value,s,slen) az_deserialize_int (value, 4, s, slen)
#define az_deserialize_double(value,s,slen) az_deserialize_int (value, 8, s, slen)

#define az_serialize_floats(d,dlen,inst,n) az_serialize_ints (d, dlen, inst, 4, n)
#define az_deserialize_floats(value,s,slen,n) az_deserialize_ints (value, 4, n, s, slen)

#ifdef __cplusplus
};
#endif

#endif
