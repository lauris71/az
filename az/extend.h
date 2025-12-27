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
 * Functions for extending types
 */

/** @ingroup types
 * @brief Get new typecode, allocate and initialize a class structure
 * 
 * Gets a new typecode, initializes its class to the minimal functional state and then calls class_init,
 * where a more specialized setup (e.g. registering interfaces and fields) should be done. The final setup
 * is performed only after class_init returns (and thus the interfaces and fields are known).
 * 
 * @param type pointer where the typecode will be written
 * @param name the name of the new class
 * @param parent_type the parent typecode
 * @param class_size the size of the class
 * @param instance_size  the size of the instance
 * @param flags type flags
 * @param class_init a callback for initialzaton of the class
 * @param instance_init a callback for the intialization of an instance
 * @param instance_finalize a callback for the finalization of an instance
 * @return allocated and initialized class
 */

AZClass *az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
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
 * @param name the name of the new class
 * @param parent_type the parent typecode
 * @param class_size the size of the class
 * @param instance_size  the size of the instance
 * @param flags type flags
 * @param class_init a callback for initialzaton of the class
 * @param instance_init a callback for the intialization of an instance
 * @param instance_finalize a callback for the finalization of an instance
 * @param data extra parameter to the class_init function
 * @return allocated and initialized class
 */
AZClass *az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data);

/* To be called from class constructors */
void az_class_set_num_interfaces (AZClass *klass, unsigned int ninterfaces);
void az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset);

#ifdef AZ_HAS_PROPERTIES
void az_class_set_num_properties (AZClass *klass, unsigned int nproperties);
/* Define property with AZ_FIELD_READ_VALUE */
void az_class_define_property_value (AZClass *klass, unsigned int idx, const uint8_t *key, unsigned int type,
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
void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...);
#endif

#ifdef __cplusplus
};
#endif

#endif
