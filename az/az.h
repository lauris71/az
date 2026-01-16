#ifndef __AZ_H__
#define __AZ_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <az/config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Predefined typecodes */

enum AZType {
	/**
	 * @brief Invalid or missing type, typecode 0
	 * 
	 */
	AZ_TYPE_NONE,
	/**
	 * @brief The abstract base class of all types
	 * 
	 */
	AZ_TYPE_ANY,

	/* Primitives */
	AZ_TYPE_BOOLEAN,
	AZ_TYPE_INT8,
	AZ_TYPE_UINT8,
	AZ_TYPE_INT16,
	AZ_TYPE_UINT16,
	AZ_TYPE_INT32,
	AZ_TYPE_UINT32,
	AZ_TYPE_INT64,
	AZ_TYPE_UINT64,
	AZ_TYPE_FLOAT,
	AZ_TYPE_DOUBLE,
	AZ_TYPE_COMPLEX_FLOAT,
	AZ_TYPE_COMPLEX_DOUBLE,
	AZ_TYPE_POINTER,

	/* Fundamental types */
	/**
	 * @brief The abstract base class of all composite value types
	 * 
	 */
	AZ_TYPE_STRUCT,
	/**
	 * @brief The abstract base class of all block types
	 * 
	 */
	AZ_TYPE_BLOCK,
	/* Fundamental types have ANY as parent */
	AZ_NUM_FUNDAMENTAL_TYPES = AZ_TYPE_BLOCK,

	/* Special types */
	/**
	 * @brief The abstract base class of the semantics of a type
	 * 
	 * An instance of _AZImplementation. Specialized for interface types, _AZClass for standalone types
	 * 
	 */
	AZ_TYPE_IMPLEMENTATION,
	/**
	 * @brief The semantics of a type
	 * 
	 * An instance of _AZClass, a subclass of _AZImplementation
	 * 
	 */
	AZ_TYPE_CLASS,
	/**
	 * @brief An abstract base class of instances of interface types
	 * 
	 */
	AZ_TYPE_INTERFACE,
#ifdef AZ_HAS_PROPERTIES
	AZ_TYPE_FIELD,
#endif
	AZ_TYPE_FUNCTION_SIGNATURE,
	AZ_TYPE_FUNCTION,
	/* Predefined composite types */
	AZ_TYPE_REFERENCE,
	AZ_TYPE_STRING,
	AZ_TYPE_BOXED_VALUE,
	AZ_TYPE_BOXED_INTERFACE,
#ifdef AZ_HAS_PACKED_VALUE
	AZ_TYPE_PACKED_VALUE,
#endif
	AZ_TYPE_OBJECT,
	/* Count */
	AZ_NUM_BASE_TYPES
};

#define AZ_TYPE_IS_ARITHMETIC(t) (((t) >= AZ_TYPE_INT8) && ((t) <= AZ_TYPE_COMPLEX_DOUBLE))
#define AZ_TYPE_IS_INTEGRAL(t) (((t) >= AZ_TYPE_INT8) && ((t) <= AZ_TYPE_UINT64))
#define AZ_TYPE_IS_SIGNED(t) (((t) == AZ_TYPE_INT8) || ((t) == AZ_TYPE_INT16) || ((t) == AZ_TYPE_INT32) || ((t) == AZ_TYPE_INT64) || ((t) == AZ_TYPE_FLOAT) || ((t) == AZ_TYPE_DOUBLE) || ((t) == AZ_TYPE_COMPLEX_FLOAT) || ((t) == AZ_TYPE_COMPLEX_DOUBLE))
#define AZ_TYPE_IS_UNSIGNED(t) (((t) == AZ_TYPE_UINT8) || ((t) == AZ_TYPE_UINT16) || ((t) == AZ_TYPE_UINT32) || ((t) == AZ_TYPE_UINT64))
#define AZ_TYPE_IS_64(t) (((t) == AZ_TYPE_INT64) || ((t) == AZ_TYPE_UINT64))
#define AZ_TYPE_IS_PRIMITIVE(t) (((t) >= AZ_TYPE_BOOLEAN) && ((t) <= AZ_TYPE_POINTER))
#define AZ_TYPE_IS_BASE(t) (((t) >= AZ_TYPE_ANY) && ((t) <= AZ_TYPE_OBJECT))

/**
 * @brief Class/implementation/type flags
 * 
 * These are various low- and high-level flags that describe the type behaviour. Some of these
 * (like `AZ_FLAG_IMPL_IS_CLASS` and `AZ_FLAG_FINAL`) define the type behavior. Some others
 * (like `AZ_FLAG_REFERENCE` and `AZ_FLAG_OBJECT`) duplicate the type hierarchy information for convenience.
 */
enum AZTypeFlags {
	/**
	 * @brief Marks that an implementation is a standalone class
	 * 
	 * It exploits the feature that classes are aligned to 8 bytes, thus if any of the
	 * lowest bits are set, the AZImplementation union contains flags and type, not a
	 * pointer to the AZClass
	 */
	AZ_FLAG_IMPL_IS_CLASS = 0x01,

	/*
	* High-order flags, can be put in typecode
	*/
	/**
	 * @brief Type is an interface, implementation is not class
	 * 
	 */
	AZ_FLAG_INTERFACE = 0x01000000,
	/**
	 * @brief Type is a block, value is a pointer to the instance (not the instance itself)
	 * 
	 */
	AZ_FLAG_BLOCK = 0x02000000,
	/**
	 * @brief Type is a reference, value creation/destruction involves reference counting
	 * 
	 */
	AZ_FLAG_REFERENCE = 0x04000000,
	/**
	 * @brief Instance if a container of another value (_AZBoxedValue) or interface (_AZBoxedInterface) type
	 * 
	 */
	AZ_FLAG_BOXED = 0x08000000,
	/**
	 * @brief A special reference type that contains a pointer to it's class
	 * 
	 */
	AZ_FLAG_OBJECT = 0x10000000,
	/**
	 * @brief Type is final, no further subclassing is allowed
	 * 
	 */
	AZ_FLAG_FINAL = 0x20000000,
	/**
	 * @brief The type instances have constructor or destructor
	 * 
	 * Subclasses should not clear this flag if set set by parent.
	 */
	AZ_FLAG_CONSTRUCT = 0x40000000,
	/* fixme: Make this dependent on construction */
	/**
	 * @brief Instance construction should be preceded by filling memory by zeroes
	 * 
	 * Subclasses should not clear this flag if set set by parent. If set the type can still implement
	 * constructor - which can then rely on the instance being zero-filled.
	 */
	AZ_FLAG_ZERO_MEMORY = 0x80000000,

	/*
	* Low-order flags, only present in class and type info
	*/

	/**
	 * @brief Type is abstract, no instancing is allowed
	 * 
	 * This flag is NOT propagated to subclasses.
	 * 
	 */
	AZ_FLAG_ABSTRACT = 0x02,

	/* Miscellaneous info flags */
	AZ_FLAG_ARITHMETIC = 0x100,
	AZ_FLAG_INTEGRAL = 0x200,
	AZ_FLAG_SIGNED = 0x400
};

/*
 * Every entity instance is a collection of bits in memory
 * The layout, meaning and basic handling of these bits is described by class
 * Classes are accessible either by class pointer or integer type
 * Polymorphic parts of an entity is described either in class or implementation
 * For standalone types class is the implementation (no implementation declared separately)
 * Implementations of interface types can be nested in other classes and implementations
 * Class is itself an instance of standalone type
 * Both classes and implementation can contain sub-implementations of other types
 */

/**
 * @brief Polymorphic parts of a type
 */
typedef struct _AZImplementation AZImplementation;

/**
 * @brief Semantics of a type
 * 
 */
typedef struct _AZClass AZClass;

/* Instance is accessed via void pointer */

/* Transferable instance */
typedef struct _AZValue AZValue;
typedef struct _AZValue64 AZValue64;

/* Members and properties */
typedef struct _AZField AZField;

/* Embeddable type, interface instance does not have members */
typedef struct _AZInterfaceClass AZInterfaceClass;

/* Predeclarations of base types */
typedef struct _AZReference AZReference;
typedef struct _AZString AZString;
typedef struct _AZBoxedValue AZBoxedValue;
typedef struct _AZBoxedInterface AZBoxedInterface;
#ifdef AZ_HAS_PACKED_VALUE
typedef struct _AZPackedValue AZPackedValue;
typedef struct _AZPackedValue64 AZPackedValue64;
#endif
typedef struct _AZObject AZObject;

typedef struct _AZFunctionSignature AZFunctionSignature;
typedef struct _AZFunctionImplementation AZFunctionImplementation;
typedef struct _AZFunctionInstance AZFunctionInstance;

/* Execution context */
typedef struct _AZContext AZContext;

/**
 * @brief Initialize type system
 * 
 * If AZ_SAFETY_CHECKS is set during the compilation of the library, it is called automatically
 * whenever any method involving types is called.
 * 
 * It is safe to call it more than once
 */

void az_init (void);

#ifdef __cplusplus
}
#endif

#endif
