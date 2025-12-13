#ifndef __AZ_PRIVATE_H__
#define __AZ_PRIVATE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

/* Turn on internal/optimized/unchecked variants */
#define AZ_INTERNAL

#include <az/types.h>
#include <az/class.h>

#define AZ_TYPE_VALUE_SIZE(t) az_class_value_size(az_type_get_class(t))

#ifdef ARIKKEI_MEMCHECK
#define ARIKKEI_CHECK_INTEGRITY() arikkei_check_integrity()
void arikkei_check_integrity (void);
#else
#define ARIKKEI_CHECK_INTEGRITY()
#endif

ARIKKEI_INLINE unsigned int
az_type_is_valid(uint32_t type) {
	if (AZ_TYPE_INDEX(type) == 0) return 0;
	if (AZ_TYPE_INDEX(type) >= az_num_types) return 0;
	AZImplementation *impl = AZ_IMPL_FROM_TYPE(type);
	return type == impl->type;
}

/* Library internals */
void az_globals_init (void);

unsigned int az_reserve_type();

/* Library internals */
void az_init_primitive_classes (void);
void az_post_init_primitive_classes (void);

void az_init_base_classes (void);
void az_post_init_base_classes (void);
void az_impl_class_post_init (void);
void az_class_class_post_init (void);

void az_init_interface_class (void);
void az_init_field_class (void);
void az_init_function_classes (void);
void az_init_reference_class (void);
void az_init_string_class (void);
void az_init_boxed_value_class (void);
void az_init_boxed_interface_class (void);
void az_init_packed_value_class (void);
void az_init_object_class(void);

/* Allocates and initializes a new class and type, does NOT call neither class constructor nor post_init */
AZClass *az_class_new (uint32_t *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));
/* Allocates and initializes a new class with pre-defined type, does NOT call neither class constructor nor post_init */
/* Used internally for fundamental types */
AZClass *az_class_new_with_type (unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name);
void az_class_new_with_value (AZClass *klass);

/* Called after class constructor has run (builds interface chain etc.) */
void az_class_post_init (AZClass *klass);

/* Constrained type */
typedef struct _AZTypeConstraint AZTypeConstraint;

struct _AZTypeConstraint {
	uint32_t is_a;
	uint32_t implements_a;
};

#ifdef __cplusplus
}
#endif

#endif /* PRIVATE_H */

