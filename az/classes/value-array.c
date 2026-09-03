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
static unsigned int value_array_ensure_room16 (AZValueArray *varray, unsigned int idx, unsigned int req16);

/* AZCollection implementation */
static unsigned int value_array_get_element_type (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst);
static unsigned int value_array_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst);
/* AZList implementation */
static const AZImplementation *value_array_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

unsigned int
az_value_array_get_type (void)
{
	static unsigned int type = 0;
	unsigned int t = AZ_TYPE_READ(type);
	if (t) return t;
	AZ_TYPES_LOCK();
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZValueArray", AZ_TYPE_REFERENCE, sizeof (AZValueArrayClass), sizeof (AZValueArray), AZ_FLAG_FINAL,
			1, 0,
			(void (*) (AZClass *)) value_array_class_init,
			(void (*) (const AZImplementation *, void *)) value_array_init,
			(void (*) (const AZImplementation *, void *)) value_array_finalize);
	}
	t = type;
	AZ_TYPES_UNLOCK();
	return t;
}

AZValueArray *
az_value_array_new (unsigned int length)
{
	AZValueArray *varray = (AZValueArray *) az_instance_new (AZ_TYPE_VALUE_ARRAY);
	az_value_array_set_length (varray, length);
	return varray;
}

static void
value_array_class_init (AZValueArrayClass *klass)
{
	klass->default_size = 4;
	az_class_declare_interface ((AZClass*) klass, 0, AZ_TYPE_LIST, ARIKKEI_OFFSET(AZValueArrayClass, list_impl), ARIKKEI_OFFSET(AZValueArray, list));
	klass->list_impl.collection_impl.get_element_type = value_array_get_element_type;
	klass->list_impl.collection_impl.contains = value_array_contains;
	klass->list_impl.get_element = value_array_get_element;
}

static void
value_array_init (AZValueArrayClass *klass, AZValueArray *varray)
{
	varray->type = AZ_TYPE_ANY;
	varray->size = klass->default_size;
	varray->data_size = 0;
	varray->list.collection.size = 0;
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
	for (i = 0; i < varray->list.collection.size; i++) {
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
	AZValueArray *varray = (AZValueArray *) ARIKKEI_BASE_ADDRESS(AZValueArray, list.collection, collection_inst);
	return varray->type;
}

static unsigned int
value_array_get_size (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst)
{
	AZValueArray *varray = (AZValueArray *) ARIKKEI_BASE_ADDRESS(AZValueArray, list.collection, collection_inst);
	return varray->list.collection.size;
}

static unsigned int
value_array_contains (const AZCollectionImplementation *collection_impl, AZCollection *collection_inst, const AZImplementation *impl, const void *inst)
{
	AZValueArray *varray = (AZValueArray *) ARIKKEI_BASE_ADDRESS(AZValueArray, list.collection, collection_inst);
	unsigned int i;
	for (i = 0; i < varray->list.collection.size; i++) {
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
	AZValueArray *varray = (AZValueArray *) ARIKKEI_BASE_ADDRESS(AZValueArray, list, list_inst);
	if (varray->values[idx].impl) {
		return az_value_copy_autobox(varray->values[idx].impl, val, value_array_element_value(varray, idx), size);
	}
	return varray->values[idx].impl;
}

void
az_value_array_set_length (AZValueArray* varray, unsigned int length)
{
	unsigned int i;
	if (length < varray->list.collection.size) {
		/* Elements are removed from the back; their data-pool slots were at the
		 * pool's tail (the pool is packed in element order), so the remaining
		 * elements stay compact - no compaction needed */
		for (i = length; i < varray->list.collection.size; i++) {
			if (varray->values[i].impl) {
				az_value_clear (varray->values[i].impl, value_array_element_value (varray, i));
			}
		}
		varray->list.collection.size = length;
	} else if (length > varray->list.collection.size) {
		if (length > varray->size) {
			varray->size = length;
			varray->values = (AZValueArrayEntry *) realloc (varray->values, varray->size * sizeof (AZValueArrayEntry));
		}
		for (i = varray->list.collection.size; i < length; i++) {
			varray->values[i].impl = NULL;
		}
		varray->list.collection.size = length;
	}
}

const AZImplementation *
az_value_array_get_element(AZValueArray *varray, unsigned int idx, AZValue *val, unsigned int size)
{
	if (varray->values[idx].impl) {
		return az_value_copy_autobox(varray->values[idx].impl, val, value_array_element_value(varray, idx), size);
	}
	return varray->values[idx].impl;
}

/*
 * Rebuilds the data pool, packed in element order, reserving req16 slots for
 * element idx at its (packed) position. The content of all other big elements
 * is moved to their new positions. Returns the position of the reserved room.
 */
static unsigned int
value_array_ensure_room16 (AZValueArray* varray, unsigned int idx, unsigned int req16)
{
	unsigned int total16 = req16;
	unsigned int i;
	for (i = 0; i < varray->list.collection.size; i++) {
		if (i == idx) continue;
		if (varray->values[i].impl) {
			unsigned int size8 = value_array_element_size8 (varray, i);
			if (size8) total16 += (size8 + 1) / 2;
		}
	}
	if (!total16) {
		if (varray->data) free (varray->data);
		varray->data = NULL;
		varray->data_size = 0;
		return 0;
	}
	AZValue *newdata = (AZValue *) malloc (total16 * sizeof (AZValue));
	unsigned int pos = 0;
	unsigned int result = 0;
	for (i = 0; i < varray->list.collection.size; i++) {
		if (i == idx) {
			result = pos;
			pos += req16;
			continue;
		}
		if (varray->values[i].impl) {
			unsigned int size8 = value_array_element_size8 (varray, i);
			if (size8) {
				unsigned int size16 = (size8 + 1) / 2;
				memcpy (newdata + pos, varray->data + varray->values[i].val_idx, size16 * sizeof (AZValue));
				varray->values[i].val_idx = pos;
				pos += size16;
			}
		}
	}
	if (varray->data) free (varray->data);
	varray->data = newdata;
	varray->data_size = total16;
	return result;
}

void
az_value_array_set_element(AZValueArray *varray, unsigned int idx, const AZImplementation *impl, void *inst)
{
	unsigned int old_big = 0;
	if (varray->values[idx].impl) {
		old_big = value_array_element_size8 (varray, idx);
		az_value_clear (varray->values[idx].impl, value_array_element_value (varray, idx));
	}
	varray->values[idx].impl = impl;
	unsigned int new_big = (impl && (AZ_IMPL_VALUE_SIZE(impl) > 8)) ? 1 : 0;
	if (old_big || new_big) {
		/* Rebuild the pool with the new requirement (reclaims the old slots if any) */
		unsigned int req16 = (new_big) ? (AZ_IMPL_VALUE_SIZE(impl) + 15) >> 4 : 0;
		varray->values[idx].val_idx = value_array_ensure_room16 (varray, idx, req16);
	}
	if (impl) {
		if (!new_big) {
			az_value_set_from_inst(impl, (AZValue *) varray->values[idx].value, inst);
		} else {
			az_value_set_from_inst(impl, varray->data + varray->values[idx].val_idx, inst);
		}
	}
}

void
az_value_array_set_element_from_val (AZValueArray* varray, unsigned int idx, const AZImplementation* impl, const AZValue* val)
{
	if (impl == AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_VALUE)) {
		AZBoxedValue *boxed = (AZBoxedValue *) val->reference;
		az_value_array_set_element(varray, idx, &boxed->klass->impl, &boxed->val);
	} else {
		az_value_array_set_element(varray, idx, impl, az_value_get_inst(impl, val));
	}
}

void
az_value_array_transfer_element (AZValueArray* varray, unsigned int idx, const AZImplementation* impl, const AZValue* val)
{
	if (impl == AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_VALUE)) {
		AZBoxedValue *boxed = (AZBoxedValue *) val->reference;
		val = &boxed->val;
		impl = &boxed->klass->impl;
	}
	unsigned int old_big = 0;
	if (varray->values[idx].impl) {
		old_big = value_array_element_size8 (varray, idx);
		az_value_clear (varray->values[idx].impl, value_array_element_value (varray, idx));
	}
	varray->values[idx].impl = impl;
	unsigned int new_big = (impl && (AZ_IMPL_VALUE_SIZE(impl) > 8)) ? 1 : 0;
	if (old_big || new_big) {
		unsigned int req16 = (new_big) ? (AZ_IMPL_VALUE_SIZE(impl) + 15) >> 4 : 0;
		varray->values[idx].val_idx = value_array_ensure_room16 (varray, idx, req16);
	}
	if (impl) {
		if (!new_big) {
			az_value_transfer(impl, (AZValue *) varray->values[idx].value, val);
		} else {
			az_value_transfer(impl, varray->data + varray->values[idx].val_idx, val);
		}
	}
}




