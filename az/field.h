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

enum AZFieldSpec {
	AZ_FIELD_INSTANCE = 0,
	AZ_FIELD_IMPLEMENTATION = 1,
	AZ_FIELD_CLASS = 2
};
#define AZ_FIELD_MASK_SPEC 0x3
#define AZ_FIELD_SHIFT_SPEC 16

enum AZFieldReadType {
	/* Not readable */
	AZ_FIELD_READ_NONE = 0,
	/* Directly read member value (final type or object) */
	AZ_FIELD_READ_VALUE = 1,
	/* Directly read instance of block type */
	AZ_FIELD_READ_INSTANCE = 7,
	/* Read from packed member value */
	AZ_FIELD_READ_PACKED = 2,
	/* Read via get_property method */
	AZ_FIELD_READ_METHOD = 3,
	/* Read static final value stored in field */
	AZ_FIELD_READ_STORED_STATIC = 4,
};
#define AZ_FIELD_MASK_READ 0x7
#define AZ_FIELD_SHIFT_READ 18

enum AZFieldWriteSpec {
	/* Not writeable */
	AZ_FIELD_WRITE_NONE = 0,
	/* Directly write member value (final type or object) */
	AZ_FIELD_WRITE_VALUE = 1,
	/* Write to packed member value */
	AZ_FIELD_WRITE_PACKED = 2,
	/* Write via set_property method */
	AZ_FIELD_WRITE_METHOD = 3
};
#define AZ_FIELD_MASK_WRITE 0x3
#define AZ_FIELD_SHIFT_WRITE 21

#define AZ_FIELD_IS_FUNCTION(f) ((f)->signature)
#define AZ_FIELD_IS_FINAL(f) ((f)->flags & AZ_FIELD_FLAG_FINAL)
#define AZ_FIELD_SPEC(f) (((f)->flags >> AZ_FIELD_SHIFT_SPEC) & AZ_FIELD_MASK_SPEC)
#define AZ_FIELD_READ(f) (((f)->flags >> AZ_FIELD_SHIFT_READ) & AZ_FIELD_MASK_READ)
#define AZ_FIELD_WRITE(f) (((f)->flags >> AZ_FIELD_SHIFT_WRITE) & AZ_FIELD_MASK_WRITE)

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
