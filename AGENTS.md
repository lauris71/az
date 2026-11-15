# Overview

Az is a runtime type system and class library for C

## Basic building blocks

- instance
- class
- type
- value
- implementation
- interface
- reference
- object

## Instance

Instance is the actual data. Each instance is some number of bits somewhere in memory.

The most general subdivision of types is value vs. block.
- value - instance is stored directly (local copy inside container, stack...)
- block - instance is stored via pointer (heap allocation, shared references...)
Basically this is the value/reference distinction of most languages (but az uses 'reference' strictly for reference-counted blocks)

## Class

Class is the semantic definition of a type. Classes are permanent.

## Type

Type is an integer representation of a class. It consists of a class index in global table and a set of flags.

## Value

Value is the "handle" of instance data visible to library.
The value of value types is the full instance (the size varies per class).
The value of block types is a pointer to the block.
Values are normally stored in AZValue union for convenience.

A special reference type AZBoxedValue can be used to store value inside reference-counted block - mostly to ensure fixed value size for storage.

## Implementation

Implementation is the concrete realization of a class's behavior. It defines how functions are invoked and how values are accessed for a particular type.
For standalone types implementation is the class itself. Interface types have distinct implementation in addition to class.

## Interface

Interfaces are block types for which the implementation is separate from the class definition.
Normally the instance of an interface is contained inside other instance and implementation inside other class/implementation. But this is not strictly required -
the containment only provides interface registration to container and automatic initialization/destruction/discovery.

Interfaces themselves can contain sub-interfaces (they are just special block-types after all).

Interfaces do not manage their own lifecycle - they are managed by the container that holds them. Because of that, there is special reference type -
AZBoxedInterface that contains both the containing instance and the interface/implementation pointers.

## Reference

Reference is a reference-counted block type. It contains a reference count as the first element and manages its lifetime through reference counting.

## Object

Object is a reference type that represents an instance with a class pointer as its first element (after the reference count and flags). This allows objects to be accessed without specifying the implementation/class.

## Accessing instances

All classes are assigned integer type values. These integer values can be used instead of class pointers to specify a type.

All implementations either are classes, or contain class pointer as the first element. Thus class can always be obtained from an implementation.

Data is generally accessed by specifying its implementation + instance or implementation + value.

    my_type_do_something(const AZImplementation *impl, void *inst);
    my_type_do_something_else(const AZImplementation *impl, AZValue *val);

To perform some action with a type, the first method (instance) is usually more convenient. The latter is commonly used for polymorphic collections (instances of block types cannot be put into collections).

To retreve polymorphic data, both implementaton and value has to be retrieved (the destination has to be value because instances of block types normally can neither be copied nor created in stack).

    AZImplementation *impl = az_parse_type(const uint8_t *buf, AZValue *val);

For final types and objects the implementation pointer is usually omitted in methods because for those it either is fixed or can be derived from the instance

Methods that do not use polymorphic parts of data also often omit the implementation pointer.

## Packed value

AZPackedValue is a convenience struct that contains both AZImplementation pointer and AZValue struct. Thus is can be used as self-contained instance handle.

## Properties

Properties are named parameters of types. They are defined in AZField structs. Each property has the following attributes:
- storage type - class/implementation/instance. Determines where the actual value is stored (logically, not necessarily physically).
- read type - properties can be refer to values/packed values or instances inside the storage. Or read by generic function. Or be non-readable.
- write type - similar to read type. The same property may have different read/write types (e.g. if writing value requires recomputing some internal data).
- signature - for function properties (to support overloading)
A final function property with class storage is roughly analogous to a static member of oo language.
A final function property with instance storage is roughly analogous to a non-static member function of oo language.
A mutable function property with instance storage is roughly analogous to mutable signal handler.

## Delegation

A class can delegate the reflection methods (get_interface, get_property, set_property) to another implementation. This is used by e.g. AZReferenceOf
to forward operations to the underlying referenced object.

## String

AZString is immutable interned reference of character data. Interning guarantees, that the equality of strings can be established by pointer comparison.

## Active object

AZActiveObject is an object subtype that has additional fetures:
- signals
- attributes (arbitrary key/value pairs)
Signalling allows creating weak references of active objects (that will be cleared by dispose signal)

## Function

AZFunction is an interface type that represents a callable entity. Functions are invoked through their implementations (by calling virtual invoke method).

## Any

Any (typecode 1) is the abstract base class of all az types, both values and blocks.

## Primitives

Primitives are final types that represent the most basic datatypes:
- boolean
- int8...uint64
- float
- double
- complex float
- complex double
- pointer
