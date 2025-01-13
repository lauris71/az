# az
C runtime type system and class library

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

## 2. Fundamental philosophy
The building blocks of az are:
- instance
- value
- class
- implementation

### 2.1.  Instance
Instance is the actual data.

Each instance coresponds to some bits in memory.

These bits may be unique (for block types) or copy/pasted during execution (value types).

### 2.2. Value
Value is the „handle” of instance data visible to library.

For value types this is the instance itself (i.e. if instance A contains instance B then A actually contains all the bits of B).

For block types the value is pointer to instance (i.e. A only contains pointer, the bits of B reside somewhere else in memory).

### 2.3. Implementation and class
For plain (non-interface) types these are the same.

Implementation contains polymorphic parts of static data (commonly virtual methods, but nothing forbids them to have „virtual data fields” also). One can think about implementation as the definiton „how these bits behave”.

Class contains the type definition of data „what these bits are”.

All classes are itself implementations (thus they can contain virtual methods and virtual data). For standalone instances this is all the polymorphism needed. But for interfaces the distinction is important. Interfaces are implemented (in different ways) inside other instances, while retaining the base type information (class), and thus part of their polymorphic data has to reside inside the implementation (or class) of the containing instance.

Which virtual methods of interface belong to class and which to implementation depends on the actual data type.

## 3. Basic type hierachy
>Any
>  Value
>  Block
>    Reference
>      Object

## 4. Special data typess
- Boxed value
- Boxed interface
- Object

### 4.1. Boxed value
Boxed value contains the type (i.e. pointer to Implementation) and instance (i.e. the actual bits) of a value type inside reference.

These are needed to pass a value of unknown size (e.g. reading a property with ’Any’ type)

### 4.2. Boxed interface
Boxed interface contains a value (i.e. pointer to the implementation and value) of containing instance and an interface (i.e. pointer to the implementation and pointer to the intance) defined in the former.

Interfaces are „owned” by containing instance. Thus methods cannot extend the lifecycle of interface beyond the duration of the instance. But in many situations one may want to extend it – for example to create a collection of interfaces.

Unless the external application logic dictates that the interfaces remain valid during the existence of such collection, boxed interface can be used instead. Boxed interface contains the value of owner and thus also the interface of interest. The interface is directly accessible (does not need to be queried if the instance type layout is not familiar to the caller).

### 4.3. Object
Object is a special reference type that ’knows’ it’s class (i.e. has a pointer to the class inside instance). Thus, unlike all other types, objects can be accessed simply by value without the need of implementation.

## 5. Specifying instances
All classes are assigned integer type values. These integer values are usually used instead of class pointers to specify type.

All implementations contain data type as the first member, thus class can always be derived from implementation.

Data is generally accessed by specifying its instance and implementation.

If type is final, implementation can be omitted.

If a method does not use polymorphic parts of data, implementation can be omitted

Object types contain class as the first memeber, thus these do not need specifying separate implementation

