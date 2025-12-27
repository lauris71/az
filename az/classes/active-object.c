#define __AZ_ACTIVE_OBJECT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/boxed-value.h>
#include <az/packed-value.h>
#include <az/string.h>
#include <az/extend.h>

#include <az/classes/active-object.h>

static void az_active_object_class_init (AZActiveObjectClass *klass);

/* AZObject implementation */
static void az_active_object_shutdown (AZObject *object);
/* Attribute array */
static unsigned int aobj_aa_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst);
static unsigned int aobj_aa_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *aobj_aa_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);
static const AZImplementation *aobj_aa_map_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size);
static unsigned int aobj_aa_keys_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst);
static const AZImplementation *aobj_aa_keys_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size);

const AZImplementation *aobj_attrd_lookup (const AZAttribDictImplementation *aa_impl, void *aa_inst, const AZString *key, AZValue *val, int size, unsigned int *flags);
unsigned int aobj_attrd_set (const AZAttribDictImplementation *aa_impl, void *aa_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags);

/* Method implementations */
static unsigned int active_object_call_setAttribute (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
static unsigned int active_object_call_getAttribute (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

/* Properties */

enum {
	FUNC_SETATTRIBUTE,
	FUNC_GETATTRIBUTE,
	NUM_PROPERTIES
};

unsigned int
az_active_object_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		az_register_type (&type, (const unsigned char *) "AZActiveObject", AZ_TYPE_OBJECT, sizeof (AZActiveObjectClass), sizeof (AZActiveObject), AZ_FLAG_ABSTRACT,
			(void (*) (AZClass *)) az_active_object_class_init, NULL, NULL);
	}
	return type;
}

static void
az_active_object_class_init (AZActiveObjectClass *klass)
{
	az_class_set_num_interfaces ((AZClass *) klass, 1);
	az_class_declare_interface ((AZClass *) klass, 0, AZ_TYPE_ATTRIBUTE_DICT, ARIKKEI_OFFSET(AZActiveObjectClass,aa_impl), 0);
	/* AZClass implementation */
	az_class_set_num_properties ((AZClass *) klass, NUM_PROPERTIES);
	az_class_define_method_va ((AZClass *) klass, FUNC_SETATTRIBUTE, (const unsigned char *) "setAttribute", active_object_call_setAttribute, AZ_TYPE_NONE, 2, AZ_TYPE_STRING, AZ_TYPE_ANY);
	az_class_define_method_va ((AZClass *) klass, FUNC_GETATTRIBUTE, (const unsigned char *) "getAttribute", active_object_call_getAttribute, AZ_TYPE_ANY, 1, AZ_TYPE_STRING);
	/* AZObject implementation */
	((AZObjectClass *) klass)->shutdown = az_active_object_shutdown;
	/* Attribute array */
	klass->aa_impl.map_impl.collection_impl.get_size = aobj_aa_get_size;
	klass->aa_impl.map_impl.collection_impl.contains = aobj_aa_contains;
	klass->aa_impl.val_list_impl.get_element = aobj_aa_get_element;
	klass->aa_impl.key_list_impl.collection_impl.contains = aobj_aa_keys_contains;
	klass->aa_impl.key_list_impl.get_element = aobj_aa_keys_get_element;
	klass->aa_impl.lookup = aobj_attrd_lookup;
	klass->aa_impl.set = aobj_attrd_set;
}

static unsigned int
active_object_call_setAttribute (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	if (!arg_impls[1]) return 0;
	az_active_object_set_attribute ((AZActiveObject *) arg_vals[0]->reference, arg_vals[1]->string, arg_impls[2], arg_vals[2]);
	return 1;
}

static unsigned int
active_object_call_getAttribute (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	if (!arg_impls[1]) return 0;
	if (!az_active_object_get_attribute (( AZActiveObject *) arg_vals[0]->reference, arg_vals[1]->string, ret_impl, ret_val)) {
		*ret_impl = NULL;
	}
	return 1;
}

static void
az_active_object_shutdown (AZObject *object)
{
	AZActiveObject *aobj = (AZActiveObject *) object;
	if (aobj->callbacks) {
		unsigned int i;
		for (i = 0; i < aobj->callbacks->length; i++) {
			AZObjectListener *listener;
			listener = aobj->callbacks->listeners + i;
			if (listener->vector->dispose) listener->vector->dispose (aobj, listener->data);
		}
		free (aobj->callbacks);
		aobj->callbacks = NULL;
	}
	if (aobj->attributes) {
		unsigned int i;
		for (i = 0; i < aobj->attributes->length; i++) {
			az_string_unref (aobj->attributes->attribs[i].key);
			az_packed_value_clear (&aobj->attributes->attribs[i].value.packed_val);
		}
		free (aobj->attributes);
		aobj->attributes = NULL;
	}
}

static AZObjectAttribute *
az_active_object_get_attribute_slot (AZActiveObject *aobj, AZString *key, unsigned int create)
{
	if (aobj->attributes) {
		unsigned int i;
		for (i = 0; i < aobj->attributes->length; i++) {
			if (aobj->attributes->attribs[i].key == key) {
				return &aobj->attributes->attribs[i];
			}
		}
	}
	if (!create) return NULL;
	if (!aobj->attributes) {
		aobj->attributes = (AZObjectAttributeArray *) malloc (sizeof (AZObjectAttributeArray) + 3 * sizeof (AZObjectAttribute));
		memset (aobj->attributes, 0, sizeof (AZObjectAttributeArray) + 3 * sizeof (AZObjectAttribute));
		aobj->attributes->size = 4;
		aobj->attributes->length = 0;
	} else if (aobj->attributes->length >= aobj->attributes->size) {
		aobj->attributes->size <<= 1;
		aobj->attributes = (AZObjectAttributeArray *) realloc (aobj->attributes, sizeof (AZObjectAttributeArray) + (aobj->attributes->size - 1) * sizeof (AZObjectAttribute));
		memset (&aobj->attributes->attribs[aobj->attributes->length], 0, (aobj->attributes->size - aobj->attributes->length) * sizeof (AZObjectAttribute));
	}
	az_string_ref (key);
	aobj->attributes->attribs[aobj->attributes->length].key = key;
	return &aobj->attributes->attribs[aobj->attributes->length++];
}

unsigned int
az_active_object_get_attribute (AZActiveObject *aobj, AZString *key, const AZImplementation **impl, AZValue64 *val)
{
	arikkei_return_val_if_fail (AZ_IS_ACTIVE_OBJECT (aobj), 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
	AZActiveObjectClass *klass = (AZActiveObjectClass *) aobj->object.klass;
	unsigned int flags;
	*impl = az_attrib_dict_lookup (&klass->aa_impl, aobj, key, val, &flags);
	return impl != NULL;
}

unsigned int
az_active_object_set_attribute (AZActiveObject *aobj, AZString *key, const AZImplementation *impl, const AZValue *val)
{
	AZActiveObjectClass *klass = (AZActiveObjectClass *) aobj->object.klass;
	return az_attrib_dict_set (&klass->aa_impl, aobj, key, impl, (impl) ? az_value_get_inst(impl, val) : NULL, 0);
}

unsigned int
az_active_object_clear_attribute (AZActiveObject *aobj, AZString *key)
{
	unsigned int i;
	arikkei_return_val_if_fail (AZ_IS_ACTIVE_OBJECT (aobj), 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	if (!aobj->attributes) return 0;
	for (i = 0; i < aobj->attributes->length; i++) {
		if (aobj->attributes->attribs[i].key == key) {
			az_string_unref (aobj->attributes->attribs[i].key);
			az_packed_value_clear (&aobj->attributes->attribs[i].value.packed_val);
			if ((i + 1) < aobj->attributes->length) {
				memcpy (&aobj->attributes->attribs[i], &aobj->attributes->attribs[i + 1], (aobj->attributes->length - (i + 1)) * sizeof (AZObjectAttribute));
			}
			aobj->attributes->length -= 1;
			if (!aobj->attributes->length) {
				free (aobj->attributes);
				aobj->attributes = NULL;
			}
			return 1;
		}
	}
	return 0;
}

void
az_active_object_add_listener (AZActiveObject *aobj, const AZObjectEventVector *vector, unsigned int size, void *data)
{
	AZObjectListener *listener;

	if (!aobj->callbacks) {
		aobj->callbacks = (AZObjectCallbackBlock *) malloc (sizeof (AZObjectCallbackBlock));
		aobj->callbacks->size = 1;
		aobj->callbacks->length = 0;
	}
	if (aobj->callbacks->length >= aobj->callbacks->size) {
		int newsize;
		newsize = aobj->callbacks->size << 1;
		aobj->callbacks = (AZObjectCallbackBlock *) realloc (aobj->callbacks, sizeof (AZObjectCallbackBlock) + (newsize - 1) * sizeof (AZObjectListener));
		aobj->callbacks->size = newsize;
	}
	listener = aobj->callbacks->listeners + aobj->callbacks->length;
	listener->vector = vector;
	listener->size = size;
	listener->data = data;
	aobj->callbacks->length += 1;
}

void
az_active_object_remove_listener_by_data (AZActiveObject *aobj, void *data)
{
	if (aobj->callbacks) {
		unsigned int i;
		for (i = 0; i < aobj->callbacks->length; i++) {
			AZObjectListener *listener;
			listener = aobj->callbacks->listeners + i;
			if (listener->data == data) {
				aobj->callbacks->length -= 1;
				if (aobj->callbacks->length < 1) {
					free (aobj->callbacks);
					aobj->callbacks = NULL;
				} else if (aobj->callbacks->length != i) {
					*listener = aobj->callbacks->listeners[aobj->callbacks->length];
				}
				return;
			}
		}
	}
}

unsigned int
az_active_object_set_attribute_i32 (AZActiveObject *aobj, const unsigned char *key, int value)
{
	AZString *str = az_string_new (key);
	unsigned int result = az_active_object_set_attribute (aobj, str, az_type_get_impl(AZ_TYPE_INT32), (const AZValue *) &value);
	az_string_unref (str);
	return result;
}

unsigned int
az_active_object_set_attribute_object (AZActiveObject *aobj, const unsigned char *key, AZObject *value)
{
	AZString *str = az_string_new (key);
	unsigned int result = az_active_object_set_attribute (aobj, str, (const AZImplementation *) value->klass, (const AZValue *) &value);
	az_string_unref (str);
	return result;
}

static unsigned int
aobj_aa_get_size (const AZCollectionImplementation *coll_impl, void *coll_inst)
{
	AZActiveObject *aobj = (AZActiveObject *) ARIKKEI_BASE_ADDRESS(AZActiveObjectClass,aa_impl,coll_impl);
	if (!aobj->attributes) return 0;
	return aobj->attributes->length;
}

static unsigned int
aobj_aa_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst)
{
	unsigned int i;
	AZActiveObject *aobj = (AZActiveObject *) coll_inst;
	if (!aobj->attributes) return 0;
	for (i = 0; i < aobj->attributes->length; i++) {
		if (az_value_equals_instance (impl, &aobj->attributes->attribs[i].value.v.value, inst)) return 1;
	}
	return 0;
}

static const AZImplementation *
aobj_aa_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZActiveObject *aobj = (AZActiveObject *) list_inst;
	if (!aobj->attributes || (aobj->attributes->length <= idx)) return NULL;
	AZPackedValue64 *pval64 = &aobj->attributes->attribs[idx].value;
	return az_value_copy_autobox (pval64->impl, val, &pval64->v.value, size);
}

static const AZImplementation *
aobj_aa_map_lookup (const AZMapImplementation *map_impl, void *map_inst, const AZImplementation *key_impl, void *key_inst, AZValue *val, unsigned int size)
{
	unsigned int i;
	if (AZ_IMPL_TYPE(key_impl) != AZ_TYPE_STRING) return NULL;
	AZActiveObject *aobj = (AZActiveObject *) map_inst;
	if (!aobj->attributes) return 0;
	for (i = 0; i < aobj->attributes->length; i++) {
		if (aobj->attributes->attribs[i].key == (AZString *) key_inst) {
			return az_value_copy_autobox (aobj->attributes->attribs[i].value.impl, val, &aobj->attributes->attribs[i].value.v.value, size);
		}
	}
	return NULL;
}

static unsigned int
aobj_aa_keys_contains (const AZCollectionImplementation *coll_impl, void *coll_inst, const AZImplementation *impl, const void *inst)
{
	unsigned int i;
	if (AZ_IMPL_TYPE(impl) != AZ_TYPE_STRING) return 0;
	AZActiveObject *aobj = (AZActiveObject *) coll_inst;
	if (!aobj->attributes) return 0;
	for (i = 0; i < aobj->attributes->length; i++) {
		if (aobj->attributes->attribs[i].key == (AZString *) inst) return 1;
	}
	return 0;
}

static const AZImplementation *
aobj_aa_keys_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZActiveObject *aobj = (AZActiveObject *) list_inst;
	if (!aobj->attributes || (aobj->attributes->length <= idx)) return NULL;
	az_value_set_reference (val, &aobj->attributes->attribs[idx].key->reference);
	return (AZImplementation *) &AZStringKlass;
}

const AZImplementation *
aobj_attrd_lookup (const AZAttribDictImplementation *aa_impl, void *aa_inst, const AZString *key, AZValue *val, int size, unsigned int *flags)
{
	unsigned int i;
	AZActiveObject *aobj = (AZActiveObject *) aa_inst;
	*flags = 0;
	if (!aobj->attributes) return NULL;
	for (i = 0; i < aobj->attributes->length; i++) {
		if (aobj->attributes->attribs[i].key == key) {
			AZPackedValue64 *pval64 = &aobj->attributes->attribs[i].value;
			return az_value_copy_autobox (pval64->impl, val, &pval64->v.value, 64);
		}
	}
	return NULL;
}

unsigned int
aobj_attrd_set (const AZAttribDictImplementation *aa_impl, void *aa_inst, AZString *key, const AZImplementation *impl, void *inst, unsigned int flags)
{
	AZObjectAttribute *attr;
	AZActiveObject *aobj = (AZActiveObject *) aa_inst;
	if (!impl) return az_active_object_clear_attribute (aobj, key);
	attr = az_active_object_get_attribute_slot (aobj, key, 1);
	az_packed_value_set_from_impl_instance (&attr->value.packed_val, impl, inst);
	return 1;
}
