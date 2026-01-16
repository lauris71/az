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
#include <az/interface.h>
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

void
az_init (void)
{
	static unsigned int initialized = 0;
	if (initialized) return;
	initialized = 1;
	az_num_types = AZ_NUM_BASE_TYPES;
	az_globals_init();
	az_init_primitive_classes();
	az_init_base_classes();

	az_init_interface_class();
#ifdef AZ_HAS_PROPERTIES
	az_init_field_class();
#endif
	az_init_function_classes();
	az_init_reference_class();
	az_init_string_class();
	az_init_boxed_value_class();
	az_init_boxed_interface_class();
#ifdef AZ_HAS_PACKED_VALUE
	az_init_packed_value_class();
#endif
	az_init_object_class();

	az_post_init_primitive_classes();
}

unsigned int
az_type_get_parent_primitive (unsigned int type)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
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
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
	arikkei_return_val_if_fail (test < az_num_types, 0);
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
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, 0);
	arikkei_return_val_if_fail (test < az_num_types, 0);
	arikkei_return_val_if_fail (AZ_TYPE_IS_INTERFACE(test), 0);
#endif
	if (!type) return 0;
	return az_instance_get_interface (&AZ_CLASS_FROM_TYPE(type)->impl, NULL, test, NULL) != NULL;
}

unsigned int
az_type_is_assignable_to (unsigned int type, unsigned int test)
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) != 0, 0);
	arikkei_return_val_if_fail (AZ_TYPE_INDEX(test) < az_num_types, 0);
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
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_num_types) az_init ();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
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
	if (!az_num_types) az_init ();
	assert (!parent_type || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size));
	assert (!parent_type || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size));
#endif
	AZClass *klass = az_class_new (type, name, parent_type, class_size, instance_size, flags, instance_init, instance_finalize);
	arikkei_return_val_if_fail (*type, NULL);
	if (class_init) class_init (klass, data);
	az_class_post_init (klass);
	return klass;
}

