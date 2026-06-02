#define __AZ_PACKED_VALUE_ARRAY_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/packed-value.h>
#include <az/private.h>
#include <az/extend.h>

#include "packed-value-array.h"

static void packed_value_array_class_init (AZPackedValueArrayClass *klass);
static void packed_value_array_finalize (AZPackedValueArrayClass *klass, AZPackedValueArray *varray);

/* AZInstance implementation */
static unsigned int packed_value_array_get_property (const AZImplementation *impl, void *instance, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);
/* AZCollection implementation */
static unsigned int packed_value_array_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst);
/* AZList implementation */
static const AZImplementation *packed_value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

enum {
	PROP_LENGTH,
	NUM_PROPERTIES
};

unsigned int
az_packed_value_array_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZPackedValueArray", AZ_TYPE_REFERENCE, sizeof (AZPackedValueArrayClass), sizeof (AZPackedValueArray), AZ_FLAG_FINAL | AZ_FLAG_ZERO_MEMORY,
			1, NUM_PROPERTIES,
			(void (*) (AZClass *)) packed_value_array_class_init,
			NULL,
			(void (*) (const AZImplementation *, void *)) packed_value_array_finalize);
	}
	return type;
}

static void
packed_value_array_class_init (AZPackedValueArrayClass *klass)
{
	klass->reference_klass.klass.alignment = 15;
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET (AZPackedValueArrayClass, list_impl), ARIKKEI_OFFSET (AZPackedValueArray, list));
	az_class_define_property((AZClass *) klass, PROP_LENGTH, (uint8_t *) "length", AZ_TYPE_UINT32, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, AZ_FIELD_WRITE_NONE, ARIKKEI_OFFSET(AZPackedValueArray,list.collection.size), NULL, NULL);
	((AZClass *) klass)->get_property = packed_value_array_get_property;
	klass->list_impl.collection_impl.get_element_type = packed_value_array_get_element_type;
	klass->list_impl.get_element = packed_value_array_get_element;
}

static void
packed_value_array_finalize (AZPackedValueArrayClass *klass, AZPackedValueArray *varray)
{
	unsigned int i;
	for (i = 0; i < varray->list.collection.size; i++) {
		az_packed_value_clear (&varray->_values[i]);
	}
	if (varray->_values) free (varray->_values);
}

static unsigned int
packed_value_array_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	AZPackedValueArray *varray = (AZPackedValueArray *) inst;
	switch (idx) {
	case PROP_LENGTH:
		*prop_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT32);
		prop_val->uint32_v = varray->list.collection.size;
		return 1;
	default:
		break;
	}
	return 0;
}

static unsigned int
packed_value_array_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst)
{
	return AZ_TYPE_ANY;
}

static const AZImplementation *
packed_value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZPackedValueArray *varray;
	arikkei_return_val_if_fail (list_impl != NULL, 0);
	arikkei_return_val_if_fail (list_inst != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
	varray = (AZPackedValueArray *) ARIKKEI_BASE_ADDRESS(AZPackedValueArray,list,list_inst);
	arikkei_return_val_if_fail (idx < varray->list.collection.size, 0);
	if (varray->_values[idx].impl) {
		return az_value_copy_autobox (varray->_values[idx].impl, val, &varray->_values[idx].v, size);
	}
	return varray->_values[idx].impl;
}

AZPackedValueArray *
az_packed_value_array_new (unsigned int length)
{
	AZPackedValueArray *varray = (AZPackedValueArray *) az_instance_new (AZ_TYPE_PACKED_VALUE_ARRAY);
	varray->list.collection.size = length;
	varray->_values = (AZPackedValue *) az_instance_new_array (AZ_TYPE_PACKED_VALUE, varray->list.collection.size);
	return varray;
}

void
az_packed_value_array_set_element (AZPackedValueArray *varray, unsigned int idx, const AZPackedValue *value)
{
	arikkei_return_if_fail (varray != NULL);
	arikkei_return_if_fail (idx < varray->list.collection.size);
	arikkei_return_if_fail (value != NULL);
	az_packed_value_copy (&varray->_values[idx], value);
}
