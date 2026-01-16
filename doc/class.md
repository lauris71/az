# Class

## Class and implementation

Class (AZClass) contains all the semantic (what this type IS) and some behavioral (how this type BEHAVES) information of a type.
For standalone types this is everything there is, but for interface types some parts of behavior are separated into implementation.

Class itself is an implementation, but this is purely semantic connection to maske all types accessible by implementation/instance
combo. Tehnically the layouts of Class and Implementation are different:

```c
struct _AZImplementation {
    union {
        struct {
            uint32_t flags;
            uint32_t type
        };
        /* The class pointer is used in standalone implementations */
        AZClass *klass;
    };
};

struct _AZClass {
    /* The flags/type struct part of the union is used in class */
    AZImplementation impl;
    ...
};
```
Whether any given `_AZImplementation` pointer is true implementation or a class can be determined by by the lowest bit in the first data field.
It is set for proper class (flags have AZ_FLAG_IMPL_IS_CLASS set) and unset for an implementation (classes are aligned to at least 8 bytes
and thus the 3 lowest order bits in class pointer are zeroes).

The actual class can thus always be derived from implementation:

```c
AZClass *klass = (impl->flags & AZ_FLAG_IMPL_IS_CLASS) ? (AZClass *) impl : impl->klass;
```

This is done by convenience macro `AZ_CLASS_FROM_IMPL(impl)`

The full `_AZClass` specification

## Remarks