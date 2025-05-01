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

#define AZ_TYPE_VALUE_SIZE(t) az_type_get_class(t)->value_size

#ifdef ARIKKEI_MEMCHECK
#define ARIKKEI_CHECK_INTEGRITY() arikkei_check_integrity()
void arikkei_check_integrity (void);
#else
#define ARIKKEI_CHECK_INTEGRITY()
#endif

#ifndef __AZ_TYPES_C__
/* Interfaces declared in specific class */
extern AZIFEntry *az_ifaces_self;
extern unsigned int az_ifaces_self_len;
extern unsigned int az_ifaces_self_size;
/* All interfaces for specific class starting from base class, first interface then sub-interfaces */
extern AZIFEntry *az_ifaces_all;
extern unsigned int az_ifaces_all_len;
extern unsigned int az_ifaces_all_size;
#else
AZIFEntry *az_ifaces_self = NULL;
unsigned int az_ifaces_self_len = 0;
unsigned int az_ifaces_self_size = 0;

AZIFEntry *az_ifaces_all;
unsigned int az_ifaces_all_len;
unsigned int az_ifaces_all_size;
#endif

ARIKKEI_INLINE unsigned int
az_type_is_valid(uint32_t type) {
	if (AZ_TYPE_INDEX(type) == 0) return 0;
	if (AZ_TYPE_INDEX(type) >= az_num_classes) return 0;
	AZImplementation *impl = AZ_IMPL_FROM_TYPE(type);
	return type == impl->type;
}

/* Library internals */
void az_classes_init (void);

void az_init_implementation_class (void);
void az_implementation_class_post_init (void);
void az_class_class_init (void);
void az_class_class_post_init (void);

/* Allocates and initializes a new class and type, does NOT call neither class constructor nor post_init */
AZClass *az_class_new (uint32_t *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));
/* Allocates and initializes a new class with pre-defined type, does NOT call neither class constructor nor post_init */
/* Used internally for primitive types */
AZClass *az_class_new_with_type (unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name);

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

