#define __AZ_REFERENCE_OF_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-strlib.h>

#include <az/private.h>
#include <az/extend.h>

#include <az/reference-of.h>

static unsigned int abstract_reference_of_type = 0;

unsigned int
az_abstract_reference_of_get_type (void)
{
	if (!abstract_reference_of_type) {
		az_register_type (&abstract_reference_of_type, (const unsigned char *) "AbstractReferenceOf", AZ_TYPE_REFERENCE, sizeof (AZReferenceOfClass), sizeof (AZReferenceOf), AZ_FLAG_ABSTRACT,
			NULL, NULL, NULL);
	}
	return abstract_reference_of_type;
}

static void reference_of_class_init (AZReferenceOfClass *klass, AZClass *inst_class);
static void reference_of_instance_init (AZReferenceOfClass *klass, AZReferenceOf *ref_of);
static void reference_of_instance_finalize (AZReferenceOfClass *klass, AZReferenceOf *ref_of);
/* AZClass implementation */
static unsigned int reference_of_to_string (const AZImplementation* impl, void *instance, unsigned char *buf, unsigned int len);

unsigned int
az_reference_of_get_type (unsigned int instance_type)
{
	static unsigned int num_subtypes = 0;
	static unsigned int *subtypes = NULL;
	arikkei_return_val_if_fail (AZ_TYPE_IS_VALUE(instance_type), 0);
	if (AZ_TYPE_INDEX(instance_type) >= num_subtypes) {
		unsigned int new_size = (AZ_TYPE_INDEX(instance_type) + 1 + 255) & 0xffffff00;
		subtypes = realloc (subtypes, new_size * sizeof (unsigned int));
		memset (&subtypes[num_subtypes], 0, (new_size - num_subtypes) * sizeof (unsigned int));
		num_subtypes = new_size;
	}
	if (!subtypes[AZ_TYPE_INDEX(instance_type)]) {
		/* Only value types */
		AZClass *inst_class = AZ_CLASS_FROM_TYPE(instance_type);
		unsigned int len = (unsigned int) strlen ((const char *) inst_class->name);
		unsigned char *name = malloc (len + 12);
		sprintf ((char *) name, "ReferenceOf%s", inst_class->name);
		unsigned int pos = (sizeof (AZReferenceOf) + inst_class->alignment) & ~(inst_class->alignment);
		az_register_composite_type (&subtypes[AZ_TYPE_INDEX(instance_type)], name, AZ_TYPE_ABSTRACT_REFERENCE_OF, sizeof (AZReferenceOfClass), pos + inst_class->instance_size, 0,
			(void (*) (AZClass *, void *)) reference_of_class_init,
			(void (*) (const AZImplementation *, void *)) reference_of_instance_init,
			(void (*) (const AZImplementation *, void *)) reference_of_instance_finalize,
			inst_class);
	}
	return subtypes[AZ_TYPE_INDEX(instance_type)];
}

static void
reference_of_class_init (AZReferenceOfClass *klass, AZClass *inst_class)
{
	if (inst_class->alignment > klass->reference_klass.klass.alignment) klass->reference_klass.klass.alignment = inst_class->alignment;
	klass->instance_type = AZ_CLASS_TYPE(inst_class);
	klass->reference_klass.klass.to_string = reference_of_to_string;
}

static void
reference_of_instance_init (AZReferenceOfClass *klass, AZReferenceOf *ref_of)
{
	az_instance_init_by_type (az_reference_of_get_instance (klass, ref_of), klass->instance_type);
}

static void
reference_of_instance_finalize (AZReferenceOfClass *klass, AZReferenceOf *ref_of)
{
	az_instance_finalize_by_type (az_reference_of_get_instance (klass, ref_of), klass->instance_type);
}

static unsigned int
reference_of_to_string (const AZImplementation* impl, void *instance, unsigned char *buf, unsigned int len)
{
	AZReferenceOfClass *ref_class = (AZReferenceOfClass *) impl;
	AZClass* inst_class = AZ_CLASS_FROM_TYPE(ref_class->instance_type);
	unsigned int pos;
	pos = arikkei_memcpy_str (buf, len, (const unsigned char *) "Reference of ");
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, inst_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " (");
	pos += az_instance_to_string (&inst_class->impl, az_reference_of_get_instance (ref_class, instance), buf + pos, (len > pos) ? len - pos : 0);
	pos += arikkei_strncpy (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) ")");
	return pos;
}

AZReferenceOf *
az_reference_of_new (unsigned int inst_type)
{
	return az_instance_new (AZ_TYPE_REFERENCE_OF (inst_type));
}

AZReferenceOf *
az_reference_of_new_value (unsigned int inst_type, const AZValue *value)
{
	unsigned int type = AZ_TYPE_REFERENCE_OF (inst_type);
	AZReferenceOf *ref = (AZReferenceOf *) az_instance_new (type);
	memcpy (az_reference_of_get_instance ((AZReferenceOfClass *) AZ_CLASS_FROM_TYPE(type), ref), value, AZ_CLASS_FROM_TYPE(inst_type)->instance_size);
	return ref;
}
