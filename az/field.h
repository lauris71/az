#ifndef __AZ_FIELD_H__
#define __AZ_FIELD_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <az/packed-value.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AZ_FIELD_INSTANCE 0
#define AZ_FIELD_IMPLEMENTATION 1
#define AZ_FIELD_CLASS 2

/* Not readable */
#define AZ_FIELD_READ_NONE 0
/* Directly read member value (final type or object) */
#define AZ_FIELD_READ_VALUE 1
/* Read from packed member value */
#define AZ_FIELD_READ_PACKED 2
/* Read static final value stored in field */
#define AZ_FIELD_READ_STORED_STATIC 3
/* Read via get_property method */
#define AZ_FIELD_READ_METHOD 4
/* Read as boxed interface of the owner instance */
#define AZ_FIELD_READ_BOXED_INTERFACE 5

/* Not writeable */
#define AZ_FIELD_WRITE_NONE 0
/* Directly write member value (final type or object) */
#define AZ_FIELD_WRITE_VALUE 1
/* Write to packed member value */
#define AZ_FIELD_WRITE_PACKED 2
/* Write via set_property method */
#define AZ_FIELD_WRITE_METHOD 3

struct _AZField {
	AZString *key;
	unsigned int type;
	unsigned int offset;
	/* Shortcuts */
	unsigned int is_reference : 1;
	unsigned int is_interface : 1;
	unsigned int is_function : 1;
	/* Mutation types */
	unsigned int is_final : 1;
	unsigned int spec : 2;
	unsigned int read : 3;
	unsigned int write : 2;
	/* Signature for functions */
	const AZFunctionSignature *signature;
	/* Default value, may be the actual value for final properties */
	AZPackedValue64 val;
};

void az_field_setup (AZField *prop, const unsigned char *key, unsigned int type, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst);

void az_field_setup_function (AZField *prop, const unsigned char *key, unsigned int is_final,
	unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig,
	const AZImplementation *impl, void *inst);

//void az_field_setup_from_def (AZField *prop, AZFieldDefinition *def);

/* Library internals */
void az_init_field_class (void);

#ifdef __cplusplus
};
#endif


#endif
