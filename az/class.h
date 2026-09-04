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

/*
 * Class allocations (heap and static) are aligned to the cache line boundary
 * and their allocation size is rounded up to a multiple of it, so the hot
 * prefix of a class never straddles two lines and two classes never share
 * a line tail.
 */
#define AZ_CLASS_ALIGNMENT 64
#ifdef __cplusplus
	#define AZ_CLASS_ALIGN alignas(AZ_CLASS_ALIGNMENT)
#else
	#define AZ_CLASS_ALIGN _Alignas(AZ_CLASS_ALIGNMENT)
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
#define AZ_IMPL_IS_BOXED_VALUE(i) ((i) == &AZBoxedValueKlass.klass.impl)
#define AZ_IMPL_IS_BOXED_INTERFACE(i) ((i) == &AZBoxedInterfaceKlass.klass.impl)

/*
 * Value type (struct) requirements
 *
 * Value instances are copied with plain memcpy and compared with memcmp
 * (az_value_copy/az_value_transfer/az_value_equals) and are cleared without
 * running any destructor (az_value_clear). Therefore a value class:
 *
 * - has to be trivially copiable: the instance may not own references and may
 *   not contain internal pointers (pointers into itself or to owned memory);
 * - must not rely on a destructor: anything stored as a value (AZValue,
 *   AZPackedValue, container elements) never gets instance_finalize run on it.
 *
 * The requirements are inherited by interface implementations: a value class
 * may only implement interfaces whose instance data is itself trivially
 * copiable and destructor-free (the interface instance region is embedded in
 * the value instance and gets the same treatment). az_class_declare_interface
 * warns about interface finalizers; the trivially-copiable part cannot be
 * checked mechanically and is the class author's responsibility.
 */

#define AZ_CLASS_TYPE(c) ((c)->impl.type)
#define AZ_CLASS_FLAGS(c) ((c)->impl.flags)
/* These mirror AZ_TYPE_IS_*(AZ_CLASS_TYPE(c)) exactly */
#define AZ_CLASS_IS_ABSTRACT(c) ((c)->impl.flags & AZ_FLAG_ABSTRACT)
#define AZ_CLASS_IS_BLOCK(c) ((c)->impl.flags & AZ_FLAG_BLOCK)
#define AZ_CLASS_IS_VALUE(c) !((c)->impl.flags & AZ_FLAG_BLOCK)
#define AZ_CLASS_IS_REFERENCE(c) ((c)->impl.flags & AZ_FLAG_REFERENCE)
#define AZ_CLASS_IS_OBJECT(c) ((c)->impl.flags & AZ_FLAG_OBJECT)
#define AZ_CLASS_IS_BOXED_VALUE(c) ((c) == (const AZClass *) &AZBoxedValueKlass)
#define AZ_CLASS_IS_BOXED_INTERFACE(c) ((c) == (const AZClass *) &AZBoxedInterfaceKlass)
#define AZ_CLASS_IS_INTERFACE(c) ((c)->impl.flags & AZ_FLAG_INTERFACE)

#define AZ_CLASS_IS_FINAL(c) ((c)->impl.flags & AZ_FLAG_FINAL)
#define AZ_CLASS_VALUE_SIZE(c) (((c)->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : (c)->instance_size)
#define AZ_CLASS_ELEMENT_SIZE(c) (((c)->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : ((c)->instance_size + (c)->alignment) & ~(c)->alignment)
#define AZ_IMPL_VALUE_SIZE(i) (AZ_IMPL_IS_BLOCK(i) ? sizeof(void *) : ((AZClass *) (i))->instance_size)
#define AZ_IMPL_ELEMENT_SIZE(i) (AZ_IMPL_IS_BLOCK(i) ? sizeof(void *) : (((AZClass *) (i))->instance_size + ((AZClass *) (i))->alignment) & ~((AZClass *) (i))->alignment)

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

/*
 * The class allocator table. Custom allocators are rare, so classes hold a
 * uint8 index into this table (allocator_idx) instead of a pointer;
 * index 0 is the default (malloc/free) allocator.
 *
 * Allocators are registered during class construction (az_class_new), i.e.
 * under the registry lock, and are immutable afterwards; the entry is written
 * before the class is published, so lock-free class readers see a valid entry.
 */
#define AZ_MAX_CLASS_ALLOCATORS 256
extern const AZInstanceAllocator *az_class_allocators[AZ_MAX_CLASS_ALLOCATORS];
/**
 * @brief Register a class allocator
 *
 * @param allocator the allocator (has to stay valid for the program lifetime)
 * @return the allocator index for AZClass::allocator_idx (0 on error/exhaustion)
 */
unsigned int az_class_register_allocator (const AZInstanceAllocator *allocator);

/*
 * Class construction and circular type references
 *
 * Registering a type reserves its typecode immediately; the class itself is
 * constructed either eagerly (top-level registration, interfaces) or lazily on
 * first class access (registration nested inside another class construction).
 * Consequently, from a class constructor:
 *
 * - any type may be referenced by typecode (properties, method signatures),
 *   including types whose registration is still in progress
 * - any OTHER type's class may be accessed - it is constructed on demand
 *   (az_type_get_class), so e.g. interface implementation works for circular
 *   references regardless of registration order
 * - the class being constructed is NOT accessible by typecode until its
 *   construction finishes (self-references have to use the typecode)
 *
 * Genuine circular dependencies through extends/implements edges (A extends B
 * and B extends A, directly or transitively) are impossible by definition;
 * they are detected during construction and reported.
 */
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
	uint8_t n_ifaces_self;
	/**
	 * @brief the alignment mask: 0 (1), 1 (2), 3 (4), 7 (8), 15 (16), 31 (32), 63 (64) or 127 (128)
	 *
	 */
	uint8_t alignment;
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
	 * @brief Index into the class delegate table (0 = no delegation)
	 *
	 * Delegation is an opt-in, cold-path feature (e.g. AZReferenceOf forwards
	 * interface and property queries to the contained type); the delegate
	 * holds the rarely used virtuals so they do not grow the class.
	 */
	uint8_t delegate_idx;
	/**
	 * @brief Index into the class allocator table (0 = default malloc/free)
	 *
	 * Custom allocators are rare (a single variant for a class subtree at
	 * most), so they are kept in a side table instead of a per-class pointer.
	 */
	uint8_t allocator_idx;
	union {
		/**
		 * @brief The interface list if n_ifaces_all <= 2
		 *
		 */
		AZIFEntry ifaces[2];
		/**
		 * @brief The interface list if n_ifaces_all > 2
		 *
		 * All interfaces of this class: the declarations of this class first,
		 * then their transitive closures, then the parent class interfaces.
		 *
		 */
		AZIFEntry *ifaces_all;
	};

	AZField *props_self;

	/**
	 * @brief The size of the instance of this type
	 *
	 */
	uint32_t instance_size;
	/**
	 * @brief The size of the class structure
	 *
	 */
	uint16_t class_size;
	/**
	 * @brief Reserved for future use
	 *
	 */
	uint16_t _reserved;

	/* ---- End of the first (hot) 64 bytes ---- */

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

	/* Constructors and destructors */
	union {
		/**
		 * @brief The instance constructor
		 *
		 * Valid unless AZ_FLAG_HAS_DEFAULT is set (shares storage with default_value).
		 */
		void (*instance_init) (const AZImplementation *impl, void *inst);
		/**
		 * @brief The default value image (AZ_FLAG_HAS_DEFAULT)
		 *
		 * Construction copies instance_size bytes from here, replacing the
		 * constructor walk for this class and its ancestors (see the flag). Only
		 * concrete value types may carry a default value; set it in class_init.
		 */
		const void *default_value;
	};
	void (*instance_finalize) (const AZImplementation *impl, void *inst);

	/* Serialization is by instance */
	/* Return number of bytes that should have been written (regardless of dlen) */
	/* It is safe to set d to NULL */
	/* Returns the number of bytes that would have been written if there was enough room in destination */
	unsigned int (*serialize) (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
	/* Deserialization is by value, i.e. new instances of reference types should be created */
	/* Returns the number of bytes consumed (0 on error) */
	unsigned int (*deserialize) (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);

	/**
	 * @brief Convert instance to string
	 * @param impl an implementation
	 * @param instance an instance
	 * @param d the destination buffer (may be NULL, nothing is written then)
	 * @param dlen the destination buffer size
	 * @return the required length of the string (excluding the terminating '\0', may be bigger than dlen)
	 *
	 * Writes at most dlen bytes. If there is extra room (the required length is
	 * smaller than dlen) the string is terminated with '\0'.
	 */
	unsigned int (*to_string) (const AZImplementation* impl, void *instance, unsigned char *d, unsigned int dlen);

	/**
	 * @brief The name of this class for convenience (not used by the library)
	 *
	 */
	const uint8_t *name;
};

/*
 * Interface list storage
 *
 * The interface declarations of a class are always the first n_ifaces_self
 * entries of the full interface list, so a single array serves both:
 *
 *   n_ifaces_all <= 2 : the list is inline in klass->ifaces
 *   n_ifaces_all > 2  : the list is the heap array klass->ifaces_all
 *
 * A class with no own declarations shares the inherited list content (the
 * union is copied from the parent class and n_ifaces_all is inherited).
 *
 * Rejected declarations (e.g. circular interface implementation) keep their
 * slots in the list with type == 0 and are skipped during iteration.
 */

static inline const AZIFEntry *
az_class_ifaces_all(const AZClass *klass)
{
	return (klass->n_ifaces_all <= 2) ? klass->ifaces : klass->ifaces_all;
}

static inline const AZIFEntry *
az_class_ifaces_self(const AZClass *klass)
{
	/* The self declarations are the first n_ifaces_self entries of the full list */
	return az_class_ifaces_all(klass);
}

static inline const AZIFEntry *
az_class_iface_self(const AZClass *klass, uint16_t idx)
{
	return az_class_ifaces_self(klass) + idx;
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

static inline unsigned int
az_class_element_size(const AZClass *klass)
{
	return (az_class_value_size(klass) + klass->alignment) & ~klass->alignment;
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
 * @param klass current class (either the the class of the impl or a superclass)
 * @param impl type implementation (can be null for static properties)
 * @param inst type instance (can be null for static or implementation properties)
 * @param key the property key
 * @param def_class result: the actual class where the property is defined
 * @param sub_impl result: the actual implementation (either impl or sub-implementation, can be null)
 * @param sub_inst result: the actual instance (either inst or sub-interface, can be null)
 * @return the property index in def_class
 */
int az_class_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);
int az_class_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);

/**
 * @brief Print class registry statistics to stdout
 *
 * Prints the distribution of classes by category (struct/block/interface/
 * reference/object), flags, interface list sizes, property counts, class and
 * instance sizes and alignments. Intended for evaluating class layout and
 * access-pattern optimizations.
 *
 * Lazily registered types whose classes have not been constructed yet are
 * counted separately and never constructed by this function.
 */
void az_classes_print_stats (void);

#ifdef __cplusplus
};
#endif

#endif
