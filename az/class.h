#ifndef __AZ_CLASS_H__
#define __AZ_CLASS_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#include <az/types.h>

typedef struct _AZIFEntry AZIFEntry;
typedef struct _AZInstanceAllocator AZInstanceAllocator;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * High-order flags, can be put in typecode
 */
/* Interface type, implementation is not class */
#define AZ_FLAG_INTERFACE 0x01000000
/* Instance is referenced by pointer */
#define AZ_FLAG_BLOCK 0x02000000
/* Reference type, copying needs reference counting */
#define AZ_FLAG_REFERENCE 0x04000000
/* Instance is a container of another type */
#define AZ_FLAG_BOXED 0x08000000
/* Instance is a container of another type */
#define AZ_FLAG_OBJECT 0x10000000
/* No subclasses */
#define AZ_FLAG_FINAL 0x20000000
/* Subclasses should not remove flag set by parent */
#define AZ_FLAG_CONSTRUCT 0x40000000
/* fixme: Make this dependent on construction */
#define AZ_FLAG_ZERO_MEMORY 0x80000000

/*
 * Low-order flags, only present in class and type info
 */

/* This has to be one of the lowest 3 bits to differentiate the pointer/flags union */

/**
 * @brief Marks that implementation is a standalone class
 * 
 * It exploits the feature that classes are aligned to 8 bytes, thus if any of the
 * lowest bits are set, the AZImplementation union contains flags and type, not a
 * pointer to the AZClass
 */
#define AZ_FLAG_IMPL_IS_CLASS 0x01

/* No instancing is allowed (this is not propagated to subclasses) */
#define AZ_FLAG_ABSTRACT 0x02

/* Miscellaneous info flags */
#define AZ_FLAG_ARITHMETIC 0x04
#define AZ_FLAG_INTEGRAL 0x08
#define AZ_FLAG_SIGNED 0x10

/**
 * @brief Superclass of all implementations and classes
 * 
 * The flag AZ_FLAG_IMPL_IS_CLASS marks whether it is class or standalone implementation. In the
 * former case the union contains flags and type, otherwise pointer to the class.
 * We use the fact that pointers are aligned to 8 bytes and thus the 3 lowest bits, including the
 * flag bit, are always zero.
 */

struct _AZImplementation {
	union {
		struct {
			uint32_t flags;
			uint32_t type;
		};
		AZClass *klass;
	};
};

/**
 * @brief true if AZImplementation is standalone AZClass
 * 
 */
#define AZ_IMPL_IS_CLASS(i) ((i)->flags & AZ_FLAG_IMPL_IS_CLASS)
#define AZ_CLASS_FROM_IMPL(i) (AZ_IMPL_IS_CLASS(i) ? (AZClass *) (i) : (i)->klass)

#define AZ_IMPL_TYPE(i) (AZ_IMPL_IS_CLASS(i) ? (i)->type : (i)->klass->impl.type)
#define AZ_IMPL_FLAGS(i) (AZ_IMPL_IS_CLASS(i) ? (i)->flags : (i)->klass->impl.flags)
/* Interfaces are blocks */
#define AZ_IMPL_IS_BLOCK(i) (!AZ_IMPL_IS_CLASS(i) || ((i)->flags &  AZ_FLAG_BLOCK))
#define AZ_IMPL_IS_VALUE(i) (AZ_IMPL_IS_CLASS(i) && !((i)->flags & AZ_FLAG_BLOCK))
#define AZ_IMPL_IS_REFERENCE(i) (AZ_IMPL_IS_CLASS(i) && ((i)->flags & AZ_FLAG_REFERENCE))
#define AZ_IMPL_IS_BOXED_VALUE(i) ((i) == &AZBoxedValueKlass.klass.impl))
#define AZ_IMPL_IS_BOXED_INTERFACE(i) ((i) == &AZBoxedInterfaceKlass.klass.impl))

#define AZ_CLASS_TYPE(c) ((c)->impl.type)
#define AZ_CLASS_FLAGS(c) ((c)->impl.flags)
#define AZ_CLASS_IS_ABSTRACT(c) ((c)->impl.flags & AZ_FLAG_ABSTRACT)
#define AZ_CLASS_IS_BLOCK(c) !((c)->impl.flags & AZ_FLAG_BLOCK)
#define AZ_CLASS_IS_VALUE(c) !((c)->impl.flags & AZ_FLAG_BLOCK)
#define AZ_CLASS_IS_REFERENCE(c) !((c)->impl.flags & AZ_FLAG_REFERENCE)
#define AZ_CLASS_IS_BOXED_VALUE(c) ((c) == &AZBoxedValueKlass))
#define AZ_CLASS_IS_BOXED_INTERFACE(c) ((c) == &AZBoxedInterfaceKlass))
#define AZ_CLASS_IS_INTERFACE(c) ((c)->impl.flags & AZ_FLAG_INTERFACE)

#define AZ_CLASS_IS_FINAL(c) ((c)->impl.flags & AZ_FLAG_FINAL)
#define AZ_CLASS_VALUE_SIZE(c) (((c)->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : (c)->instance_size)
#define AZ_CLASS_ELEMENT_SIZE(c) (((c)->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : ((c)->instance_size + (c)->alignment) & ~(c)->alignment)

struct _AZIFEntry {
	uint32_t type;
	uint16_t impl_offset;
	uint16_t inst_offset;
};

struct _AZInstanceAllocator {
	void *(*allocate) (AZClass *klass);
	void *(*allocate_array) (AZClass *klass, unsigned int n_elements);
	void (*free) (AZClass *klass, void *location);
	void (*free_array) (AZClass *klass, void *location, unsigned int n_elements);
};

struct _AZClass {
	AZImplementation impl;

	AZClass *parent;

	uint16_t n_ifaces_self;
	uint16_t n_ifaces_all;
	uint16_t n_props_self;
	uint16_t _filler;
	union {
		AZIFEntry ifaces[2];
		struct {
			/* Interfaces implemented here */
			AZIFEntry *ifaces_self;
			/* Interface chain (ascending, interface then sub-interfaces */
			AZIFEntry *ifaces_all;
		};
	};

#ifdef AZ_HAS_PROPERTIES
	AZField *props_self;
#endif

	const uint8_t *name;
	/* Alignment mask: 0 (1), 1 (2), 3 (4), 7 (8) or 15 (16) */
	uint16_t alignment;
	/* Size of class structure */
	uint16_t class_size;
	/* Size of instance */
	uint32_t instance_size;

	/* Memory management */
	AZInstanceAllocator *allocator;

	/* Constructors and destructors */
	void (*instance_init) (const AZImplementation *impl, void *inst);
	void (*instance_finalize) (const AZImplementation *impl, void *inst);

	/* Serialization is by instance */
	/* Return number of bytes that should have been written (regardless of dlen) */
	/* It is safe to set d to NULL */
	/* Returns the number of bytes that would have been written if there was enough room in destination */
	unsigned int (*serialize) (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
	/* Deserialization is by value, i.e. new instances of reference types should be created */
	/* Returns the number of bytes consumed (0 on error) */
	unsigned int (*deserialize) (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);

	unsigned int (*to_string) (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen);
#ifdef AZ_HAS_PROPERTIES
	/**
	* @brief Get property value
	* @param impl An implementation of query instance
	* @param inst A query instance
	* @param idx An index of the property as declared in query class
	* @param prop_impl The returned implementation
	* @param prop_val The returned value
	* @param ctx The execution context
	* @return 1 on success, 0 on error
	*
	* Get indexed property of the instance.
	* Property is returned by value.
	* Any and non-final value type properties should accept NULL value to read exact type
	*/
	unsigned int (*get_property) (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);
	/* Property is set by instance */
	/* Returns 1 on success, 0 if property cannot be set */
	unsigned int (*set_property) (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
#endif
};

/*
 * We trade some branching for cache locality here
 *
 * n_ifaces_self == n_ifaces_all:
 *   n_ifaces_self <= 2 : self, all = ifaces[0..1]
 *   n_ifaces_self > 2 : self, all = ifaces_self
 * n_ifaces_self < n_ifaces_all:
 * 	 n_ifaces_self == 0:
 *     n_ifaces_all <= 2 : all = ifaces[0..1]
 *     n_ifaces_all > 2 : all = ifaces_all
 *   n_ifaces_self == 1:
 *     n_ifaces_all == 2 : self, all = ifaces[0..1]
 *     n_ifaces_all > 2 : self = ifaces[0], all = ifaces_all
 *   n_ifaces_self > 1 : self = ifaces_self, all = ifaces_all
 */

static inline const AZIFEntry *
az_class_ifaces_self(const AZClass *klass)
{
	if (klass->n_ifaces_self == klass->n_ifaces_all) {
		return (klass->n_ifaces_self <= 2) ? &klass->ifaces[0] : &klass->ifaces_self[0];
	} else {
		return (klass->n_ifaces_self <= 1) ? &klass->ifaces[0] : &klass->ifaces_self[0];
	}
}

static inline const AZIFEntry *
az_class_iface_self(const AZClass *klass, uint16_t idx)
{
	return az_class_ifaces_self(klass) + idx;
}

static inline const AZIFEntry *
az_class_ifaces_all(const AZClass *klass)
{
	if (klass->n_ifaces_self == klass->n_ifaces_all) {
		return (klass->n_ifaces_self <= 2) ? &klass->ifaces[0] : &klass->ifaces_all[0];
	} else {
		if (klass->n_ifaces_self <= 1) {
			return (klass->n_ifaces_all <= 2) ? &klass->ifaces[0] : &klass->ifaces_all[0];
		} else {
			return &klass->ifaces_all[0];
		}
	}
}

static inline const AZIFEntry *
az_class_iface_all(const AZClass *klass, uint16_t idx)
{
	return az_class_ifaces_all(klass) + idx;
}

static inline unsigned int
az_class_value_size(const AZClass *klass)
{
	return (klass->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : klass->instance_size;
}

static inline AZClass *
az_class_parent(const AZClass *klass) {
	return klass->parent;
}

#ifdef __cplusplus
};
#endif

#endif
