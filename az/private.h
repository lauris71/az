#ifndef __AZ_PRIVATE_H__
#define __AZ_PRIVATE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

/* Turn on internal/optimized/unchecked variants */
#define AZ_INTERNAL

#include <az/types.h>
#include <az/class.h>

#define AZ_TYPE_VALUE_SIZE(t) az_class_value_size(az_type_get_class(t))

#if defined(AZ_GLOBALS_STATIC) || defined(AZ_GLOBALS_SINGLE_THREAD)
	/* Slow path for deferred (reserved but not yet constructed) types */
	unsigned int az_type_is_valid_deferred (uint32_t type);
	static inline unsigned int
	az_type_is_valid(uint32_t type)
	{
		if (AZ_TYPE_INDEX(type) == 0) return 0;
		if (AZ_TYPE_INDEX(type) >= az_num_types) return 0;
		/* The slot holds either the class, or a tagged construction descriptor */
#if defined(AZ_GLOBALS_STATIC)
		uintptr_t p = atomic_load_explicit (&az_types[AZ_TYPE_INDEX(type)], memory_order_acquire);
#else
		uintptr_t p = az_types[AZ_TYPE_INDEX(type)];
#endif
		if (!p) return 0;
		if (p & 1) return az_type_is_valid_deferred (type);
		return type == ((AZClass *) p)->impl.type;
	}
	#define ENSURE_INITIALIZED() if (!az_num_types) az_init()
#elif defined(AZ_GLOBALS_MULTI_THREAD)
	unsigned int az_type_is_valid(uint32_t type);
	/* fixme: Think if we can do multi-threaded initialization */
	#define ENSURE_INITIALIZED()
#endif

#ifdef AZ_SAFETY_CHECKS
#define AZ_CHECK_TYPE(t) arikkei_return_if_fail(az_type_is_valid(t))
#define AZ_CHECK_TYPE_RET(t,v) arikkei_return_val_if_fail(az_type_is_valid(t), v)
#else
#define AZ_CHECK_TYPE(t)
#define AZ_CHECK_TYPE_RET(t,v)
#endif

/* Library internals */
void az_globals_init (void);

/**
 * @brief Publish a fully constructed class
 *
 * Writes the class to its az_types slot with release semantics (where applicable),
 * replacing the construction descriptor if the registration was deferred. After
 * this any thread may access the class via the AZ_CLASS_FROM_TYPE fast path.
 *
 * @param klass A class to publish
 */
void az_class_publish (AZClass *klass);

/* Registration data for a reserved but not yet constructed class (private.c) */
typedef struct _AZTypeDescriptor AZTypeDescriptor;

/**
 * @brief Registers a type in the type system
 *
 * Reserves the typecode immediately (the slot gets a construction descriptor), so
 * the typecode is always valid from return. The class itself is constructed
 * immediately if force_construct is set or the call is not nested inside another
 * class construction; otherwise construction is deferred until the first class
 * access (az_type_get_class).
 *
 * @return The constructed class, or NULL if construction was deferred or failed
 */
AZClass *az_type_register_internal (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	unsigned int n_interfaces_self, unsigned int n_properties_self,
	void (*class_init) (AZClass *), void (*class_init_ex) (AZClass *, void *), void *data,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	unsigned int implementation_size, void (*implementation_init) (AZImplementation *),
	unsigned int force_construct);

/* Library internals */
void az_init_primitive_classes (void);
void az_post_init_primitive_classes (void);

void az_init_base_classes (void);
void az_post_init_base_classes (void);
void az_impl_class_post_init (void);
void az_class_class_post_init (void);

void az_init_interface_class (void);
void az_init_field_class (void);
void az_init_function_classes (void);
void az_init_reference_class (void);
void az_init_string_class (void);
void az_init_boxed_value_class (void);
void az_init_boxed_interface_class (void);
void az_init_packed_value_class (void);
void az_init_object_class(void);
void az_init_output_stream_class(void);
void az_init_input_stream_class(void);

/* Allocates and initializes a new class; does NOT register it, call neither class constructor nor post_init */
/* The parent class is constructed on demand; the typecode is assigned by the caller (az_type_construct) */
AZClass *az_class_new (const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));
/* Used internally for fundamental types: publishes the statically initialized class */
void az_class_new_with_value (AZClass *klass);

/* Called after class constructor has run (builds interface chain etc.) */
void az_class_post_init (AZClass *klass);

/* Constrained type */
typedef struct _AZTypeConstraint AZTypeConstraint;

struct _AZTypeConstraint {
	uint32_t is_a;
	uint32_t implements_a;
};

#ifdef __cplusplus
}
#endif

#endif /* PRIVATE_H */

