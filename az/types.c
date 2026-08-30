#define __AZ_TYPES_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

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
az_type_is_a (unsigned int type, unsigned int to_type)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(to_type), 0);
#endif
	if (!type) return 0;
	if (type == to_type) return 1;

	klass = AZ_CLASS_FROM_TYPE(type);
	while (klass->parent) {
		if (klass->parent->impl.type == to_type) return 1;
		klass = klass->parent;
	}
	return 0;
}

unsigned int
az_type_implements (unsigned int type, unsigned int to_type)
{
	if (!type) return 0;
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(to_type), 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE(to_type), 0);
#endif
	if (!type) return 0;
	return az_instance_get_interface (&AZ_CLASS_FROM_TYPE(type)->impl, NULL, to_type, NULL) != NULL;
}

unsigned int
az_type_is_assignable_to (unsigned int type, unsigned int to_type)
{
#ifdef AZ_SAFETY_CHECKS
	ENSURE_INITIALIZED();
	arikkei_return_val_if_fail (!type || az_type_is_valid(type), 0);
	arikkei_return_val_if_fail (az_type_is_valid(to_type), 0);
#endif
	if (!type) {
		/* None can be assigned to any */
		if (to_type == AZ_TYPE_ANY) return 1;
		/* None can be assigned to blocks */
		if (AZ_TYPE_IS_BLOCK(to_type)) return 1;
	}
	if (az_type_is_a (type, to_type)) return 1;
	if (AZ_TYPE_IS_INTERFACE(to_type)) {
		return az_type_implements (type, to_type);
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
#endif
	/* Top-level registrations construct eagerly, nested ones are deferred until first class access */
	return az_type_register_internal (type, name, parent_type, class_size, instance_size, flags,
		n_interfaces_self, n_properties_self,
		class_init, NULL, NULL,
		instance_init, instance_finalize,
		0, NULL, 0);
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
#endif
	/* Top-level registrations construct eagerly, nested ones are deferred until first class access */
	return az_type_register_internal (type, name, parent_type, class_size, instance_size, flags,
		n_interfaces_self, n_properties_self,
		NULL, class_init, data,
		instance_init, instance_finalize,
		0, NULL, 0);
}

