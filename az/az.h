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

/** @ingroup types
 * @brief Type flags
 * 
 * These are high-level flags that describe the most important type/class attributes.
 * 
 * They are stored in the typecode (bits 31-24), so they are available directly from typecode without accessing the
 * class array (saving pointer dereference and potential thread synchronization).
 */
enum AZTypeFlags {
	/**
	 * @brief Type is a block, value is a pointer to the instance (not the instance itself)
	 * 
	 */
	AZ_FLAG_BLOCK = 0x01000000,
	/**
	 * @brief Type is an interface, implementation is not class
	 * 
	 */
	AZ_FLAG_INTERFACE = 0x02000000,
	/**
	 * @brief Type is a reference, value creation/destruction involves reference counting
	 * 
	 */
	AZ_FLAG_REFERENCE = 0x04000000,
	/**
	 * @brief Instance is a container of another value (_AZBoxedValue) or interface (_AZBoxedInterface) type
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
	 * @brief Type is abstract, no instancing is allowed
	 * 
	 * This flag is NOT propagated to subclasses.
	 * 
	 */
	AZ_FLAG_ABSTRACT = 0x40000000,
	/**
	 * @brief The type instances have constructor or destructor
	 * 
	 * Subclasses should not clear this flag if set set by parent.
	 * This flags is automatically set during type registration if:
	 * - the type has constructor (init) or destructor (finalize)
	 * - AZ_FLAGS_ZERO_MEMORY is set
	 * - class implements interfaces or has properties
	 */
	AZ_FLAG_CONSTRUCT = 0x80000000,
};

/** @ingroup types
 * @brief Implementation/Class flags
 * 
 * These are stored in lower-order bits (bits 23-0) and either describe the implementation behaviour or less-used type
 * hierarchy information and are stored only in the class (implementation) flags field.
 * 
 * The AZ_FLAG_IMPL_IS_CLASS flag is a special case: it is stored in the implementation flags and marks
 * whether the implementation is a class (containing flags field) or a standalone implementation of an
 * interface (containing pointer to the class instead).
 */
enum AZImplementationFlags {
	/**
	 * @brief Marks that an implementation is a standalone class
	 * 
	 * It exploits the feature that classes are aligned to 8 bytes. Thus, if any of the 3
	 * lowest bits is set, the AZImplementation union contains flags and type, not a
	 * pointer to the AZClass
	 */
	AZ_FLAG_IMPL_IS_CLASS = 0x01,
	/**
	 * @brief Instance construction should be preceded by filling memory by zeroes
	 * 
	 * Subclasses should not clear this flag if set set by parent. If set the type can still implement
	 * constructor - which can then rely on the instance being zero-filled.
	 */
	AZ_FLAG_ZERO_MEMORY = 0x04,

	/* Miscellaneous info flags */
	AZ_FLAG_ARITHMETIC = 0x100,
	AZ_FLAG_INTEGRAL = 0x200,
	AZ_FLAG_SIGNED = 0x400
};

/** @ingroup types
 * @brief Predefined type indices
 *
 * These are stored in the upper-order bits (bits 31-24) of the typecode. The index is used to
 * access the type information (class) in the single global array.
 */

enum AZTypeIdx {
	AZ_TYPE_IDX_NONE,
	AZ_TYPE_IDX_ANY,
	AZ_TYPE_IDX_BOOLEAN,
	AZ_TYPE_IDX_INT8,
	AZ_TYPE_IDX_UINT8,
	AZ_TYPE_IDX_INT16,
	AZ_TYPE_IDX_UINT16,
	AZ_TYPE_IDX_INT32,
	AZ_TYPE_IDX_UINT32,
	AZ_TYPE_IDX_INT64,
	AZ_TYPE_IDX_UINT64,
	AZ_TYPE_IDX_FLOAT,
	AZ_TYPE_IDX_DOUBLE,
	AZ_TYPE_IDX_COMPLEX_FLOAT,
	AZ_TYPE_IDX_COMPLEX_DOUBLE,
	AZ_TYPE_IDX_POINTER,
	AZ_TYPE_IDX_STRUCT,
	AZ_TYPE_IDX_BLOCK,
	AZ_TYPE_IDX_IMPLEMENTATION,
	AZ_TYPE_IDX_CLASS,
	AZ_TYPE_IDX_INTERFACE,
	AZ_TYPE_IDX_FIELD,
	AZ_TYPE_IDX_FUNCTION_SIGNATURE,
	AZ_TYPE_IDX_FUNCTION,
	AZ_TYPE_IDX_REFERENCE,
	AZ_TYPE_IDX_STRING,
	AZ_TYPE_IDX_BOXED_VALUE,
	AZ_TYPE_IDX_BOXED_INTERFACE,
	AZ_TYPE_IDX_PACKED_VALUE,
	AZ_TYPE_IDX_OBJECT
};

/* Fundamental types have ANY as parent */
#define AZ_NUM_FUNDAMENTAL_TYPES (AZ_TYPE_IDX_BLOCK + 1)
#define AZ_NUM_BASE_TYPES (AZ_TYPE_IDX_OBJECT + 1)

/** @ingroup types
 * @brief Predefined typecodes
 *
 * The actual typecodes (index plus flags).
 * 
 * These are used througout the api as a more compact type identifier than class pointer.
 * Semantically they are synonymous to a class, NOT an implementation - the interfaces have to
 * be accessed through an Implementation/Instance pointer pair because typecode only gives access
 * to the class of the interface, not to it's actual implementation.
 */

enum AZType {
	/**
	 * @brief Invalid or missing type, typecode 0
	 * 
	 */
	AZ_TYPE_NONE = AZ_TYPE_IDX_NONE,
	/**
	 * @brief The abstract base class of all types
	 * 
	 */
	AZ_TYPE_ANY = AZ_TYPE_IDX_ANY | AZ_FLAG_ABSTRACT,

	/* Primitives */
	AZ_TYPE_BOOLEAN = AZ_TYPE_IDX_BOOLEAN | AZ_FLAG_FINAL,
	AZ_TYPE_INT8 = AZ_TYPE_IDX_INT8 | AZ_FLAG_FINAL,
	AZ_TYPE_UINT8 = AZ_TYPE_IDX_UINT8 | AZ_FLAG_FINAL,
	AZ_TYPE_INT16 = AZ_TYPE_IDX_INT16 | AZ_FLAG_FINAL,
	AZ_TYPE_UINT16 = AZ_TYPE_IDX_UINT16 | AZ_FLAG_FINAL,
	AZ_TYPE_INT32 = AZ_TYPE_IDX_INT32 | AZ_FLAG_FINAL,
	AZ_TYPE_UINT32 = AZ_TYPE_IDX_UINT32 | AZ_FLAG_FINAL,
	AZ_TYPE_INT64 = AZ_TYPE_IDX_INT64 | AZ_FLAG_FINAL,
	AZ_TYPE_UINT64 = AZ_TYPE_IDX_UINT64 | AZ_FLAG_FINAL,
	AZ_TYPE_FLOAT = AZ_TYPE_IDX_FLOAT | AZ_FLAG_FINAL,
	AZ_TYPE_DOUBLE = AZ_TYPE_IDX_DOUBLE | AZ_FLAG_FINAL,
	AZ_TYPE_COMPLEX_FLOAT = AZ_TYPE_IDX_COMPLEX_FLOAT | AZ_FLAG_FINAL,
	AZ_TYPE_COMPLEX_DOUBLE = AZ_TYPE_IDX_COMPLEX_DOUBLE | AZ_FLAG_FINAL,
	AZ_TYPE_POINTER = AZ_TYPE_IDX_POINTER | AZ_FLAG_FINAL,

	/* Fundamental types */
	/**
	 * @brief The abstract base class of all composite value types
	 * 
	 */
	AZ_TYPE_STRUCT = AZ_TYPE_IDX_STRUCT | AZ_FLAG_ABSTRACT,
	/**
	 * @brief The abstract base class of all block types
	 * 
	 */
	AZ_TYPE_BLOCK = AZ_TYPE_IDX_BLOCK | AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT,

	/* Special types */
	/**
	 * @brief The abstract base class of the semantics of a type
	 * 
	 * An instance of _AZImplementation. Specialized for interface types, _AZClass for standalone types
	 * 
	 */
	AZ_TYPE_IMPLEMENTATION = AZ_TYPE_IDX_IMPLEMENTATION | AZ_FLAG_BLOCK,
	/**
	 * @brief The semantics of a type
	 * 
	 * An instance of _AZClass, a subclass of _AZImplementation
	 * 
	 */
	AZ_TYPE_CLASS = AZ_TYPE_IDX_CLASS | AZ_FLAG_BLOCK | AZ_FLAG_FINAL,
	/**
	 * @brief An abstract base class of an instance of interface type
	 * 
	 */
	AZ_TYPE_INTERFACE = AZ_TYPE_IDX_INTERFACE | AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE,
	AZ_TYPE_FIELD = AZ_TYPE_IDX_FIELD | AZ_FLAG_BLOCK | AZ_FLAG_FINAL,
	AZ_TYPE_FUNCTION_SIGNATURE = AZ_TYPE_IDX_FUNCTION_SIGNATURE | AZ_FLAG_BLOCK | AZ_FLAG_FINAL,
	AZ_TYPE_FUNCTION = AZ_TYPE_IDX_FUNCTION | AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE,
	/* Predefined composite types */
	/**
	 * @brief A block type that implements reference counting
	 * 
	 */
	AZ_TYPE_REFERENCE = AZ_TYPE_IDX_REFERENCE | AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_REFERENCE,
	/**
	 * @brief An immutable reference-counted utf8 string type
	 * 
	 * The strings having the same textual value are collated using an hash table. Thus string equality
	 * can be checked by comparing the pointers.
	 * 
	 */
	AZ_TYPE_STRING = AZ_TYPE_IDX_STRING | AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_REFERENCE,
	/**
	 * @brief A reference that can contain an arbitrary value-type
	 * 
	 * These are mostly used to fit any value type into predefined storage space
	 * 
	 */
	AZ_TYPE_BOXED_VALUE = AZ_TYPE_IDX_BOXED_VALUE | AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED,
	/**
	 * @brief A reference that contain a value and resolved interface of it
	 * 
	 * These are used to store interfaces by guaranteeing that the containing instance remains alive (and
	 * in case of value types at the same place) during the interface lifecycle.
	 */
	AZ_TYPE_BOXED_INTERFACE = AZ_TYPE_IDX_BOXED_INTERFACE | AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_REFERENCE | AZ_FLAG_BOXED,
	/**
	 * @brief A convenience container that stores both a value and a pointer to it's implementation
	 * 
	 */
	AZ_TYPE_PACKED_VALUE = AZ_TYPE_IDX_PACKED_VALUE | AZ_FLAG_BLOCK | AZ_FLAG_FINAL,
	/**
	 * @brief A special reference type that contains a pointer to it's own class
	 * 
	 * Objects can be used without specifying their implementations.
	 * 
	 */
	AZ_TYPE_OBJECT = AZ_TYPE_IDX_OBJECT | AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_REFERENCE | AZ_FLAG_OBJECT,
};

#define AZ_TYPE_IS_ARITHMETIC(t) ((AZ_TYPE_INDEX(t) >= AZ_TYPE_IDX_INT8) && (AZ_TYPE_INDEX(t) <= AZ_TYPE_IDX_COMPLEX_DOUBLE))
#define AZ_TYPE_IS_INTEGRAL(t) ((AZ_TYPE_INDEX(t) >= AZ_TYPE_IDX_INT8) && (AZ_TYPE_INDEX(t) <= AZ_TYPE_IDX_UINT64))
#define AZ_TYPE_IS_SIGNED(t) (((t) == AZ_TYPE_INT8) || ((t) == AZ_TYPE_INT16) || ((t) == AZ_TYPE_INT32) || ((t) == AZ_TYPE_INT64) || ((t) == AZ_TYPE_FLOAT) || ((t) == AZ_TYPE_DOUBLE) || ((t) == AZ_TYPE_COMPLEX_FLOAT) || ((t) == AZ_TYPE_COMPLEX_DOUBLE))
#define AZ_TYPE_IS_UNSIGNED(t) (((t) == AZ_TYPE_UINT8) || ((t) == AZ_TYPE_UINT16) || ((t) == AZ_TYPE_UINT32) || ((t) == AZ_TYPE_UINT64))
#define AZ_TYPE_IS_PRIMITIVE(t) ((AZ_TYPE_INDEX(t) >= AZ_TYPE_IDX_BOOLEAN) && (AZ_TYPE_INDEX(t) <= AZ_TYPE_IDX_POINTER))
#define AZ_TYPE_IS_BASE(t) ((AZ_TYPE_INDEX(t) >= AZ_TYPE_IDX_ANY) && (AZ_TYPE_INDEX(t) <= AZ_TYPE_IDX_OBJECT))

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
typedef struct _AZPackedValue AZPackedValue;
typedef struct _AZPackedValue64 AZPackedValue64;
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
 * in AZ_GLOBALS_STATIC and AZ_GLOBALS_SINGLE_THREAD modes whenever any method involving types is called.
 * 
 * It is safe to call it more than once
 */

void az_init (void);

#ifdef __cplusplus
}
#endif

#endif
