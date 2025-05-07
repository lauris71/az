#ifndef __AZ_REFERENCE_OF_H__
#define __AZ_REFERENCE_OF_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2017
*/

typedef struct _AZReferenceOf AZReferenceOf;
typedef struct _AZReferenceOfClass AZReferenceOfClass;

#define AZ_TYPE_ABSTRACT_REFERENCE_OF az_abstract_reference_of_get_type ()
#define AZ_TYPE_REFERENCE_OF(t) az_reference_of_get_type (t)

#include <arikkei/arikkei-utils.h>

#include <az/reference.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Composite type that boxes value type in reference
 *
 * Alignment - max (reference alignment, value alignment)
 * Properly aligns value inside reference instance
 */

struct _AZReferenceOf {
	AZReference reference;
};

struct _AZReferenceOfClass {
	AZReferenceClass reference_klass;
	unsigned int instance_type;
};

unsigned int az_abstract_reference_of_get_type (void);
unsigned int az_reference_of_get_type (unsigned int instance_type);

AZReferenceOf *az_reference_of_new (unsigned int inst_type);
AZReferenceOf *az_reference_of_new_value (unsigned int inst_type, const AZValue *value);

ARIKKEI_INLINE unsigned int
az_type_is_a_reference_of (unsigned int type, unsigned int referenced_type)
{
	AZClass *klass = az_type_get_class (type);
	return (klass->flags & AZ_FLAG_REFERENCE) &&
		az_type_is_a (type, AZ_TYPE_ABSTRACT_REFERENCE_OF) &&
		az_type_is_a (((AZReferenceOfClass *) klass)->instance_type, referenced_type);
}

ARIKKEI_INLINE void *
az_reference_of_get_instance (AZReferenceOfClass *ref_klass, AZReferenceOf *ref)
{
	AZClass *inst_class = az_type_get_class (ref_klass->instance_type);
	unsigned int pos = ((unsigned int) sizeof (AZReferenceOf) + inst_class->alignment) & ~inst_class->alignment;
	return (AZValue *) ((char *) (ref) + pos);
}

#ifdef __cplusplus
};
#endif

#endif
