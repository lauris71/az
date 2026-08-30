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
#include <stdatomic.h>

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

/*
 * Basic type queries
 */

#if defined(AZ_GLOBALS_STATIC)
	/* Fixed-length static array, thread-safe with lock-free fast path */
	extern _Atomic(uintptr_t) az_types[];
	extern _Atomic unsigned int az_num_types;
	/* Slow path: locked lookup, constructs the class on demand */
	AZClass *az_type_get_class (unsigned int type);
	/*
	 * Lock-free fast path
	 *
	 * A slot holds either a class pointer or a tagged (LSB set) construction descriptor
	 * for a type whose class is reserved but not yet constructed. Class pointers are
	 * published with release semantics only after full construction (az_class_publish),
	 * so an acquire load yielding an untagged pointer is guaranteed to see the complete
	 * class. Anything else falls back to the locked slow path, which constructs the
	 * class on demand.
	 */
	static inline AZClass *
	az_class_from_type (unsigned int type)
	{
		uintptr_t p = atomic_load_explicit (&az_types[AZ_TYPE_INDEX(type)], memory_order_acquire);
		if (p && !(p & 1)) return (AZClass *) p;
		return az_type_get_class (type);
	}
	#define AZ_CLASS_FROM_TYPE(t) az_class_from_type(t)
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_class_from_type(t))
#elif defined(AZ_GLOBALS_SINGLE_THREAD)
	/* Dynamically allocated array*/
	extern uintptr_t *az_types;
	extern unsigned int az_num_types;
	/* Slow path: lookup, constructs the class on demand */
	AZClass *az_type_get_class (unsigned int type);
	static inline AZClass *
	az_class_from_type (unsigned int type)
	{
		uintptr_t p = az_types[AZ_TYPE_INDEX(type)];
		if (p && !(p & 1)) return (AZClass *) p;
		return az_type_get_class (type);
	}
	#define AZ_CLASS_FROM_TYPE(t) az_class_from_type(t)
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_class_from_type(t))
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	AZClass *az_type_get_class (unsigned int type);
	#define AZ_CLASS_FROM_TYPE(t) az_type_get_class(t)
	#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_type_get_class(t))
#endif

#define AZ_TYPE_FROM_INDEX(i) (AZ_IMPL_FROM_TYPE(i)->type)

/* Flag-based type checks: equivalent to az_type_is_a (t, AZ_TYPE_...) against the flag-defining */
/* root type (flags are inherited by subtypes), but resolved directly from the typecode */
#define AZ_TYPE_IS_BLOCK(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_VALUE(t) !(AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_INTERFACE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_INTERFACE)
#define AZ_TYPE_IS_REFERENCE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_REFERENCE)
/* Both boxed types are final, so the typecode index identifies them exactly */
#define AZ_TYPE_IS_BOXED(t) ((AZ_TYPE_INDEX(t) == AZ_TYPE_IDX_BOXED_VALUE) || (AZ_TYPE_INDEX(t) == AZ_TYPE_IDX_BOXED_INTERFACE))
#define AZ_TYPE_IS_OBJECT(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_OBJECT)
#define AZ_TYPE_IS_FINAL(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_FINAL)
#define AZ_TYPE_IS_ABSTRACT(t) (AZ_CLASS_FLAGS(AZ_CLASS_FROM_TYPE(t)) & AZ_FLAG_ABSTRACT)

#if defined(AZ_GLOBALS_SINGLE_THREAD)
	#define AZ_TYPES_LOCK()
	#define AZ_TYPES_UNLOCK()
	#define AZ_TYPE_READ(t) (t)
#elif defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_MULTI_THREAD)
	/**
	 * @brief Lock the type system mutex
	 *
	 * The mutex is recursive, so it can be taken recursively from class/implementation
	 * constructors and nested get_type() calls.
	 *
	 * Registration (az_register_type/az_register_composite_type) holds the lock through
	 * the entire reserve -> construct -> publish sequence. The typecode is reserved
	 * first and stored to the caller's variable (with release semantics) before any
	 * construction, so class constructors may reference not-yet-constructed types by
	 * typecode (e.g. methods of A and B taking arguments of each other's type).
	 *
	 * Top-level registrations construct the class eagerly; registrations nested inside
	 * another class construction only reserve the typecode and defer construction to
	 * the first class access (az_type_get_class). The slot holds a tagged construction
	 * descriptor until the class is published with release semantics: an untagged slot
	 * pointer always denotes a fully constructed class.
	 *
	 * Lazy type registration (get_type methods) uses double-checked locking: a fast-path
	 * acquire-read of the static type variable, followed by the locked check-and-register
	 * sequence:
	 *
	 * unsigned int t = AZ_TYPE_READ(type);
	 * if (t) return t;
	 * AZ_TYPES_LOCK();
	 * if (!type) {
	 *     az_register_type (&type, ...);
	 * }
	 * t = type;
	 * AZ_TYPES_UNLOCK();
	 * return t;
	 *
	 * Under AZ_GLOBALS_STATIC reading the class itself is also lock-free: the acquire
	 * load in the AZ_CLASS_FROM_TYPE fast path synchronizes with the release store of
	 * the slot. Under AZ_GLOBALS_MULTI_THREAD all class access goes through the locked
	 * az_type_get_class.
	 *
	 * For types with dynamically-grown subtype arrays (e.g. az_reference_of_get_type),
	 * the fast-path is not safe because the array itself may be reallocated; these must
	 * use the always-lock pattern (AZ_TYPES_LOCK before any read).
	 */
	void az_types_lock();
	void az_types_unlock();
	#define AZ_TYPES_LOCK() az_types_lock()
	#define AZ_TYPES_UNLOCK() az_types_unlock()
	#define AZ_TYPE_READ(t) atomic_load_explicit((_Atomic unsigned int *)&(t), memory_order_acquire)
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
 * @param to_type the type tested against
 * @return 1 if type is a subtype of to_type, 0 if not or if either type is invalid
 */
unsigned int az_type_is_a (unsigned int type, unsigned int to_type);

/** @ingroup types
 * @brief Checks whether the given type implements an interface type
 * 
 * @param type the type that is checked
 * @param to_type the interface type tested against
 * @return 1 if type implements to_type, 0 if not or if either type is invalid
 */
unsigned int az_type_implements (unsigned int type, unsigned int to_type);
/** @ingroup types
 * @brief Checks whether a value of certain type can be assigned to a variable of given type
 * 
 * True if:
 * - type is to_type
 * - type implements to_type
 * - type is NONE and to_type is ANY or BLOCK
 * 
 * @param type the type that is checked
 * @param to_type the type of variable to be tested against
 * @return 1 if type can be assigned, 0 if not or if either type is invalid
 */
unsigned int az_type_is_assignable_to (unsigned int type, unsigned int to_type);

#ifdef __cplusplus
};
#endif

#endif
