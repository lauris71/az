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

#include <az/base.h>
#include <az/class.h>
#include <az/extend.h>
#include <az/interface.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/boxed-interface.h>
#include <az/packed-value.h>
#include <az/field.h>

#include <az/types.h>

unsigned int
az_type_get_parent_primitive (unsigned int type)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (az_type_is_valid(type), 0);
#endif
	if (AZ_TYPE_INDEX(type) < AZ_NUM_FUNDAMENTAL_TYPES) return type;
	klass = AZ_CLASS_FROM_TYPE(type)->parent;
	while (AZ_TYPE_INDEX(AZ_CLASS_TYPE(klass)) >= AZ_NUM_FUNDAMENTAL_TYPES) {
		klass = klass->parent;
	}
	return AZ_CLASS_TYPE(klass);
}

unsigned int
az_type_is_a (unsigned int type, unsigned int test)
{
	AZClass *klass;
	unsigned int i;
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(test), 0);
#endif
	if (!type) return 0;
	if (type == test) return 1;

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_SINGLE_THREAD)
	test = AZ_TYPE_INDEX(test);
	uint32_t idx = az_types[AZ_TYPE_INDEX(type)].pidx;
	while (idx) {
		if (idx == test) return 1;
		idx = az_types[idx].pidx;
	}
#else
	klass = AZ_CLASS_FROM_TYPE(type);
	while (klass->parent) {
		if (klass->parent->impl.type == test) return 1;
		klass = klass->parent;
	}
#endif
	return 0;
}

unsigned int
az_type_implements (unsigned int type, unsigned int test)
{
	if (!type) return 0;
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(test), 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE(test), 0);
#endif
	if (!type) return 0;
	return az_instance_get_interface (&AZ_CLASS_FROM_TYPE(type)->impl, NULL, test, NULL) != NULL;
}

unsigned int
az_type_is_assignable_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (!type || az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(test), 0);
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

AZClass *
az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
#endif
	if ((flags & AZ_FLAG_ZERO_MEMORY) || n_interfaces_self || instance_init || instance_finalize) {
		flags |= AZ_FLAG_CONSTRUCT;
	}
	AZClass *klass = az_class_new (name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	/* Type has to be registered before class_init so it is accessible in class constructor (ifaces, properties) */
	*type = klass->impl.type;
	if (n_interfaces_self) az_class_set_num_interfaces (klass, n_interfaces_self);
	if (n_properties_self) az_class_set_num_properties (klass, n_properties_self);
	if (class_init) class_init (klass);
	az_class_post_init (klass);
	return klass;
}

AZClass *
az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
#endif
	if ((flags & AZ_FLAG_ZERO_MEMORY) || n_interfaces_self || instance_init || instance_finalize) {
		flags |= AZ_FLAG_CONSTRUCT;
	}
	AZClass *klass = az_class_new (name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	if (n_interfaces_self) az_class_set_num_interfaces (klass, n_interfaces_self);
	if (n_properties_self) az_class_set_num_properties (klass, n_properties_self);
	if (class_init) class_init (klass, data);
	az_class_post_init (klass);
	*type = klass->impl.type;
	return klass;
}

