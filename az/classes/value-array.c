#define __AZ_VALUE_ARRAY_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/boxed-value.h>
#include <az/private.h>
#include <az/extend.h>

#include "value-array.h"

static void value_array_class_init (AZValueArrayClass *klass);
static void value_array_init (AZValueArrayClass *klass, AZValueArray *varray);
static void value_array_finalize (AZValueArrayClass *klass, AZValueArray *varray);

/* AZCollection implementation */
static unsigned int value_array_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst);
static unsigned int value_array_get_size (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst);
static unsigned int value_array_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

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
	klass->list_impl.collection_impl.get_element_type = value_array_get_element_type;
	klass->list_impl.collection_impl.get_size = value_array_get_size;
	klass->list_impl.collection_impl.contains = value_array_contains;
	klass->list_impl.get_element = value_array_get_element;
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
	if (AZ_IMPL_VALUE_SIZE(varray->values[idx].impl) <= 8) return 0;
	return (AZ_IMPL_VALUE_SIZE(varray->values[idx].impl) + 7) >> 3;
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
			az_value_clear (varray->values[i].impl, value_array_element_value (varray, i));
		}
	}
	free (varray->values);
	if (varray->data) free (varray->data);
}

static unsigned int
value_array_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	return varray->type;
}

static unsigned int
value_array_get_size (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	return varray->length;
}

static unsigned int
value_array_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZValueArray *varray = (AZValueArray *) collection_inst;
	unsigned int i;
	for (i = 0; i < varray->length; i++) {
		if (varray->values[i].impl != impl) continue;
		if (!impl) return 1;
		if (!AZ_IMPL_VALUE_SIZE(varray->values[i].impl)) return 1;
		if (AZ_TYPE_IS_VALUE(AZ_IMPL_TYPE(varray->values[i].impl))) {
			if (!memcmp (value_array_element_value (varray, i), inst, AZ_IMPL_VALUE_SIZE(varray->values[i].impl))) return 1;
		} else {
			if (!memcmp (value_array_element_value (varray, i), &inst, AZ_IMPL_VALUE_SIZE(varray->values[i].impl))) return 1;
		}
	}
	return 0;
}

static const AZImplementation *
value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZValueArray *varray = (AZValueArray *) list_inst;
	if (varray->values[idx].impl) {
		return az_value_copy_autobox (varray->values[idx].impl, val, value_array_element_value (varray, idx), size);
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
				az_value_clear (varray->values[i].impl, value_array_element_value (varray, i));
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
	if (varray->values[idx].impl) {
		az_value_clear (varray->values[idx].impl, value_array_element_value (varray, idx));
	}
	varray->values[idx].impl = impl;
	if (impl) {
		if (AZ_IMPL_VALUE_SIZE(impl) <= 8) {
			az_value_copy (impl, (AZValue *) varray->values[idx].value, val);
		} else {
			value_array_ensure_room16 (varray, idx, (AZ_IMPL_VALUE_SIZE(impl) + 15) >> 4);
			az_value_copy (impl, varray->data + varray->values[idx].val_idx, val);
		}
	}
}

void
az_value_array_transfer_element (AZValueArray* varray, unsigned int idx, const AZImplementation* impl, const AZValue* val)
{
	if (varray->values[idx].impl) {
		az_value_clear (varray->values[idx].impl, value_array_element_value (varray, idx));
	}
	varray->values[idx].impl = impl;
	if (impl) {
		unsigned int size = AZ_IMPL_VALUE_SIZE(impl);
		if (size <= 8) {
			az_value_transfer (impl, (AZValue *) varray->values[idx].value, val);
		} else {
			value_array_ensure_room16 (varray, idx, (size + 15) >> 4);
			az_value_transfer (impl, varray->data + varray->values[idx].val_idx, val);
		}
	}
}




