# az
C runtime type system and class library

## Features

Provides a way to manage full runtime type system for both C builtin types and
arbitrary composite types (C structs).

- construction and destruction
- inheritance
- polymorphism
- multiple inheritance (Java style interfaces)
- reflection

It also implements a set of convenient types

- references
- objects
- active objects (with signaling and attributes)
- collection interfaces
- function interface

It is by default compiled as drop-in static library.

## Building

It needs arikkei for some low-level string methods. Just clone [arikkei](https://github.com/lauris71/arikkei) to accessible place (either to the root of the main project or az).
Then, in the az project directory execute:

    cmake -S . -B build
    cmake --build build

## Motivation
### Frustration with C++
1. No unified ABI. Overloading virtual method dispatch would help a lot, but this is not possible
2. No standardized RTTI. Especially no RTTI for primitive types
3. Ugly and bloated syntax, especially if std::* is used
4. No clean POD syntax (struct should have been forced POD)
5. No manual aliasing of name mangling
6. Braindead multiple inheritance (Java got it almost right with object/interface)

### Frustration with Glib
1. For some reason it is not drop’n’build
2. Gobject is too complex/opaque with signals etc.
3. No RTTI for primitive types

## Fundamental ideas
The building blocks of az are:
- instance
- value
- class
- implementation

### Instance
Instance is the actual data. Each instance is just some number of bits somewhere in memory.

These bits may be unique (for block types) or copied around (value types) during execution.

### Value
Value is the "handle" of instance data visible to library.

For value types this is the instance itself (i.e. if instance A contains instance B of a value type, then A actually contains all the bits of B).

For block types the value is pointer to instance (i.e. if instance A contains instance C of a block type, then A only contains pointer, the bits of B reside somewhere else).

We use the term "block" because "reference" is a specific subtype (implements reference counting) of block.

### Implementation and class
For plain (non-interface) types these are the same.

Implementation contains polymorphic parts of static data (commonly virtual methods, but nothing forbids them to have virtual data fields also). One can think about implementation as the definiton "how these bits of instance behave".

Class contains the type definition of data "what these bits of instance are".

All classes are itself implementations (thus they can contain virtual methods and virtual data). For standalone instances this is all the polymorphism needed. But for interfaces the distinction is important.

Interfaces are implemented (in different ways) inside other instances. All implementations of the same interface have the same base type information (class), and thus part of their polymorphic data has to be specified by the implementation of the containing instance. Normally this is done by embedding interface implementation insde the containing implementation (or class).

Which virtual methods of interface belong to the class and which to the implementation depends on the semantics of the interface and methods.

## Basic type hierachy
>Any
>  + Value types (there is no common value superclass)
>  + Block
>    + Implementation
>      + Class
>    + Interface
>    + Reference
>      + Boxed value
>      + Boxed interface
>      + String
>      + Object

## Special data types
- Boxed value
- Boxed interface
- Object

### Boxed value
Boxed value contains the type (i.e. pointer to an Implementation) and instance (i.e. the actual bits) of a value type inside reference.

These are needed to retrieve a value of unknown size (e.g. reading a property with "Any" type)

### Boxed interface
Boxed interface contains value (i.e. pointer to the implementation and the value) of the containing instance and an interface (i.e. pointer to the implementation and pointer to the intance) implemented in the former.

Interfaces are "owned" by the containing instance. Thus methods cannot extend the lifecycle of interface beyond the duration of the instance. But in many situations one may want to extend it – e.g. to create a collection of interfaces.

Unless the external application logic dictates that the interfaces remain valid during the existence of such collection, boxed interface can be used instead. Boxed interface contains the value of owner and thus also the interface of interest. The interface is directly accessible (does not need to be queried if the instance type layout is not familiar to the caller).

### Object
Object is a special reference type that "knows" it’s class (i.e. has a pointer to the class inside instance). Thus, unlike other non-final types, objects can be accessed simply by value without the need of specifying implementation.

## Specifying instances
All classes are assigned integer type values. These integer values are usually used instead of class pointers to specify a type.

All implementations contain type value as the first member, thus class can always be obtained from an implementation.

Data is generally accessed by specifying its implementation and instance of implementation and value.

    my_type_do_something(const AZImplementation *impl, void *inst);
    my_type_do_something_else(const AZImplementation *impl, AZValue *val);

To perform some action with type the first (instance) is usually more convenient. The latter is commonly used for polymorphic collections (instances of block types cannot be put into collections).

To retreve polymorphic data, both implementaton and value has to be retrieved (it has to be value because instances of block types cannot normally be created in stack).

    AZImplementation *impl = az_parse_type(const uint8_t *buf, AZValue *val);

If type is final, implementation can be omitted in methods.

If a method does not use polymorphic parts of data, implementation can be omitted

Object types contain class as the first memeber, thus methods of objects do not have to specify implementation.

## Some notes

Polymorphism in C means that there are strict limits to both compile-time and run-time type safety. But imo type safety is currently much overhyped. If you need compile-time type safety to protect yourself against bugs you have much bigger problems and probably should not write complex code at all.

Objects provide run-time type safety but this is not free. For more primitive types one has to rely on good
programming practice (clean structure, clean naming conventions etc.)

Creating reflexive types involves writing non-trivial amount of boilerplate code (writing class structure, class and instance initializers, field specifications and so on.) Imo it is worth it.