#ifndef __AZ_TYPES_H__
#define __AZ_TYPES_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdint.h>
#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/az.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup types AZ types
 *  The basic type access
 * 
 * An AZ type ia a 32-bit unsigned integer, composed of two parts:
 * - 24-bit type index (bits 23-0), used to access type info array
 * - 8-bit flags (bits 31-24), used to store type flags
 * 
 * Types are used throughout the api as a more compact type identifier than class pointer.
 * 
 * Semantically types are synonymous to a class, NOT an implementation - the interfaces have to
 * be accessed through an Implementation/Instance pointer pair because typecode only gives access
 * to the class of the interface, not to it's actual implementation.
 */

 /**
  * @brief The mask of index bits of a typecode
  * 
  */
#define AZ_TYPE_MASK 0x00ffffff
/**
 * @brief Get a type index from a typecode
 * 
 */
#define AZ_TYPE_INDEX(t) ((t) & AZ_TYPE_MASK)
/**
 * @brief Get a type flags from a typecode
 * 
 */
#define AZ_TYPE_FLAGS(t) ((t) & ~AZ_TYPE_MASK)

typedef struct _AZTypeInfo AZTypeInfo;

/**
 * @brief An internal array element of type info
 * 
 * It is used to get a class from typecode (type index) and to determine parent types
 * without having to traverse the class hierarchy.
 * 
 */
struct _AZTypeInfo {
	AZClass *klass;
	/* Parent INDEX */
	uint32_t pidx;
};

/*
 * Basic type queries
 */

#if defined(AZ_GLOBALS_STATIC)
	/* Fixed-length static array */
	extern AZTypeInfo az_types[];
	extern unsigned int az_num_types;
	#define AZ_CLASS_FROM_TYPE(t) az_types[AZ_TYPE_INDEX(t)].klass
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_types[AZ_TYPE_INDEX(t)].klass)
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	/* Dynamically allocated array*/
	extern AZTypeInfo *az_types;
	extern unsigned int az_num_types;
	#define AZ_CLASS_FROM_TYPE(t) az_types[AZ_TYPE_INDEX(t)].klass
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_types[AZ_TYPE_INDEX(t)].klass)
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	#define AZ_CLASS_FROM_TYPE(t) az_type_get_class(t)
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_type_get_class(t))
#endif

#define AZ_TYPE_FROM_INDEX(i) (AZ_IMPL_FROM_TYPE(i)->type)

#define AZ_TYPE_IS_BLOCK(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_VALUE(t) !(AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_INTERFACE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_INTERFACE)
#define AZ_TYPE_IS_REFERENCE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_REFERENCE)
#define AZ_TYPE_IS_BOXED(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_BOXED)
#define AZ_TYPE_IS_OBJECT(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_OBJECT)
#define AZ_TYPE_IS_FINAL(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_FINAL)
#define AZ_TYPE_IS_ABSTRACT(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_ABSTRACT)

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_SINGLE_THREAD)
	#ifdef AZ_SAFETY_CHECKS
		static inline AZClass *
		az_type_get_class (unsigned int type)
		{
			if (!az_num_types) az_init ();
			arikkei_return_val_if_fail (AZ_TYPE_INDEX(type) < az_num_types, NULL);
			return AZ_CLASS_FROM_TYPE(type);
		}
	#else
		#define az_type_get_class AZ_CLASS_FROM_TYPE
	#endif
	#define AZ_TYPES_LOCK()
	#define AZ_TYPES_UNLOCK()
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	AZClass *az_type_get_class (unsigned int type);
	/**
	 * @brief Lock the type system mutex
	 * 
	 * The mutex is recursive, so subclasses can use it in get_type() etc. methods.
	 */
	void az_types_lock();
	void az_types_unlock();
	#define AZ_TYPES_LOCK() az_types_lock()
	#define AZ_TYPES_UNLOCK() az_types_unlock()
#endif

#define az_type_get_impl(t) ((AZImplementation *) az_type_get_class(t))

/** @ingroup types
 * @brief Get parent primitive type
 * 
 * @param type the query type
 * @return the parent primitive (AZ_TYPE_BOOLEAN - AZ_TYPE_BLOCK) or AZ_TYPE_NONE for invalid type 
 */
unsigned int az_type_get_parent_primitive (unsigned int type);

/** @ingroup types
 * @brief Checks whether the given type is a subtype of another
 * 
 * @param type the type that is checked
 * @param test the type tested against
 * @return 1 if type is a subtype of test, 0 if not or if either type is invalid
 */
unsigned int az_type_is_a (unsigned int type, unsigned int test);

/** @ingroup types
 * @brief Checks whether the given type implements an interface type
 * 
 * @param type the type that is checked
 * @param test the interface type tested against
 * @return 1 if type implements test, 0 if not or if either type is invalid
 */
unsigned int az_type_implements (unsigned int type, unsigned int test);
/** @ingroup types
 * @brief Checks whether a value of certain type can be assigned to a variable of given type
 * 
 * True if:
 * - type is test
 * - type implements test
 * - type is NONE and test is ANY or BLOCK
 * 
 * @param type the type that is checked
 * @param test the type of variable to be tested against
 * @return 1 if type can be assigned, 0 if not or if either type is invalid
 */
unsigned int az_type_is_assignable_to (unsigned int type, unsigned int test);

#ifdef __cplusplus
};
#endif

#endif
