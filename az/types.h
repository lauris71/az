#ifndef __AZ_TYPES_H__
#define __AZ_TYPES_H__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdint.h>
#include <stdlib.h>

#include <arikkei/arikkei-utils.h>

#include <az/az.h>
#include <az/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup types AZ types
 *  The basic type access
 */

#define AZ_TYPE_MASK 0x00ffffff
#define AZ_TYPE_INDEX(t) ((t) & AZ_TYPE_MASK)

typedef struct _AZTypeInfo AZTypeInfo;

struct _AZTypeInfo {
	uint32_t flags;
	/* Parent INDEX */
	uint32_t pidx;
	AZClass *klass;
};

/*
 * Three variants of handling global type arrays:
 * AZ_GLOBALS_FIXED_SIZE - use compile-time fixed size arrays (AZ_MAX_TYPES)
 * AZ_GLOBALS_SINGLE_THREAD - completely ignore concurrency 
 * AZ_GLOBALS_MULTI_THREAD - use mutex
 * 
 * The following methods/macros are redefined depending on globals handling:
 * az_init()
 * az_reserve_type()
 * az_type_get_class()
 * az_type_get_info();
 * AZ_CLASS_FROM_TYPE
 * AZ_IMPL_FROM_TYPE
 * AZ_TYPE_FLAGS
 */

#define AZ_GLOBALS_FIXED_SIZE
#define AZ_MAX_TYPES 256

/*
 * Basic type queries
 */

#if defined AZ_GLOBALS_FIXED_SIZE

#ifndef __AZ_TYPES_C__
extern AZTypeInfo az_types[];
extern unsigned int az_num_types;
#else
AZTypeInfo az_types[AZ_MAX_TYPES];
unsigned int az_num_types = 0;
#endif
/* No safety checking */
#define AZ_INFO_FROM_TYPE(t) &az_types[AZ_TYPE_INDEX(t)]
#define AZ_CLASS_FROM_TYPE(t) az_types[AZ_TYPE_INDEX(t)].klass
#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_types[AZ_TYPE_INDEX(t)].klass)
#else

/* C array of all defined classes */
#ifndef __AZ_TYPES_C__
extern AZTypeInfo *az_types;
extern unsigned int az_num_types;
#else
AZTypeInfo *az_types = NULL;
unsigned int az_num_types = 0;
#endif
/* No safety checking */
#define AZ_INFO_FROM_TYPE(t) &az_types[AZ_TYPE_INDEX(t)]
#define AZ_CLASS_FROM_TYPE(t) az_types[AZ_TYPE_INDEX(t)].klass
#define AZ_IMPL_FROM_TYPE(t) ((AZImplementation *) az_types[AZ_TYPE_INDEX(t)].klass)

#endif

#define AZ_TYPE_FLAGS(t) az_types[AZ_TYPE_INDEX(t)].flags
#define AZ_TYPE_IS_BLOCK(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_VALUE(t) !(AZ_TYPE_FLAGS(t) & AZ_FLAG_BLOCK)
#define AZ_TYPE_IS_REFERENCE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_REFERENCE)
#define AZ_TYPE_IS_INTERFACE(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_INTERFACE)

#define AZ_TYPE_IS_FINAL(t) (AZ_TYPE_FLAGS(t) & AZ_FLAG_FINAL)

#ifdef AZ_SAFETY_CHECKS
static inline AZTypeInfo *
az_type_get_info (unsigned int type)
{
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, NULL);
	return AZ_INFO_FROM_TYPE(type);
}
static inline AZClass *
az_type_get_class (unsigned int type)
{
	if (!az_num_types) az_init ();
	arikkei_return_val_if_fail (type < az_num_types, NULL);
	return AZ_CLASS_FROM_TYPE(type);
}
#else
#define az_type_get_info AZ_INFO_FROM_TYPE
#define az_type_get_class AZ_CLASS_FROM_TYPE
#endif

#define az_type_get_impl(t) ((AZImplementation *) az_type_get_class(t))

/** @ingroup types
 * @brief Get parent primitive type
 * 
 * @param type the query type
 * @return the parent primitive (AZ_TYPE_BOOLEAN - AZ_TYPE_BLOCK) or AZ_TYPE_NONE for invalid type 
 */
unsigned int az_type_get_parent_primitive (unsigned int type);

/** @ingroup types
 * @brief Checks whether the given type is a subtype of another
 * 
 * @param type the type that is checked
 * @param test the type tested against
 * @return 1 if type is a subtype of test, 0 if not or if either type is invalid
 */
unsigned int az_type_is_a (unsigned int type, unsigned int test);

/** @ingroup types
 * @brief Checks whether the given type implements an interface type
 * 
 * @param type the type that is checked
 * @param test the interface type tested against
 * @return 1 if type implements test, 0 if not or if either type is invalid
 */
unsigned int az_type_implements (unsigned int type, unsigned int test);
/** @ingroup types
 * @brief Checks whether a value of certain type can be assigned to a variable of given type
 * 
 * True if:
 * - type is test
 * - type implements test
 * - type is NONE and test is ANY or BLOCK
 * 
 * @param type the type that is checked
 * @param test the type of variable to be tested against
 * @return 1 if type can be assigned, 0 if not or if either type is invalid
 */
unsigned int az_type_is_assignable_to (unsigned int type, unsigned int test);

/* Basic constructor frontends */
/**
 * @brief Initialize a new instance
 * 
 * Initializes a new instance by calling recursively all superclass and interface constructors
 * @param inst pointer to instance
 * @param type instance type
 */
#ifdef AZ_SAFETY_CHECKS
#define az_instance_init_by_type(i,t) az_instance_init(AZ_IMPL_FROM_TYPE(t), i)
#define az_interface_init(impl,inst) az_instance_init(impl, inst)
#define az_instance_finalize_by_type(i,t) az_instance_finalize(AZ_IMPL_FROM_TYPE(t), i)
#define az_interface_finalize(impl,inst) az_instance_finalize(impl, inst)
#else
static void
az_instance_init_by_type (void *inst, unsigned int type)
{
	if (AZ_TYPE_FLAGS(type) & (AZ_FLAG_ZERO_MEMORY | AZ_FLAG_CONSTRUCT)) az_instance_initialize(AZ_IMPL_FROM_TYPE(type), inst);
}
static void
az_interface_init (const AZImplementation *impl, void *inst)
{
	if (AZ_TYPE_FLAGS(type) & (AZ_FLAG_ZERO_MEMORY | AZ_FLAG_CONSTRUCT)) az_instance_initialize(impl, inst);
}
static void
az_instance_finalize_by_type (void *inst, unsigned int type)
{
	if (AZ_TYPE_FLAGS(type) & AZ_FLAG_CONSTRUCT) az_instance_finalize(AZ_IMPL_FROM_TYPE(type), inst);
}
static void
az_interface_finalize (const AZImplementation *impl, void *inst)
{
	if (AZ_TYPE_FLAGS(type) & AZ_FLAG_CONSTRUCT) az_instance_finalize(impl, inst);
}
#endif

/* Get rootmost interface */
const AZImplementation *az_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst);
static inline const AZImplementation *
az_get_interface_from_type (unsigned int type, void *inst, unsigned int if_type, void **if_inst)
{
	return az_get_interface (AZ_IMPL_FROM_TYPE(type), inst, if_type, if_inst);
}

unsigned int az_deserialize_value (const AZImplementation *impl, AZValue *value, const unsigned char *s, unsigned int slen, AZContext *ctx);

#ifdef AZ_HAS_PROPERTIES

unsigned int az_instance_set_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
unsigned int az_instance_get_property (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val);
unsigned int az_instance_get_function (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val);

unsigned int az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx);
unsigned int az_instance_get_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation **prop_impl, AZValue64 *prop_val, unsigned int val_size, AZContext *ctx);
#endif

#ifdef __cplusplus
};
#endif

#endif
