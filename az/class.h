#ifndef __AZ_CLASS_H__
#define __AZ_CLASS_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#include <az/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implementation flags */
#define AZ_FLAG_IMPL_IS_CLASS 0x01

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

#define AZ_IMPL_IS_CLASS(i) ((i)->flags & AZ_FLAG_IMPL_IS_CLASS)
#define AZ_CLASS_FROM_IMPL(i) (AZ_IMPL_IS_CLASS(i) ? (AZClass *) (i) : (i)->klass)

#define AZ_IMPL_TYPE(i) (AZ_IMPL_IS_CLASS(i) ? (i)->type : (i)->klass->impl.type)
#define AZ_IMPL_FLAGS(i) (AZ_IMPL_IS_CLASS(i) ? (i)->flags : (i)->klass->impl.flags)
#define AZ_CLASS_TYPE(c) ((c)->impl.type)
#define AZ_CLASS_FLAGS(c) ((c)->impl.flags)

#define AZ_IMPL_IS_VALUE(i) (((i)->flags & (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_VALUE)) == (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_VALUE))
#define AZ_IMPL_IS_BLOCK(i) (((i)->flags & (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_BLOCK)) == (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_BLOCK))
#define AZ_IMPL_IS_REFERENCE(i) (((i)->flags & (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_REFERENCE)) == (AZ_FLAG_IMPL_IS_CLASS | AZ_FLAG_REFERENCE))

#define AZ_CLASS_IS_VALUE(c) !((c)->impl.flags & AZ_FLAG_BLOCK)
/* Type flags */
/* Interface type, implementation is not class */
#define AZ_FLAG_INTERFACE 0x01000000
/* Instance is referenced by pointer */
#define AZ_FLAG_BLOCK 0x02000000
/* Reference type, copying needs reference counting */
#define AZ_FLAG_REFERENCE 0x04000000
/* Instance is a container of another type */
#define AZ_FLAG_BOXED 0x08000000

/* No instancing is allowed (this is not propagated to subclasses) */
#define AZ_FLAG_ABSTRACT 0x10000000
/* No subclasses */
#define AZ_FLAG_FINAL 0x20000000
/* Subclasses should not remove flag set by parent */
#define AZ_FLAG_ZERO_MEMORY 0x40000000
#define AZ_FLAG_CONSTRUCT 0x80000000

// fixme: remove
#define AZ_FLAG_VALUE 0

#define AZ_CLASS_ELEMENT_SIZE(klass) ((klass->instance_size + klass->alignment) & ~klass->alignment)

typedef struct _AZInstanceAllocator AZInstanceAllocator;

struct _AZInstanceAllocator {
	void *(*allocate) (AZClass *klass);
	void *(*allocate_array) (AZClass *klass, unsigned int nelements);
	void (*free) (AZClass *klass, void *location);
	void (*free_array) (AZClass *klass, void *location, unsigned int nelements);
};

struct _AZClass {
	AZImplementation impl;
	//unsigned int flags;

	AZClass *parent;

	uint16_t n_ifaces_self;
	uint16_t n_ifaces_all;
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
	unsigned int n_properties_self;
	AZField *properties_self;
#endif

	const uint8_t *name;
	/* Alignment mask: 0 (1), 1 (2), 3 (4), 7 (8) or 15 (16) */
	uint16_t alignment;
	/* Size of class structure */
	uint16_t class_size;
	/* Size of instance */
	uint32_t instance_size;

	/* Default value */
	void *default_val;

	/* Memory management */
	AZInstanceAllocator *allocator;

	/*
	* This allows superclasses to adjust values, that depend on the actual type and are not
	* handled by pre_init and post_init
	* 
	* Called from post_init
	* 
	*/
	void (*init_recursive) (AZClass *klass);
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

static inline AZClass *
az_class_parent(const AZClass *klass) {
	return klass->parent;
}

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
//	if (klass->n_ifaces_self == klass->n_ifaces_all) {
//		return (klass->n_ifaces_self <= 2) ? &klass->ifaces[idx] : &klass->ifaces_self[idx];
//	} else {
//		return (klass->n_ifaces_self <= 1) ? &klass->ifaces[idx] : &klass->ifaces_self[idx];
//	}
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
//	if (klass->n_ifaces_self == klass->n_ifaces_all) {
//		return (klass->n_ifaces_self <= 2) ? &klass->ifaces[idx] : &klass->ifaces_all[idx];
//	} else {
//		if (klass->n_ifaces_self <= 1) {
//			return (klass->n_ifaces_all <= 2) ? &klass->ifaces[idx] : &klass->ifaces_all[idx];
//		} else {
//			return &klass->ifaces_all[idx];
//		}
//	}
}

static inline unsigned int
az_class_value_size(const AZClass *klass)
{
	return (klass->impl.flags & AZ_FLAG_BLOCK) ? sizeof(void *) : klass->instance_size;
}

/* To be called from class constructors */
void az_class_set_num_interfaces (AZClass *klass, unsigned int ninterfaces);
void az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset);

#ifdef AZ_HAS_PROPERTIES
void az_class_set_num_properties (AZClass *klass, unsigned int nproperties);
void az_class_define_property (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst);
void az_class_define_property_function_val (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, const AZImplementation *impl, void *inst);
void az_class_define_property_function_packed (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig);
void az_class_property_setup (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_static, unsigned int can_read, unsigned int can_write, unsigned int is_final, unsigned int is_value,
	unsigned int value_type, void *inst);

void az_class_define_method (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *));
void az_class_define_method_va (AZClass *klass, unsigned int idx, const unsigned char* key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...);
void az_class_define_static_method (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *));
void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...);
#endif

#ifdef __cplusplus
};
#endif

#endif
