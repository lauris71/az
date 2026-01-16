#ifndef __AZ_CLASS_H__
#define __AZ_CLASS_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#include <stdint.h>

#include <az/az.h>

typedef struct _AZIFEntry AZIFEntry;
typedef struct _AZInstanceAllocator AZInstanceAllocator;

#ifdef __cplusplus
extern "C" {
#endif

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
			/**
			 * @brief A meaningful combination of `AZTypeFlags`
			 * 
			 */
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
	/**
	 * @brief Pointer to the parent class
	 * 
	 */
	AZClass *parent;

	/**
	 * @brief The number of interfaces declared in this class
	 * 
	 */
	uint16_t n_ifaces_self;
	/**
	 * @brief The number of interfaces implemented in this class
	 * 
	 * The sum of interfaces declared in given class, in all it's parent classes,
	 * in all it's interfaces and in the parent classes of interfaces.
	 * 
	 */
	uint16_t n_ifaces_all;
	/**
	 * @brief The number of properties declared in this class
	 * 
	 */
	uint16_t n_props_self;
	/**
	 * @brief Alignment filler
	 * 
	 */
	uint16_t _filler;
	union {
		/**
		 * @brief Interface declarations if (n_ifaces_self + n_ifaces_all) <= 2
		 * 
		 */
		AZIFEntry ifaces[2];
		struct {
			/**
			 * @brief Interface declarations of this class
			 * 
			 */
			AZIFEntry *ifaces_self;
			/**
			 * @brief Interface declarations of this class, all it's parents and sub-interfaces
			 * 
			 * Listed in ascending order (fist all declared interfaces with their sub-interfaces, then the interfaces of parent class...)
			 * 
			 */
			AZIFEntry *ifaces_all;
		};
	};

#ifdef AZ_HAS_PROPERTIES
	AZField *props_self;
#endif

	/**
	 * @brief The name of this class for convenience (not used by the library)
	 * 
	 */
	const uint8_t *name;
	/**
	 * @brief the alignment mask: 0 (1), 1 (2), 3 (4), 7 (8) or 15 (16)
	 * 
	 */
	uint16_t alignment;
	/**
	 * @brief The size of the class structure
	 * 
	 */
	uint16_t class_size;
	/**
	 * @brief The size of the instance of this type 
	 * 
	 */
	uint32_t instance_size;

	/**
	 * @brief Memory allocator
	 * 
	 * This being NULL means that the default allocators (malloc/free) are used
	 * 
	 */
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

/**
 * @brief get the index, the containing class and corresponding implementation and instance of a property
 * 
 * Searches class and interface definitions recursively for a property.
 * The order is class->interface[0]->superinterfaces->interface[1]...->superclasses
 * If the index is >= 0, the property definition is def_class->props_self[index] 
 * 
 * @param klass current class (either the same as implementation or a superclass)
 * @param impl type implementation (can be null for static properties)
 * @param inst type instance (can be null for static or implementation properties)
 * @param key the property key
 * @param def_class result for the class where the property is defined
 * @param sub_impl result for the actual implementation (either impl or sub-implementation, can be null)
 * @param sub_inst result for the actual instance (either inst or sub-interface, can be null)
 * @return the property index in def_class
 */
int az_class_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);
int az_class_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);

#ifdef __cplusplus
};
#endif

#endif
