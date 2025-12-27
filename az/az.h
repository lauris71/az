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

enum {
	/* Invalid or missing type, typecode 0 */
	AZ_TYPE_NONE,
	/* Universal base class */
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
	/* Struct is base for all composite value types */
	AZ_TYPE_STRUCT,
	/* Block is the base of all composite reference types */
	AZ_TYPE_BLOCK,
	/* Fundamental types have ANY as parent */
	AZ_NUM_FUNDAMENTAL_TYPES = AZ_TYPE_BLOCK,

	/* Special types */
	AZ_TYPE_IMPLEMENTATION,
	AZ_TYPE_CLASS,
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

/* Semantics of a type */
typedef struct _AZClass AZClass;

/**
 * @brief Polymorphic parts of a type
 */
typedef struct _AZImplementation AZImplementation;

/* Instance is accessed via void pointer */

/* Transferable handle */
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
 * whenever any type is accessed
 * 
 * It is safe to call it more than once
 */

void az_init (void);

#ifdef __cplusplus
}
#endif

#endif
