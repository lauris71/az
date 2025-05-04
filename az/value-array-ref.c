#define __AZ_VALUE_ARRAY_REF_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2020
*/

#include <az/field.h>
#include <az/value-array-ref.h>

static void value_array_ref_class_init (AZValueArrayRefClass* klass);
static void value_array_ref_init (AZValueArrayRefClass* klass, AZValueArrayRef* varef);
static void value_array_ref_finalize (AZValueArrayRefClass* klass, AZValueArrayRef* varef);

/* AZCollection implementation */
static unsigned int value_array_ref_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int value_array_ref_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int value_array_ref_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation* value_array_ref_get_element (const AZListImplementation* list_impl, void *list_inst, unsigned int idx, AZValue64 *val);

enum {
	PROP_LENGTH,
	NUM_PROPERTIES
};

static AZValueArrayRefClass *varef_class = NULL;

unsigned int
az_value_array_ref_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZValueArrayRef", AZ_TYPE_REFERENCE, sizeof (AZValueArrayRefClass), sizeof (AZValueArrayRef), AZ_FLAG_FINAL,
			(void (*) (AZClass*)) value_array_ref_class_init,
			(void (*) (const AZImplementation *, void *)) value_array_ref_init,
			(void (*) (const AZImplementation *, void *)) value_array_ref_finalize);
	}
	return type;
}

static void
value_array_ref_class_init (AZValueArrayRefClass* klass)
{
	varef_class = klass;
	klass->reference_klass.klass.alignment = 15;
	az_class_set_num_interfaces ((AZClass *) klass, 1);
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET (AZValueArrayRefClass, list_implementation), 0);
	az_class_set_num_properties (( AZClass*) klass, NUM_PROPERTIES);
	az_class_define_property ((AZClass*) klass, PROP_LENGTH, (const unsigned char *) "length", AZ_TYPE_UINT32, 0, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0,
		ARIKKEI_OFFSET(AZValueArrayRef, varray) + ARIKKEI_OFFSET(AZValueArray, length), NULL, NULL);
	klass->list_implementation.collection_impl.get_element_type = value_array_ref_get_element_type;
	klass->list_implementation.collection_impl.get_size = value_array_ref_get_size;
	klass->list_implementation.collection_impl.contains = value_array_ref_contains;
	klass->list_implementation.get_element = value_array_ref_get_element;
}

static void
value_array_ref_init (AZValueArrayRefClass *klass, AZValueArrayRef *varef)
{
	az_instance_init (&varef->varray, AZ_TYPE_VALUE_ARRAY);
}

static void
value_array_ref_finalize (AZValueArrayRefClass *klass, AZValueArrayRef *varef)
{
	az_instance_finalize (&varef->varray, AZ_TYPE_VALUE_ARRAY);
}

static unsigned int
value_array_ref_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZValueArrayRef *varef = (AZValueArrayRef *) collection_inst;
	return varef->varray.type;
}

static unsigned int
value_array_ref_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZValueArrayRef *varef = (AZValueArrayRef *) collection_inst;
	return varef->varray.length;
}

static unsigned int
value_array_ref_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZValueArrayRef *varef = (AZValueArrayRef *) collection_inst;
	return az_collection_contains (&az_value_array_class->list_implementation.collection_impl, &varef->varray, impl, inst);
}

static const AZImplementation *
value_array_ref_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *val)
{
	AZValueArrayRef *varef = (AZValueArrayRef *) list_inst;
	return az_list_get_element (&az_value_array_class->list_implementation, &varef->varray, idx, val);
}

AZValueArrayRef *
az_value_array_ref_new (unsigned int length)
{
	AZValueArrayRef *varef = (AZValueArrayRef*) az_instance_new (AZ_TYPE_VALUE_ARRAY_REF);
	az_value_array_set_length (&varef->varray, length);
	return varef;
}

void
az_value_array_ref_set_element (AZValueArrayRef *varef, unsigned int idx, const AZImplementation *impl, AZValue* value)
{
	az_value_array_set_element (&varef->varray, idx, impl, value);
}
