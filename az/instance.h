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

/**
 * @brief Get property from instance
 * 
 * 
 * 
 * @param impl the type implementation
 * @param inst the type instance
 * @param key property key
 * @param dst_impl the result implementation
 * @param dst_val the result value
 * @return 1 if successfule, 0 if not
 */
unsigned int az_instance_get_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val);
unsigned int az_instance_get_function_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val);
unsigned int az_instance_set_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);

#ifdef AZ_HAS_PROPERTIES
/**
 * @brief Get property value by defining class and property index
 * 
 * This is the most generic method that can retrieve any kind of property given the defining class
 * and the class, implementation and instance of type. For static properties the instance and/or implementation
 * can be NULL.
 * The normal order of execution is to first call az_class_lookup property and, if the property is found, use
 * def_class, AZ_CLASS_FROM_IMPL(impl), impl, inst and index or, for pure statics, def_class, def_class and index
 * to fetch the property value.
 * 
 * @param def_klass the class where property is defined
 * @param klass the actuqal class of the instance
 * @param impl the actual implementation of the instance (or NULL)
 * @param inst the actual instance (or NULL)
 * @param id property index in class
 * @param prop_impl result implementation
 * @param prop_val result value
 * @param val_size result value size (value is autoboxed if size is bigger)
 * @param ctx an execution context
 * @return 1 if successful, 0 if the property is not readable
 */
unsigned int az_instance_get_property_by_id (const AZClass *def_klass, const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation **prop_impl, AZValue64 *prop_val, unsigned int val_size, AZContext *ctx);
unsigned int az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
#endif

#ifdef __cplusplus
}
#endif

#endif
