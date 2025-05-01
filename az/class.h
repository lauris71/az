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

/**
 * @brief Superclass of all implementations
 * 
 */
struct _AZImplementation {
	unsigned int type;
};

/* Class flags */

/* No instancing is allowed (this is not propagated to subclasses) */
#define AZ_CLASS_IS_ABSTRACT 1
/* No subclasses */
#define AZ_CLASS_IS_FINAL 2
/* Instance is value */
#define AZ_CLASS_IS_VALUE 4
/* Subclasses should not remove flag set by parent */
#define AZ_CLASS_ZERO_MEMORY 8
/* Special handling */
#define AZ_CLASS_IS_BLOCK 16
#define AZ_CLASS_IS_REFERENCE 32
#define AZ_CLASS_IS_INTERFACE 64

typedef struct _AZInstanceAllocator AZInstanceAllocator;

struct _AZInstanceAllocator {
	void *(*allocate) (AZClass *klass);
	void *(*allocate_array) (AZClass *klass, unsigned int nelements);
	void (*free) (AZClass *klass, void *location);
	void (*free_array) (AZClass *klass, void *location, unsigned int nelements);
};

struct _AZClass {
	AZImplementation implementation;

	unsigned int flags;

	AZClass *parent;

	uint16_t n_ifaces_self;
	uint16_t n_ifaces_all;
	/* Interfaces implemented here */
	unsigned int ifaces_self;
	/* Interface chain (ascending, interface then sub-interfaces */
	unsigned int ifaces_all;

#ifdef AZ_HAS_PROPERTIES
	unsigned int n_properties_self;
	AZField *properties_self;
#endif

	const uint8_t *name;
	/* Has to be 0 (for zero-sized types), 1, 2, 4, 8 or 16 */
	unsigned int alignment;
	/* Size of class structure */
	unsigned int class_size;
	/* Size of instance */
	unsigned int instance_size;
	/* Size of value */
	unsigned int value_size;
	/* Size of values in arrays (rounded to 16 bytes for aligned types) */
	unsigned int element_size;

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

/* To be called from class constructors */
void az_class_set_num_interfaces (AZClass *klass, unsigned int ninterfaces);
void az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset);

#ifdef AZ_HAS_PROPERTIES
void az_class_set_num_properties (AZClass *klass, unsigned int nproperties);
void az_class_define_property (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst);
void az_class_define_property_function (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZFunctionSignature *sig, const AZImplementation *impl, void *inst);
void az_class_define_property_from_def (AZClass *klass, AZFieldDefinition *def);
void az_class_define_properties (AZClass *klass, AZFieldDefinition *defs, unsigned int num_defs);
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
