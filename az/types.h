#ifndef __AZ_TYPES_H__
#define __AZ_TYPES_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/


#include <stdint.h>
#include <stdlib.h>

#include <az/az.h>


#ifdef __cplusplus
extern "C" {
#endif

#define AZ_TYPE_MASK 0x00ffffff
#define AZ_TYPE_INDEX(t) ((t) & AZ_TYPE_MASK)

typedef struct _AZTypeInfo AZTypeInfo;

struct _AZTypeInfo {
	uint32_t flags;
	/* Parent INDEX */
	uint32_t parent;
};

/* C array of all defined classes */

#ifndef __AZ_TYPES_C__
extern AZClass **az_classes;
extern AZTypeInfo *az_types;
extern unsigned int az_num_classes;
#else
AZClass **az_classes = NULL;
AZTypeInfo *az_types = NULL;
unsigned int az_num_classes = 0;
#endif

/*
 * Basic type queries
 */

/* No safety checking */
#define AZ_CLASS_FROM_TYPE(t) az_classes[AZ_TYPE_INDEX(t)]
#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_classes[AZ_TYPE_INDEX(t)])

#define AZ_TYPE_FLAGS(t) az_types[AZ_TYPE_INDEX(t)].flags
#define AZ_TYPE_IS_REFERENCE(t) (AZ_TYPE_FLAGS(t) & AZ_CLASS_IS_REFERENCE)
#define AZ_TYPE_IS_INTERFACE(t) (AZ_TYPE_FLAGS(t) & AZ_CLASS_IS_INTERFACE)
#define AZ_TYPE_IS_BLOCK(t) (AZ_TYPE_FLAGS(t) & AZ_CLASS_IS_BLOCK)
#define AZ_TYPE_IS_VALUE(t) (AZ_TYPE_FLAGS(t) & AZ_CLASS_IS_VALUE)
#define AZ_TYPE_IS_FINAL(t) (AZ_TYPE_FLAGS(t) & AZ_CLASS_IS_FINAL)

#ifdef AZ_SAFETY_CHECKS
ARIKKEI_INLINE AZClass *
az_type_get_class (unsigned int type)
{
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail (type < az_num_classes, NULL);
	return az_classes[AZ_TYPE_INDEX(type)];
}
#define az_type_get_impl(t) ((AZImplementation *) az_type_get_class(t))
#else
#define az_type_get_class AZ_CLASS_FROM_TYPE
#define az_type_get_impl AZ_IMPL_FROM_TYPE
#endif

unsigned int az_type_get_parent_primitive (unsigned int type);

unsigned int az_type_is_a (unsigned int type, unsigned int test);
unsigned int az_type_implements (unsigned int type, unsigned int test);
/**
 * @brief tests whether type can be assigned to a variable
 * 
 * True if:
 *   type is test
 *   type implements test
 *   type is NONE and test is ANY/BLOCK
 * 
 * @param type the query
 * @param test the type to be tested against
 * @return true if type can be assigned
 */
unsigned int az_type_is_assignable_to (unsigned int type, unsigned int test);
/**
 * @brief tests whether type can automatically be converted
 * 
 * True if:
 *   type is test
 *   both type and test are arithmetic and have automatic conversion rule
 * 
 * @param type the query
 * @param test the type to be tested against
 * @return true if type can be converted
 */
unsigned int az_type_is_convertible_to (unsigned int type, unsigned int test);

/* Basic constructor frontends */
void az_instance_init (void *inst, unsigned int type);
void az_instance_finalize (void *inst, unsigned int type);
/* Initialize a new implementation for interface */
void az_implementation_init (AZImplementation *impl, unsigned int type);
/* These are needed for unregistered interfaces */
void az_interface_init (const AZImplementation *impl, void *inst);
void az_interface_finalize (const AZImplementation *impl, void *inst);

void *az_instance_new (unsigned int type);
void *az_instance_new_array (unsigned int type, unsigned int nelements);
void az_instance_delete (unsigned int type, void *inst);
void az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements);

/* Get rootmost interface */
const AZImplementation *az_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst);
ARIKKEI_INLINE const AZImplementation *
az_get_interface_from_type (unsigned int type, void *inst, unsigned int if_type, void **if_inst)
{
	return az_get_interface (AZ_IMPL_FROM_TYPE(type), inst, if_type, if_inst);
}

unsigned int az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx);
unsigned int az_deserialize_value (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);
/* If len > 0 writes the terminating 0 */
unsigned int az_instance_to_string (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen);

/*
 * Frontend to az_register_class
 * Get new typecode, allocate and initialize a class structure
 * Return registered class
 */

AZClass *az_register_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *));

/*
* Pass arguments to class_init (for composite types)
*/

AZClass *az_register_composite_type (unsigned int *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*class_init) (AZClass *, void *),
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *),
	void *data);

#ifdef AZ_HAS_PROPERTIES
/*
 * Get index of topmost property with given name and handling class, implementation and instance, return -1 if not found
 * The order is class->interface[0]->superinterfaces->interface[1]...->superclasses
 */
int az_lookup_property (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop);
int az_lookup_function (const AZClass *klass, const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **def_impl, void **def_inst, AZField **prop);

unsigned int az_instance_set_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
unsigned int az_instance_get_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val);
unsigned int az_instance_get_function (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val);

unsigned int az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
unsigned int az_instance_get_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation **prop_impl, AZValue64 *prop_val, AZContext *ctx);
#endif

#ifdef __cplusplus
};
#endif

#endif
