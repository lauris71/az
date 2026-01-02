#define __AZ_INSTANCE_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdlib.h>
#include <string.h>

#include <az/boxed-interface.h>
#include <az/class.h>
#include <az/instance.h>
#include <az/types.h>

 /*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. Initializes parent type
 * 2. Initializes all local interfaces
 * 3. instance_init
 */

void
az_instance_init_recursive (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int zeroed)
{
	/* Fundamental tyles do not have constructors */
	if (klass->parent && (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass->parent)) >= AZ_NUM_FUNDAMENTAL_TYPES)) {
		az_instance_init_recursive (klass->parent, impl, inst, zeroed);
	}
	/* Interfaces */
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		if (!zeroed && (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY)) memset (sub_inst, 0, sub_class->instance_size);
		az_instance_init_recursive (sub_class, sub_impl, sub_inst, zeroed || (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY));
		ifentry += 1;
	}
	/* Instance itself */
	if (klass->instance_init) klass->instance_init (impl, inst);
}

/*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. instance_finalize
 * 2. Finalizes all local interfaces
 * 3. Finalizes parent type
 */

void
az_instance_finalize_recursive (const AZClass *klass, const AZImplementation *impl, void *inst)
{
	if (klass->instance_finalize) klass->instance_finalize (impl, inst);
	const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
		AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
		AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
		void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
		az_instance_finalize_recursive (sub_class, sub_impl, sub_inst);
		ifentry += 1;
	}
	if (klass->parent && (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass->parent)) >= AZ_NUM_FUNDAMENTAL_TYPES)) {
		az_instance_finalize_recursive (klass->parent, impl, inst);
	}
}

void
az_instance_init (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail(impl != NULL);
	arikkei_return_if_fail(inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	if (klass->impl.flags & AZ_FLAG_CONSTRUCT) az_instance_init_recursive (klass, impl, inst, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
}

void
az_instance_finalize (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	if (klass->impl.flags & AZ_FLAG_CONSTRUCT) az_instance_finalize_recursive (klass, impl, inst);
}

void *
az_instance_new (unsigned int type)
{
	AZClass *klass;
	void *inst;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) != 0, NULL);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = AZ_CLASS_FROM_TYPE(type);
	arikkei_return_val_if_fail (!(klass->impl.flags & AZ_FLAG_ABSTRACT), NULL);
	if (klass->allocator && klass->allocator->allocate) {
		inst = klass->allocator->allocate (klass);
	} else {
		inst = malloc (klass->instance_size);
	}
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	az_instance_init_recursive (klass, &klass->impl, inst, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
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
	arikkei_return_val_if_fail (type < az_num_types, NULL);
	arikkei_return_val_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE), NULL);
#endif
	klass = az_type_get_class (type);
	if (klass->allocator && klass->allocator->allocate_array) {
		elements = klass->allocator->allocate_array (klass, nelements);
	} else {
		elements = malloc (nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	}
	if (klass->impl.flags & AZ_FLAG_ZERO_MEMORY) memset (elements, 0, nelements * AZ_CLASS_ELEMENT_SIZE(klass));
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		az_instance_init_recursive (klass, &klass->impl, instance, klass->impl.flags & AZ_FLAG_ZERO_MEMORY);
	}
	return elements;
}

void
az_instance_delete (unsigned int type, void *instance)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (type != 0);
	arikkei_return_if_fail (type < az_num_types);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	az_instance_finalize_recursive (klass, &klass->impl, instance);
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
	arikkei_return_if_fail (type < az_num_types);
	arikkei_return_if_fail (!az_type_is_a (type, AZ_TYPE_INTERFACE));
#endif
	klass = az_type_get_class (type);
	for (i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		az_instance_finalize_recursive (klass, &klass->impl, instance);
	}
	if (klass->allocator && klass->allocator->free_array) {
		klass->allocator->free_array (klass, elements, nelements);
	} else {
		free (elements);
	}
}

unsigned int
az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	return (klass->serialize) ? klass->serialize (impl, inst, d, dlen, ctx) : 0;
}

/* fixme: Make signature correct */

unsigned int
az_instance_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int dlen)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
	return klass->to_string (&klass->impl, inst, d, dlen);
}

const AZImplementation *
az_instance_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (impl == AZ_BOXED_INTERFACE_IMPL) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	if (az_type_is_a (AZ_IMPL_TYPE(impl), if_type)) {
		if (if_inst) *if_inst = inst;
		return impl;
	}
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	const AZIFEntry *ifentry = az_class_iface_all(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_all; i++) {
		if (az_type_is_a(ifentry->type, if_type)) {
			AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
			if (if_inst) *if_inst = (char *) inst + ifentry->inst_offset;
			return sub_impl;
		}
		ifentry += 1;
	}
	if (if_inst) *if_inst = NULL;
	return NULL;
}
