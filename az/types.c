#define __AZ_TYPES_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-utils.h>

#include <az/class.h>
#include <az/function.h>
#include <az/interface.h>
#include <az/object.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference.h>
#include <az/string.h>
#include <az/boxed-interface.h>
#ifdef AZ_HAS_PACKED_VALUE
#include <az/packed-value.h>
#endif
#ifdef AZ_HAS_PROPERTIES
#include <az/field.h>
#endif

#include <az/types.h>

static void instance_init_recursive (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int zeroed);
static void instance_finalize_recursive (const AZClass *klass, const AZImplementation *impl, void *inst);
static void implementation_init_recursive (AZInterfaceClass *iface_class, AZImplementation *impl);

void
az_init (void)
{
	if (az_classes) return;
	az_classes_init();
	az_init_primitive_classes();
	az_init_implementation_class();
	az_class_class_init();
	az_init_interface_class();
#ifdef AZ_HAS_PROPERTIES
	az_init_field_class();
#endif
	az_init_reference_class();
	az_init_function_classes();
	az_init_string_class();
	az_init_boxed_interface_class();
#ifdef AZ_HAS_PACKED_VALUE
	az_init_packed_value_class();
#endif
	az_num_classes = AZ_NUM_BASE_TYPES;

	az_post_init_primitive_classes();
    az_implementation_class_post_init();
	az_class_class_post_init();
}

unsigned int
az_type_get_parent_primitive (unsigned int type)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (type < az_num_classes, 0);
#endif
	if (AZ_TYPE_INDEX(type) < AZ_NUM_PRIMITIVE_TYPES) return type;
	klass = az_classes[AZ_TYPE_INDEX(type)]->parent;
	while (AZ_TYPE_INDEX(klass->implementation.type) >= AZ_NUM_PRIMITIVE_TYPES) {
		klass = klass->parent;
	}
	return klass->implementation.type;
}

unsigned int
az_type_is_a (unsigned int type, unsigned int test)
{
	AZClass *klass;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (type < az_num_classes, 0);
	arikkei_return_val_if_fail (test < az_num_classes, 0);
#endif
	if (!type) return 0;
	if (type == test) return 1;
	test = AZ_TYPE_INDEX(test);
	uint32_t idx = az_types[AZ_TYPE_INDEX(type)].pidx;
	while (idx) {
		if (idx == test) return 1;
		idx = az_types[idx].pidx;
	}
	return 0;
}

unsigned int
az_type_implements (unsigned int type, unsigned int test)
{
	if (!type) return 0;
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (type < az_num_classes, 0);
	arikkei_return_val_if_fail (test < az_num_classes, 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE(test), 0);
#endif
	if (!type) return 0;
	return az_get_interface (&AZ_CLASS_FROM_TYPE(type)->implementation, NULL, test, NULL) != NULL;
}

unsigned int
az_type_is_assignable_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_classes, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) != 0, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) < az_num_classes, 0);
#endif
	if (!type) {
		/* None can be assigned to any */
		if (test == AZ_TYPE_ANY) return 1;
		/* None can be assigned to blocks */
		if (AZ_TYPE_IS_BLOCK(test)) return 1;
	}
	if (az_type_is_a (type, test)) return 1;
	if (AZ_TYPE_IS_INTERFACE(test)) {
		return az_type_implements (type, test);
	}
	return 0;
}

unsigned int
az_type_is_convertible_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_classes, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) != 0, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) < az_num_classes, 0);
#endif
	if (az_type_is_assignable_to (type, test)) return 1;
	/* Only arithmetic types have autoconversion */
	if (AZ_TYPE_IS_ARITHMETIC(type) && AZ_TYPE_IS_ARITHMETIC(test)) {
		if (az_primitive_can_convert(test, type) == AZ_CONVERT_AUTO) return 1;
		fprintf (stderr, "Convert %s -> %s\n", AZ_CLASS_FROM_TYPE(type)->name, AZ_CLASS_FROM_TYPE(test)->name);
		return 1;
	}
	return 0;
}

void
az_instance_init (void *inst, unsigned int type)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (inst != NULL);
	arikkei_return_if_fail (az_type_is_valid(type));
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	AZClass *klass = AZ_CLASS_FROM_TYPE(type);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!(klass->flags & AZ_FLAG_ABSTRACT));
#endif
	if (klass->flags & AZ_CLASS_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	instance_init_recursive (klass, &klass->implementation, inst, klass->flags & AZ_CLASS_ZERO_MEMORY);
}

void
az_instance_finalize (void *inst, unsigned int type)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (inst != NULL);
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_classes);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	AZClass *klass = AZ_CLASS_FROM_TYPE(type);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!(klass->flags & AZ_FLAG_ABSTRACT));
#endif
	instance_finalize_recursive (klass, &klass->implementation, inst);
}

void
az_implementation_init (AZImplementation *impl, unsigned int type)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_classes);
	arikkei_return_if_fail (az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	AZInterfaceClass *ifclass = (AZInterfaceClass *) az_type_get_class (type);
	impl->type = type;
	implementation_init_recursive (ifclass, impl);
}

void
az_interface_init (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
	arikkei_return_if_fail (AZ_TYPE_INDEX(impl->type) != 0);
	arikkei_return_if_fail (AZ_TYPE_INDEX(impl->type) < az_num_classes);
	arikkei_return_if_fail (AZ_TYPE_IS_INTERFACE(impl->type));
#endif
	AZClass *klass = AZ_CLASS_FROM_TYPE(impl->type);
	if (klass->flags & AZ_CLASS_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	instance_init_recursive (klass, impl, inst, klass->flags & AZ_CLASS_ZERO_MEMORY);
}

void
az_interface_finalize (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
	arikkei_return_if_fail (AZ_TYPE_INDEX(impl->type) != 0);
	arikkei_return_if_fail (AZ_TYPE_INDEX(impl->type) < az_num_classes);
	arikkei_return_if_fail (AZ_TYPE_IS_INTERFACE(impl->type));
#endif
	AZClass *klass = AZ_CLASS_FROM_TYPE(impl->type);
	instance_finalize_recursive (klass, impl, inst);
}

void *
az_instance_new (unsigned int type)
{
	AZClass *klass;
	void *inst;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) != 0, NULL);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_classes, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = az_type_get_class (type);
	arikkei_return_val_if_fail (!(klass->flags & AZ_FLAG_ABSTRACT), NULL);
	if (klass->allocator && klass->allocator->allocate) {
		inst = klass->allocator->allocate (klass);
	} else {
		inst = malloc (klass->instance_size);
	}
	if (klass->flags & AZ_CLASS_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	instance_init_recursive (klass, &klass->implementation, inst, klass->flags & AZ_CLASS_ZERO_MEMORY);
	return inst;
}

void *
az_instance_new_array (unsigned int type, unsigned int nelements)
{
	AZClass *klass;
	void *elements;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (type != 0, NULL);
	arikkei_return_val_if_fail (type < az_num_classes, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = az_type_get_class (type);
	if (klass->allocator && klass->allocator->allocate_array) {
		elements = klass->allocator->allocate_array (klass, nelements);
	} else {
		elements = malloc (nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	}
	if (klass->flags & AZ_CLASS_ZERO_MEMORY) memset (elements, 0, nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		instance_init_recursive (klass, &klass->implementation, instance, klass->flags & AZ_CLASS_ZERO_MEMORY);
	}
	return elements;
}

void
az_instance_delete (unsigned int type, void *instance)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_classes);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	instance_finalize_recursive (klass, &klass->implementation, instance);
	if (klass->allocator && klass->allocator->free) {
		klass->allocator->free (klass, instance);
	} else {
		free (instance);
	}
}

void
az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements)
{
	AZClass *klass;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_classes);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		instance_finalize_recursive (klass, &klass->implementation, instance);
	}
	if (klass->allocator && klass->allocator->free_array) {
		klass->allocator->free_array (klass, elements, nelements);
	} else {
		free (elements);
	}
}

const AZImplementation *
az_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	AZImplementation *sub_impl;
	unsigned int i;
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (impl->type == AZ_TYPE_BOXED_INTERFACE) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	if (az_type_is_a (impl->type, if_type)) {
		if (if_inst) *if_inst = inst;
		return impl;
	}
	AZClass *klass = AZ_CLASS_FROM_TYPE(impl->type);
	AZIFEntry *ifentry = az_class_iface_all(klass, 0);
	for (i = 0; i < klass->n_ifaces_all; i++) {
		if (az_type_is_a(ifentry->type, if_type)) {
			sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
			if (if_inst) *if_inst = (char *) inst + ifentry->inst_offset;
			return sub_impl;
		}
		ifentry += 1;
	}
	if (if_inst) *if_inst = NULL;
	return NULL;
}

unsigned int
az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	klass = az_type_get_class (impl->type);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return (klass->serialize) ? klass->serialize (impl, inst, d, dlen, ctx) : 0;
}

unsigned int
az_deserialize_value (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (value != NULL, 0);
#endif
	klass = az_type_get_class (impl->type);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return (klass->deserialize) ? klass->deserialize (impl, value, s, slen, ctx) : 0;
}

/* fixme: Make signature correct */

unsigned int
az_instance_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZClass* klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	klass = az_type_get_class (impl->type);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return klass->to_string (&klass->implementation, inst, buf, len);
}

AZClass *
az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	assert (!parent_type || (class_size >= az_classes[AZ_TYPE_INDEX(parent_type)]->class_size));
	assert (!parent_type || (instance_size >= az_classes[AZ_TYPE_INDEX(parent_type)]->instance_size));
#endif
	AZClass *klass = az_class_new (type, name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	arikkei_return_val_if_fail (*type, NULL);
	if (class_init) class_init (klass);
	az_class_post_init (klass);
	return klass;
}

AZClass *
az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	assert (!parent_type || (class_size >= az_classes[AZ_TYPE_INDEX(parent_type)]->class_size));
	assert (!parent_type || (instance_size >= az_classes[AZ_TYPE_INDEX(parent_type)]->instance_size));
#endif
	AZClass *klass = az_class_new (type, name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	arikkei_return_val_if_fail (*type, NULL);
	if (class_init) class_init (klass, data);
	az_class_post_init (klass);
	return klass;
}

/*
 * class - the actual class
 * impl - the actual implementation (same as class for non-interface types)
 *
 * 1. Initializes parent type
 * 2. Initializes all local interfaces
 * 3. instance_init
 */

static void
instance_init_recursive (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int zeroed)
{
	/* Every interface has to be subclass of AZInterface */
	if (klass->parent && (AZ_TYPE_INDEX(klass->parent->implementation.type) >= AZ_NUM_PRIMITIVE_TYPES)) {
		instance_init_recursive (klass->parent, impl, inst, zeroed);
	}
	/* Interfaces */
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		if (!zeroed && (sub_class->flags & AZ_CLASS_ZERO_MEMORY)) memset (sub_inst, 0, sub_class->instance_size);
		instance_init_recursive (sub_class, sub_impl, sub_inst, zeroed || (sub_class->flags & AZ_CLASS_ZERO_MEMORY));
		ifentry += 1;
	}
	/* Instance itself */
	if (klass->instance_init) klass->instance_init (impl, inst);
}

/*
 * class - the actual class
 * impl - the actual implementation (same as class for non-interface types)
 *
 * 1. instance_finalize
 * 2. Finalizes all local interfaces
 * 3. Finalizes parent type
 */

static void
instance_finalize_recursive (const AZClass *klass, const AZImplementation *impl, void *inst)
{
	if (klass->instance_finalize) klass->instance_finalize (impl, inst);
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		instance_finalize_recursive (sub_class, sub_impl, sub_inst);
		ifentry += 1;
	}
	if (klass->parent && (AZ_TYPE_INDEX(klass->parent->implementation.type) >= AZ_NUM_PRIMITIVE_TYPES)) {
		instance_finalize_recursive (klass->parent, impl, inst);
	}
}

static void
implementation_init_recursive (AZInterfaceClass *iface_class, AZImplementation *impl)
{
	AZClass *klass = (AZClass *) iface_class;
	/* Init superimplementations */
	if (klass->parent && (klass->parent->implementation.type >= AZ_TYPE_INTERFACE)) {
		implementation_init_recursive ((AZInterfaceClass *) klass->parent, impl);
	}
	/* Init subimplementations */
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZInterfaceClass *sub_class = (AZInterfaceClass *) AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		/* fixme: Why do we zero implementation? (Lauris) */
		//if (sub_class->klass.flags & AZ_CLASS_ZERO_MEMORY) {
		//	memset (sub_impl, 0, sub_class->implementation_size);
		//}
		sub_impl->type = ifentry->type;
		implementation_init_recursive (sub_class, sub_impl);
		ifentry += 1;
	}
	/* Implementation itself */
	if (iface_class->implementation_init) iface_class->implementation_init (impl);
}

#ifdef AZ_HAS_PROPERTIES
int
az_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop)
{
	int result, i;
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
#if 0
	if (!strcmp (klass->name, "AosoraImage")) {
		fprintf (stderr, ".");
	}
#endif
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (i = 0; i < (int) klass->n_properties_self; i++) {
		if (!strcmp ((const char *) key, (const char *) klass->properties_self[i].key->str)) {
			*def_class = klass;
			*def_impl = impl;
			if (def_inst) *def_inst = inst;
			if (prop) *prop = &klass->properties_self[i];
			return i;
		}
	}
	/* interfaces */
	for (i = 0; i < (int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		/* Check properties of this interface */
		result = az_lookup_property (AZ_CLASS_FROM_TYPE(sub_impl->type), sub_impl, sub_inst, key, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_lookup_property (klass->parent, impl, inst, key, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	return -1;
}

int
az_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop)
{
	int result, i;
	arikkei_return_val_if_fail (impl != NULL, -1);
	arikkei_return_val_if_fail (key != NULL, -1);
	/* NB! Until "new" is handled differently we have to go subclass-first */
	for (i = 0; i < (int) klass->n_properties_self; i++) {
		if (!strcmp ((const char *) key, (const char *) klass->properties_self[i].key->str) && klass->properties_self->is_function) {
			if (klass->properties_self[i].signature && !az_function_signature_is_assignable_to (klass->properties_self[i].signature, sig, 0)) {
				continue;
			}
			*def_class = klass;
			*def_impl = impl;
			if (def_inst) *def_inst = inst;
			if (prop) *prop = &klass->properties_self[i];
			return i;
		}
	}
	/* interfaces */
	for (i = 0; i < ( int) klass->n_ifaces_self; i++) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, i);
		AZImplementation *c_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *c_inst = (void *) ((char *) inst + ifentry->inst_offset);
		/* Check properties of this interface */
		result = az_lookup_function (AZ_CLASS_FROM_TYPE(c_impl->type), c_impl, c_inst, key, sig, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	/* Superclass */
	if (klass->parent) {
		result = az_lookup_function (klass->parent, impl, inst, key, sig, def_class, def_impl, def_inst, prop);
		if (result >= 0) return result;
	}
	return -1;
}

unsigned int
az_instance_set_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	idx = az_lookup_property (AZ_CLASS_FROM_TYPE(impl->type), impl, inst, key, &sub_class, &sub_impl, &sub_inst, NULL);
	if (idx < 0) return 0;
	return az_instance_set_property_by_id (sub_class, sub_impl, sub_inst, idx, prop_impl, prop_inst, ctx);
}

unsigned int
az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (!klass->properties_self[idx].is_final, 0);
	arikkei_return_val_if_fail (klass->properties_self[idx].write != AZ_FIELD_WRITE_NONE, 0);
	if (klass->properties_self[idx].is_interface) {
		if (prop_impl && !az_type_implements (prop_impl->type, klass->properties_self[idx].type)) {
			fprintf (stderr, ".");
		}
		arikkei_return_val_if_fail (!prop_impl || az_type_implements (prop_impl->type, klass->properties_self[idx].type), 0);
		if (klass->properties_self[idx].is_function && prop_impl) {
			AZFunctionInstance *func_inst;
			const AZFunctionImplementation *func_impl = (const AZFunctionImplementation *) az_get_interface (prop_impl, prop_inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
			const AZFunctionSignature *sig = az_function_get_signature (func_impl, func_inst);
			if (klass->properties_self[idx].signature && !az_function_signature_is_assignable_to (sig, klass->properties_self[idx].signature, 1)) {
				fprintf (stderr, ".");
			}
			arikkei_return_val_if_fail (!klass->properties_self[idx].signature || az_function_signature_is_assignable_to (sig, klass->properties_self[idx].signature, 1), 0);
		}
	} else {
		arikkei_return_val_if_fail (!prop_impl || az_type_is_a (prop_impl->type, klass->properties_self[idx].type), 0);
	}
	if (klass->properties_self[idx].write == AZ_FIELD_WRITE_VALUE) {
		AZValue *val;
		if (klass->properties_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZValue *) ((char *) inst + klass->properties_self[idx].offset);
		} else if (klass->properties_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZValue *) ((char *) impl + klass->properties_self[idx].offset);
		} else {
			val = (AZValue *) ((char *) klass + klass->properties_self[idx].offset);
		}
		az_set_value_from_instance (prop_impl, val, prop_inst);
	} else if (klass->properties_self[idx].write == AZ_FIELD_WRITE_PACKED) {
		AZPackedValue *val;
		if (klass->properties_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZPackedValue *) ((char *) inst + klass->properties_self[idx].offset);
		} else if (klass->properties_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZPackedValue *) ((char *) impl + klass->properties_self[idx].offset);
		} else {
			val = (AZPackedValue *) ((char *) klass + klass->properties_self[idx].offset);
		}
		az_packed_value_set_from_impl_instance (val, prop_impl, prop_inst);
	} else if (klass->properties_self[idx].write == AZ_FIELD_WRITE_METHOD) {
		return klass->set_property (impl, inst, idx, prop_impl, prop_inst, NULL);
	}
	return 1;
}

unsigned int
az_instance_get_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	idx = az_lookup_property (AZ_CLASS_FROM_TYPE(impl->type), impl, inst, key, &sub_class, &sub_impl, &sub_inst, NULL);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (sub_class, sub_impl, sub_inst, idx, dst_impl, dst_val, NULL);
}

unsigned int
az_instance_get_function (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	idx = az_lookup_function (AZ_CLASS_FROM_TYPE(impl->type), impl, inst, key, sig, &sub_class, &sub_impl, &sub_inst, NULL);
	if (idx < 0) return 0;
	az_instance_get_property_by_id (sub_class, sub_impl, sub_inst, idx, dst_impl, (AZValue64 *) dst_val, NULL);
	return 1;
}

unsigned int
az_instance_get_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue64 *prop_val, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (prop_impl != NULL, 0);
	arikkei_return_val_if_fail (prop_val != NULL, 0);
	arikkei_return_val_if_fail (klass->properties_self[idx].read != AZ_FIELD_READ_NONE, 0);
	if (klass->properties_self[idx].read == AZ_FIELD_READ_VALUE) {
		/* Bare value inside instance */
		AZValue *val;
		if (klass->properties_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZValue *) ((char *) inst + klass->properties_self[idx].offset);
		} else if (klass->properties_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZValue *) ((char *) impl + klass->properties_self[idx].offset);
		} else {
			val = (AZValue *) ((char *) klass + klass->properties_self[idx].offset);
		}
		if (az_type_is_a (klass->properties_self[idx].type, AZ_TYPE_OBJECT)) {
			az_value_set_object (prop_impl, &prop_val->value, *((AZObject **) val));
		} else {
			AZClass *prop_class = AZ_CLASS_FROM_TYPE(klass->properties_self[idx].type);
			arikkei_return_val_if_fail (prop_class->flags & AZ_FLAG_FINAL, 0);
			*prop_impl = &prop_class->implementation;
			az_copy_value (&prop_class->implementation, &prop_val->value, val);
		}
	} else if (klass->properties_self[idx].read == AZ_FIELD_READ_STORED_STATIC) {
		*prop_impl = klass->properties_self[idx].val.impl;
		az_copy_value (klass->properties_self[idx].val.impl, &prop_val->value, &klass->properties_self[idx].val.v.value);
	} else if (klass->properties_self[idx].read == AZ_FIELD_READ_PACKED) {
		AZPackedValue *val;
		if (klass->properties_self[idx].spec == AZ_FIELD_INSTANCE) {
			val = (AZPackedValue *) ((char *) inst + klass->properties_self[idx].offset);
		} else if (klass->properties_self[idx].spec == AZ_FIELD_IMPLEMENTATION) {
			val = (AZPackedValue *) ((char *) impl + klass->properties_self[idx].offset);
		} else {
			val = (AZPackedValue *) ((char *) klass + klass->properties_self[idx].offset);
		}
		az_value_set_from_packed_value (prop_impl, &prop_val->value, val);
	} else if (klass->properties_self[idx].read == AZ_FIELD_READ_METHOD) {
		return klass->get_property (impl, inst, idx, prop_impl, &prop_val->value, ctx);
	} else if (klass->properties_self[idx].read == AZ_FIELD_READ_BOXED_INTERFACE) {
		AZClass *prop_class = AZ_CLASS_FROM_TYPE(klass->properties_self[idx].type);
		arikkei_return_val_if_fail (prop_class->flags & AZ_CLASS_IS_INTERFACE, 0);
		AZBoxedInterface *boxed = az_boxed_interface_new (impl, inst, (const AZImplementation *) ((char *) impl + klass->properties_self[idx].offset), (char *) inst + klass->properties_self[idx].offset);
		*prop_impl = (const AZImplementation *) az_boxed_interface_class;
		az_value_transfer_reference (&prop_val->value, (AZReference *) boxed);
	} else {
		/* Not readable */
		return 0;
	}
	return 1;
}
#endif
