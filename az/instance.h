#ifndef __AZ_INSTANCE_H__
#define __AZ_INSTANCE_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <az/az.h>
#include <az/class.h>
#include <az/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initialize a new instance
 * 
 * Initializes a new type instance. The order of operations is:
 * If AZ_FLAG_ZERO_MEMORY is set clear instance memoery.
 * If AZ_FLAG CONSTRUCT is set, call:
 * 1. the superclass constructor
 * 2. constructors of all interfaces
 * 3. instance_init virtual method
 * 
 * @param impl type implementation
 * @param inst uninitialized instance
 */
void az_instance_init (const AZImplementation *impl, void *inst);
/**
 * @brief destroy an instance
 * 
 * Destroys an type instance. The order of operations is:
 * If AZ_FLAG CONSTRUCT is set, call:
 * 1. instance_finalize virtual method
 * 2. destructos of all interfaces
 * 2. the superclass destructor
 * 
 * @param impl type implementation
 * @param inst type instance
 */
void az_instance_finalize (const AZImplementation *impl, void *inst);

/**
 * @brief Initialize a new instance
 * 
 * Convenience frontent to az_instance_init
 * @param inst pointer to instance
 * @param type instance type
 */
static inline void
az_instance_init_by_type (void *inst, unsigned int type)
{
	if (AZ_TYPE_FLAGS(type) & (AZ_FLAG_ZERO_MEMORY | AZ_FLAG_CONSTRUCT)) az_instance_init(AZ_IMPL_FROM_TYPE(type), inst);
}
static inline void
az_instance_finalize_by_type (void *inst, unsigned int type)
{
	if (AZ_TYPE_FLAGS(type) & AZ_FLAG_CONSTRUCT) az_instance_finalize(AZ_IMPL_FROM_TYPE(type), inst);
}

void *az_instance_new (unsigned int type);
void *az_instance_new_array (unsigned int type, unsigned int nelements);
void az_instance_delete (unsigned int type, void *inst);
void az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements);

unsigned int az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
/**
 * @brief writes a string representation of instance to the buffer
 * 
 * Returns the number of content bytes (excluding the terminating 0)
 * Unless len == 0, terminating 0 is always written
 * 
 * @param impl type implementation
 * @param inst type instance
 * @param d destination buffer
 * @param dlen destination buffer size
 * @return the content length (can be > dlen, not counting terminating 0)
 */
unsigned int az_instance_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen);

/* Get rootmost interface */
const AZImplementation *az_instance_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst);
static inline const AZImplementation *
az_instance_get_interface_from_type (unsigned int type, void *inst, unsigned int if_type, void **if_inst)
{
	return az_instance_get_interface (AZ_IMPL_FROM_TYPE(type), inst, if_type, if_inst);
}

#ifdef __cplusplus
}
#endif

#endif
