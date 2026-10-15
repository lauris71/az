#ifndef __AZ_EXTEND_H__
#define __AZ_EXTEND_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <az/class.h>
#include <az/field.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structs and functions needed only for extending types (e.g. normally used in class_init)
 *
 * These are collected int separate header to reduce the clutter of class.h
 */

/** @ingroup types
 * @brief Get new typecode, allocate and initialize a class structure
 * 
 * Gets a new typecode, initializes its class to the minimal functional state and then calls class_init,
 * where a more specialized setup (e.g. registering interfaces and fields) should be done. The final setup
 * is performed only after class_init returns (and thus the interfaces and fields are known).
 * 
 * @param type pointer where the typecode will be written
 * @param name the name of the new type
 * @param parent_type the parent typecode
 * @param class_size the size of the class
 * @param instance_size the size of the instance
 * @param flags type flags
 * @param n_interfaces_self number of interfaces implemented by this type
 * @param n_properties_self number of properties defined for this type
 * @param class_init the class constructor
 * @param instance_init the instance constructor
 * @param instance_finalize the instance destructor
 * @return allocated and initialized class
 */

AZClass *az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));

/** @ingroup types
 * @brief Get new typecode, allocate and initialize a class structure
 * 
 * Gets a new typecode, initializes its class to the minimal functional state and then calls class_init,
 * where a more specialized setup (e.g. registering interfaces and fields) should be done. The final setup
 * is performed only after class_init returns (and thus the interfaces and fields are known).
 * Unlike az_register_type this method allows passing extra parameter to the class_init method.
 * It is used to build composite classes (e.g. reference of another type). 
 * 
 * @param type pointer where the typecode will be written
 * @param name the name of the new type
 * @param parent_type the parent typecode
 * @param class_size the size of the class
 * @param instance_size  the size of the instance
 * @param flags type flags
 * @param n_interfaces_self number of interfaces implemented by this type
 * @param n_properties_self number of properties defined for this type
 * @param class_init the class constructor
 * @param instance_init a the instance constructor
 * @param instance_finalize the instance destructor
 * @param data extra parameter to the class constructor
 * @return allocated and initialized class
 */
AZClass *az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data);

	struct _AZInstanceAllocator {
	/*
	 * Allocate an instance of the given size in bytes (the class's instance_size
	 * for fixed-size classes; possibly bigger for AZ_FLAG_VARIABLE_SIZE ones).
	 * The allocator decides internally between fixed-block pooling and generic
	 * allocation based on the requested size.
	 */
	void *(*allocate) (AZClass *klass, unsigned int size);
	void *(*allocate_array) (AZClass *klass, unsigned int n_elements);
	/*
	 * Free an instance. For variable-size classes the instance must retain
	 * enough information to recompute its allocation size after finalization
	 * (e.g. the boxed interface keeps the container implementation), so the
	 * allocator can tell pool memory and generic memory apart.
	 */
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
extern const AZInstanceAllocator *az_allocators[AZ_MAX_CLASS_ALLOCATORS];
/**
 * @brief Register a class allocator
 *
 * @param allocator the allocator (has to stay valid for the program lifetime)
 * @return the allocator index for AZClass::allocator_idx (0 on error/exhaustion)
 */
unsigned int az_register_allocator (const AZInstanceAllocator *allocator);

/*
 * Class delegates
 *
 * A delegate lets a class answer interface/property queries and structural
 * enumeration on behalf of a contained or wrapped value (e.g. AZBoxedValue,
 * AZBoxedInterface, AZReferenceOf). Classes with a delegate have delegate_idx
 * != 0 (the class's first cache line); the lookup/iteration entry points try
 * the delegate first and fall back to the class's own tables.
 *
 * The delegate methods receive the delegating class, the impl and inst the
 * query was issued with (inst may be NULL for type-level queries, e.g. from
 * az_type_implements - the delegate has to pass NULL through, not compute a
 * content instance).
 *
 * Rules:
 * - A delegate must delegate to a different class; the built-in delegates only
 *   target value/interface content, which never carries a delegate itself.
 * - Dispatch happens once at the public entry points; the default workers
 *   never re-dispatch, so recursion into interfaces and parent classes does
 *   not re-enter delegation.
 * - lookup slots return "not found" (NULL/NU_SUCH_PROPERTY) to fall back to the class's own
 *   members; the foreach slots, if present, perform the full enumeration.
 *
 * Delegates are registered during class construction (i.e. under the registry
 * lock) and are immutable afterwards; the table entry is written before the
 * class is published, so lock-free class readers see a valid entry.
 */
typedef struct _AZClassDelegate AZClassDelegate;

struct _AZClassDelegate {
	const AZImplementation *(*get_interface) (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst);
	int (*lookup_property) (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);
	int (*lookup_function) (const AZClass *klass, const AZImplementation *impl, void *inst, const AZString *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);
	int (*foreach_property) (const AZClass *klass, const AZImplementation *impl, void *inst, AZPropertyForeachFunc cb, void *data);
	int (*foreach_interface) (const AZClass *klass, const AZImplementation *impl, void *inst, AZInterfaceForeachFunc cb, void *data);
};

#define AZ_MAX_CLASS_DELEGATES 256
extern const AZClassDelegate *az_delegates[AZ_MAX_CLASS_DELEGATES];
/**
 * @brief Register a class delegate
 *
 * @param delegate the delegate (has to stay valid for the program lifetime)
 * @return the delegate index for AZClass::delegate_idx (0 on error/exhaustion)
 */
unsigned int az_register_delegate (const AZClassDelegate *delegate);

/* To be called from class constructors */
void az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset);

/* Define property with AZ_FIELD_READ_VALUE */
AZField *az_class_define_property_value (AZClass *klass, unsigned int idx, const uint8_t *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int write, unsigned int offset);
/* Define property with AZ_FIELD_READ_PACKED */
void az_class_define_property_packed (AZClass *klass, unsigned int idx, const uint8_t *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int write, unsigned int offset);
void az_class_define_property (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst);
void az_class_define_property_function_val (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, const AZImplementation *impl, void *inst);
void az_class_define_property_function_packed (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig);

void az_class_define_method (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *));
void az_class_define_method_va (AZClass *klass, unsigned int idx, const unsigned char* key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...);
void az_class_define_static_method (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *));
void az_class_define_static_method_native (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	void (*invoke) (void));
void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...);
void az_class_define_static_method_native_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	void (*invoke) (void),
	unsigned int ret_type, unsigned int n_args, ...);

#ifdef __cplusplus
};
#endif

#endif
