#ifndef __AZ_FIELD_H__
#define __AZ_FIELD_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

typedef struct _AZFieldFunction AZFieldFUnction;

#include <az/packed-value.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AZ_FIELD_FLAG_FINAL 0b01

#define AZ_FIELD_MASK_SPEC (0x3 << 1)
#define AZ_FIELD_INSTANCE (0 << 1)
#define AZ_FIELD_IMPLEMENTATION (1 << 1)
#define AZ_FIELD_CLASS (2 << 1)

#define AZ_FIELD_MASK_READ (0x7 << 3)
/* Not readable */
#define AZ_FIELD_READ_NONE (0 << 3)
/* Directly read member value (final type or object) */
#define AZ_FIELD_READ_VALUE (1 << 3)
/* Directly read instance of block type */
#define AZ_FIELD_READ_INSTANCE (7 << 3)
/* Read from packed member value */
#define AZ_FIELD_READ_PACKED (2 << 3)
/* Read via get_property method */
#define AZ_FIELD_READ_METHOD (3 << 3)
/* Read static final value stored in field */
#define AZ_FIELD_READ_STORED_STATIC (4 << 3)

#define AZ_FIELD_MASK_WRITE (0x3 << 6)
/* Not writeable */
#define AZ_FIELD_WRITE_NONE (0 << 6)
/* Directly write member value (final type or object) */
#define AZ_FIELD_WRITE_VALUE (1 << 6)
/* Write to packed member value */
#define AZ_FIELD_WRITE_PACKED (2 << 6)
/* Write via set_property method */
#define AZ_FIELD_WRITE_METHOD (3 << 6)

#define AZ_FIELD_IS_FUNCTION(f) ((f)->signature)
#define AZ_FIELD_IS_FINAL(f) ((f)->flags & AZ_FIELD_FLAG_FINAL)
#define AZ_FIELD_SPEC(f) ((f)->flags & AZ_FIELD_MASK_SPEC)
#define AZ_FIELD_READ(f) ((f)->flags & AZ_FIELD_MASK_READ)
#define AZ_FIELD_WRITE(f) ((f)->flags & AZ_FIELD_MASK_WRITE)

struct _AZField {
	AZString *key;
	uint32_t type;
	uint32_t flags;
	/* Signature for functions */
	const AZFunctionSignature *signature;
	union {
		struct {
			/* Offset in instance/implementation/class */
			uint32_t offset;
			/* Masked (bit-field) values */
			/* The type index of the containing value (AZ_TYPE_IDX_UINT8...AZ_TYPE_IDX_UINT64, */
			/* determines the width of the integer read/written at offset, 0 defaults to uint32) */
			uint8_t value_type_idx;
			/* Number of bits to shift right before masking */
			uint8_t shift;
			/* The number of contiguous bits in the mask */
			uint8_t mask_width;
			/* For boolean types */
			/* Final value to xor with */
			uint8_t bits;
		};
		/* Stored value, may be the actual value for final properties */
		struct {
			const AZImplementation *impl;
			AZValue v;
		} value;
	};
} ARIKKEI_ALIGN_16;

extern AZClass AZFieldKlass;

/* Read is VALUE or PACKED, write is NONE, the same as read or METHOD */
void az_field_setup_value (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, unsigned int offset);

/* Read is STORED_STATIC, write is NONE */
void az_field_setup_stored (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, const AZImplementation *impl, void *inst);

void az_field_setup_method (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write);

void az_field_setup_function (AZField *prop, const unsigned char *key, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, const AZFunctionSignature *sig,
	const AZImplementation *impl, void *inst);

void az_field_setup_function_packed (AZField *prop, const unsigned char *key, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, const AZFunctionSignature *sig, unsigned int offset);

#ifdef __cplusplus
};
#endif


#endif
