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
/* Boxed values/interfaces: operates on the box itself - lifecycle is never forwarded to the content */
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
/* Boxed values/interfaces: operates on the box itself - lifecycle is never forwarded to the content */
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
	if (AZ_TYPE_HAS_CONSTRUCT(type)) az_instance_init(AZ_IMPL_FROM_TYPE(type), inst);
}
static inline void
az_instance_finalize_by_type (void *inst, unsigned int type)
{
	if (AZ_TYPE_HAS_CONSTRUCT(type)) az_instance_finalize(AZ_IMPL_FROM_TYPE(type), inst);
}

/**
 * @brief Allocate and initialize a new instance
 * 
 * Class must be:
 * - Not abstract (AZ_FLAG_ABSTRACT not set)
 * - Fixed size instance (AZ_FLAG_VARIABLE_SIZE not set)
 * - Instance size > 0
 * If class has an allocator, it's allocate method is used, otherwise the generic malloc
 * 
 * @param type the typecode
 * @return the new instance
 */
void *az_instance_new (unsigned int type);
/**
 * @brief Allocate and initialize a variable-size instance
 *
 * Only valid for classes with AZ_FLAG_VARIABLE_SIZE; size must be at least the
 * class's (minimum) instance_size. The class allocator, if any, decides
 * between pooled and generic allocation based on the size.
 *
 * @param type the typecode
 * @param size the allocation size in bytes
 * @return the new instance
 */
void *az_instance_new_sized (unsigned int type, unsigned int size);
void *az_instance_new_array (unsigned int type, unsigned int nelements);
void az_instance_delete (unsigned int type, void *inst);
/**
 * @brief Release the instance memory without finalizing it
 *
 * The free half of az_instance_delete (which is az_instance_finalize +
 * az_instance_free). Used by the reference destruction tail, which finalizes
 * unlocked but releases memory under the lifecycle write lock.
 */
void az_instance_free (unsigned int type, void *inst);
/**
 * @brief Class-based variant of az_instance_free
 *
 * Avoids the registry lookup when the class is already known (e.g. the
 * reference destruction tail, which calls this under the lifecycle write lock).
 */
void az_instance_free_with_class (AZClass *klass, void *inst);
void az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements);
/**
 * @brief Check if an instance is of a given type
 * 
 * For boxed values and boxed interfaces the check is performed on the underlying type.
 * 
 * @param impl The type implementation
 * @param inst The instance to check
 * @param type The typecode to compare against
 * @return unsigned int 
 */
unsigned int az_instance_is_a(const AZImplementation *impl, void *inst, unsigned int type);
/**
 * @brief Check if an instance implements a given interface
 * 
 * @param impl The type implementation
 * @param inst The instance to check
 * @param type The interface type to check for
 * @return unsigned int 
 */
unsigned int az_instance_implements(const AZImplementation *impl, void *inst, unsigned int type);

int64_t az_instance_serialize (const AZImplementation *impl, void *inst, uint8_t *d, uint64_t dlen);
/**
 * @brief writes a string representation of instance to the buffer
 * 
 * Returns the number of content bytes (excluding the terminating 0) needed for the full representation.
 * If dlen is less than required, the string is truncated.
 * Unless len is 0 or d is NULL, the terminating 0 is always written
 * 
 * @param impl type implementation
 * @param inst type instance
 * @param d destination buffer (may be NULL, nothing is written then)
 * @param dlen destination buffer size
 * @return the required length of the string (excluding the terminating '\0', may be bigger than dlen)
 */
int64_t az_instance_to_string (const AZImplementation *impl, void *inst, uint8_t *d, uint64_t dlen);

/**
 * @brief get a newly allocated string representation of instance
 *
 * Convenience frontend to az_instance_to_string. To speed things up for the
 * normal case the instance is first rendered into a local buffer and copied,
 * only if the representation does not fit there an exact size buffer is
 * allocated and the instance is rendered again.
 *
 * @param impl type implementation
 * @param inst type instance
 * @return the newly allocated terminated string (free with free())
 */
uint8_t *az_instance_to_string_new (const AZImplementation *impl, void *inst);

/**
 * @brief Get the first interface of given type as naked interface
 * 
 * The search order is:
 * - the instance itself (if it is an interface of right type)
 * - all interfaces implemented by given class (self-interfaces)
 * - all sub-interfaces implemented by self-interfaces (recursively)
 * - all interfaces (first self-interfaces, then sub-interfaces) implemented by parent
 * As the same interface type may be implemented at different places, only the first will be returned.
 * Boxed interfaces and boxed values are transparently delegated.
 * 
 * @param impl The type implementation
 * @param inst The type instance
 * @param if_type The interface type
 * @param if_inst The output value for interface instance
 * @return The implementation of an interface
 */
const AZImplementation *az_instance_get_interface(const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst);
/**
 * @brief Get the first interface of given type as a boxed interface handle value, if boxing is possible, otherwise as naked interface
 *
 * The destination value receives the interface handle: a boxed interface
 * (AZ_BOXED_INTERFACE_IMPL + owned reference) when a box can own the container
 * lifecycle, or the naked view (interface implementation + borrowed instance).
 * The result may hold a reference and has to be cleared by the caller.
 *
 * A sub or same interface of a naked interface remains naked
 * The same interface of the boxed interface returns a reference to the same box
 * A subinterface of a boxed interface returns a new box
 * An interface of a value or reference container creates a new box
 * An interface of a plain block container remains naked (caller-managed lifetime)
 *
 * @param impl The type implementation
 * @param inst The type instance
 * @param if_type The interface type
 * @param dst The destination value for the interface handle (has to be cleared by the caller)
 * @return The implementation stored in dst (AZ_BOXED_INTERFACE_IMPL if boxed), or NULL if there is no such interface
 */
const AZImplementation *az_instance_get_interface_autobox(const AZImplementation *impl, void *inst, unsigned int if_type, AZValue *dst);

static inline const AZImplementation *
az_instance_get_interface_from_type (unsigned int type, void *inst, unsigned int if_type, void **if_inst)
{
	return az_instance_get_interface(AZ_IMPL_FROM_TYPE(type), inst, if_type, if_inst);
}


#ifdef __cplusplus
}
#endif

#endif
