#define __AZ_VALUE_ARRAY_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/packed-value.h>
#include <az/private.h>
#include <az/value-array.h>

static void value_array_class_init (AZValueArrayClass *klass);
static void value_array_init (AZValueArrayClass *klass, AZValueArray *varray);
static void value_array_finalize (AZValueArrayClass *klass, AZValueArray *varray);

/* AZCollection implementation */
static unsigned int value_array_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int value_array_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int value_array_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *value);

AZValueArrayClass *az_value_array_class = NULL;

unsigned int
az_value_array_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZValueArray", AZ_TYPE_BLOCK, sizeof (AZValueArrayClass), sizeof (AZValueArray), AZ_FLAG_FINAL,
			(void (*) (AZClass *)) value_array_class_init,
			(void (*) (const AZImplementation *, void *)) value_array_init,
			(void (*) (const AZImplementation *, void *)) value_array_finalize);
	}
	return type;
}

static void
value_array_class_init (AZValueArrayClass *klass)
{
	az_value_array_class = klass;
	klass->default_size = 4;
	klass->list_implementation.collection_impl.get_element_type = value_array_get_element_type;
	klass->list_implementation.collection_impl.get_size = value_array_get_size;
	klass->list_implementation.collection_impl.contains = value_array_contains;
	klass->list_implementation.get_element = value_array_get_element;
}

static void
value_array_init (AZValueArrayClass *klass, AZValueArray *varray)
{
	varray->type = AZ_TYPE_ANY;
	varray->size = klass->default_size;
	varray->data_size = 0;
	varray->length = 0;
	varray->values = (AZValueArrayEntry *) malloc (varray->size * sizeof (AZValueArrayEntry));
	varray->data = NULL;
}

static unsigned int
value_array_element_size8 (AZValueArray *varray, unsigned int idx)
{
	if (AZ_TYPE_VALUE_SIZE(varray->values[idx].impl->type) <= 8) return 0;
	return (AZ_TYPE_VALUE_SIZE(varray->values[idx].impl->type) + 7) >> 3;
}

static AZValue *
value_array_element_value (AZValueArray *varray, unsigned int idx)
{
	if (!value_array_element_size8 (varray, idx)) {
		return (AZValue *) varray->values[idx].value;
	} else {
		return varray->data + varray->values[idx].val_idx;
	}
}

static void
value_array_finalize (AZValueArrayClass *klass, AZValueArray *varray)
{
	unsigned int i;
	for (i = 0; i < varray->length; i++) {
		if (varray->values[i].impl) {
			az_clear_value (varray->values[i].impl, value_array_element_value (varray, i));
		}
	}
	free (varray->values);
	if (varray->data) free (varray->data);
}

static unsigned int
value_array_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	return varray->type;
}

static unsigned int
value_array_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	return varray->length;
}

static unsigned int
value_array_contains (const AZCollectionImplementation *collection_impl, void *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	unsigned int i;
	for (i = 0; i < varray->length; i++) {
		if (varray->values[i].impl != impl) continue;
		if (!impl) return 1;
		if (!AZ_TYPE_VALUE_SIZE(varray->values[i].impl->type)) return 1;
		if (AZ_TYPE_IS_VALUE(varray->values[i].impl->type)) {
			if (!memcmp (value_array_element_value (varray, i), inst, AZ_TYPE_VALUE_SIZE(varray->values[i].impl->type))) return 1;
		} else {
			if (!memcmp (value_array_element_value (varray, i), &inst, AZ_TYPE_VALUE_SIZE(varray->values[i].impl->type))) return 1;
		}
	}
	return 0;
}

static const AZImplementation *
value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *val)
{
	AZValueArray *varray = (AZValueArray *) list_inst;
	if (varray->values[idx].impl) {
		az_copy_value (varray->values[idx].impl, &val->value, value_array_element_value (varray, idx));
	}
	return varray->values[idx].impl;
}

void
az_value_array_set_length (AZValueArray* varray, unsigned int length)
{
	unsigned int i;
	if (length < varray->length) {
		for (i = length; i < varray->length; i++) {
			if (varray->values[i].impl) {
				az_clear_value (varray->values[i].impl, value_array_element_value (varray, i));
			}
		}
		varray->length = length;
	} else if (length > varray->length) {
		if (length > varray->size) {
			varray->size = length;
			varray->values = (AZValueArrayEntry *) realloc (varray->values, varray->size * sizeof (AZValueArrayEntry));
		}
		for (i = varray->length; i < length; i++) {
			varray->values[i].impl = NULL;
		}
		varray->length = length;
	}
}

static unsigned int
value_array_ensure_room16 (AZValueArray* varray, unsigned int idx, unsigned int req16)
{
	unsigned int left_start, left_end, right_start, right_end;
	unsigned int i;
	if (!varray->data) {
		varray->data_size = (req16 > 4) ? req16 : 4;
		varray->data = (AZValue *) malloc (varray->data_size * sizeof (AZValue));
		return 0;
	}
	/* fixme: Compact during scan? (Lauris) */
	left_start = left_end = 0;
	for (i = 0; i < idx; i++) {
		if (varray->values[i].impl) {
			unsigned int size8 = value_array_element_size8 (varray, i);
			if (size8) {
				unsigned int size16 = (size8 + 1) / 2;
				left_end = varray->values[i].val_idx + size16;
			}
		}
	}
	/* fixme: Compact during scan? (Lauris) */
	right_start = right_end = 0;
	for (i = idx + 1; i < varray->length; i++) {
		if (varray->values[i].impl) {
			unsigned int size8 = value_array_element_size8 (varray, i);
			if (size8) {
				unsigned int size16 = (size8 + 1) / 2;
				if (right_end) right_start = varray->values[i].val_idx;
				right_end = varray->values[i].val_idx + size16;
			}
		}
	}
	if (!right_end) {
		/* No large elements at right, append to data block */
		if ((left_end + req16) > varray->size) {
			varray->size *= 2;
			if ((left_end + req16) > varray->size) varray->size = left_end + req16;
			varray->data = (AZValue *) realloc (varray->data, varray->size * sizeof (AZValue));
		}
	} else {
		if ((right_start - left_end) >= req16) {
			/* We have enough room */
		} else {
			/* Have to shift values right */
			unsigned int delta16 = (right_start - left_end) - req16;
			if ((right_end + delta16) > varray->size) {
				varray->size *= 2;
				if ((right_end + delta16) > varray->size) varray->size = right_end + delta16;
				varray->data = ( AZValue*) realloc (varray->data, varray->size * sizeof (AZValue));
			}
			right_start += delta16;
			for (i = idx + 1; i < varray->length; i++) {
				if (varray->values[i].impl) {
					unsigned int size8 = value_array_element_size8 (varray, i);
					if (size8) {
						unsigned int size16 = (size8 + 1) / 2;
						varray->values[i].val_idx += delta16;
						right_start += size16;
					}
				}
			}
		}
	}
	return left_end;
}

void
az_value_array_set_element (AZValueArray* varray, unsigned int idx, const AZImplementation* impl, const AZValue* val)
{
	if (varray->values[idx].impl) az_clear_value (varray->values[idx].impl, value_array_element_value (varray, idx));
	varray->values[idx].impl = impl;
	if (impl) {
		if (AZ_TYPE_VALUE_SIZE(impl->type) <= 8) {
			az_copy_value (impl, (AZValue *) varray->values[idx].value, val);
		} else {
			value_array_ensure_room16 (varray, idx, (AZ_TYPE_VALUE_SIZE(impl->type) + 15) >> 4);
			az_copy_value (impl, varray->data + varray->values[idx].val_idx, val);
		}
	}
}

void
az_value_array_transfer_element (AZValueArray* varray, unsigned int idx, const AZImplementation* impl, const AZValue* val)
{
	if (varray->values[idx].impl) az_clear_value (varray->values[idx].impl, value_array_element_value (varray, idx));
	varray->values[idx].impl = impl;
	if (impl) {
		if (AZ_TYPE_VALUE_SIZE(impl->type) <= 8) {
			az_transfer_value (impl, (AZValue *) varray->values[idx].value, val);
		} else {
			value_array_ensure_room16 (varray, idx, (AZ_TYPE_VALUE_SIZE(impl->type) + 15) >> 4);
			az_transfer_value (impl, varray->data + varray->values[idx].val_idx, val);
		}
	}
}


static void packed_value_array_class_init (AZPackedValueArrayClass *klass);
static void packed_value_array_finalize (AZPackedValueArrayClass *klass, AZPackedValueArray *varray);

/* AZInstance implementation */
static unsigned int packed_value_array_get_property (const AZImplementation *impl, void *instance, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);
/* AZCollection implementation */
static unsigned int packed_value_array_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst);
static unsigned int packed_value_array_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst);
/* AZList implementation */
static const AZImplementation *packed_value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *value);

enum {
	PROP_LENGTH,
	NUM_PROPERTIES
};

unsigned int
az_packed_value_array_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZPackedValueArray", AZ_TYPE_REFERENCE, sizeof (AZPackedValueArrayClass), sizeof (AZPackedValueArray), AZ_FLAG_FINAL | AZ_CLASS_ZERO_MEMORY,
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
	az_class_set_num_interfaces ((AZClass *) klass, 1);
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET (AZPackedValueArrayClass, list_implementation), 0);
	az_class_set_num_properties ((AZClass *) klass, NUM_PROPERTIES);
	az_class_property_setup ((AZClass *) klass, PROP_LENGTH, (const unsigned char *) "length", AZ_TYPE_UINT32, 0, 1, 0, 1, 0, AZ_TYPE_NONE, NULL);
	((AZClass *) klass)->get_property = packed_value_array_get_property;
	klass->list_implementation.collection_impl.get_element_type = packed_value_array_get_element_type;
	klass->list_implementation.collection_impl.get_size = packed_value_array_get_size;
	klass->list_implementation.get_element = packed_value_array_get_element;
}

static void
packed_value_array_finalize (AZPackedValueArrayClass *klass, AZPackedValueArray *varray)
{
	unsigned int i;
	for (i = 0; i < varray->length; i++) {
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
		prop_val->uint32_v = varray->length;
		return 1;
	default:
		break;
	}
	return 0;
}

static unsigned int
packed_value_array_get_element_type (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	return AZ_TYPE_ANY;
}

static unsigned int
packed_value_array_get_size (const AZCollectionImplementation *collection_impl, void *collection_inst)
{
	AZPackedValueArray *varray = (AZPackedValueArray *) collection_inst;
	return varray->length;
}

static const AZImplementation *
packed_value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue64 *val)
{
	AZPackedValueArray *varray;
	arikkei_return_val_if_fail (list_impl != NULL, 0);
	arikkei_return_val_if_fail (list_inst != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
	varray = (AZPackedValueArray *) list_inst;
	arikkei_return_val_if_fail (idx < varray->length, 0);
	if (varray->_values[idx].impl) {
		az_copy_value (varray->_values[idx].impl, &val->value, &varray->_values[idx].v);
	}
	/* az_packed_value_copy (value, &varray->_values[index]); */
	return varray->_values[idx].impl;
}

AZPackedValueArray *
az_packed_value_array_new (unsigned int length)
{
	AZPackedValueArray *varray = (AZPackedValueArray *) az_instance_new (AZ_TYPE_PACKED_VALUE_ARRAY);
	varray->length = length;
	varray->_values = (AZPackedValue *) az_instance_new_array (AZ_TYPE_PACKED_VALUE, varray->length);
	return varray;
}

void
az_packed_value_array_set_element (AZPackedValueArray *varray, unsigned int idx, const AZPackedValue *value)
{
	arikkei_return_if_fail (varray != NULL);
	arikkei_return_if_fail (idx < varray->length);
	arikkei_return_if_fail (value != NULL);
	az_packed_value_copy (&varray->_values[idx], value);
}
