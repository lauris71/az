#ifndef __AZ_PROPERTIES_H__
#define __AZ_PROPERTIES_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2026
*/

#include <stdint.h>

#include <az/class.h>
#include <az/value.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Property access
 *
 * Properties are class members, but access goes through an implementation and
 * optionally an instance - the class is always derived from the implementation
 * (AZ_CLASS_FROM_IMPL), and an implementation is either the class itself or an
 * interface implementation whose class is known. A class-level query therefore
 * simply passes the class implementation (&klass->impl) with a NULL instance.
 *
 * This is the common user-level API: looking up properties and reading,
 * writing and enumerating them. The property specification used during class
 * construction lives in az/field.h.
 */

typedef int (*AZPropertyForeachFunc) (const AZClass *def_class, unsigned int prop_idx, const AZImplementation *impl, void *inst, void *data);
typedef int (*AZInterfaceForeachFunc) (const AZImplementation *iface_impl, void *iface_inst, unsigned int if_type, unsigned int level, void *data);

/**
 * @brief get the index, the containing class and corresponding implementation and instance of a property
 * 
 * Searches class and interface definitions recursively for a property.
 * The order is class->interface[0]->superinterfaces->interface[1]...->superclasses
 * If the index is >= 0, the property definition is def_class->props_self[index] 
 * 
 * The class is derived from the implementation (AZ_CLASS_FROM_IMPL): a class
 * implementation searches its own class, an interface implementation searches
 * the interface class. Class-level queries pass the class implementation
 * (&klass->impl) with a NULL instance.
 *
 * @param impl type implementation (the class implementation for class-level queries)
 * @param inst type instance (can be null for class/implementation level queries)
 * @param key the property key
 * @param def_class result: the actual class where the property is defined
 * @param sub_impl result: the actual implementation (either impl or sub-implementation, can be null)
 * @param sub_inst result: the actual instance (either inst or sub-interface, can be null)
 * @return the property index in def_class or error code
 */
int az_lookup_property (const AZImplementation *impl, void *inst, const AZString *key, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);
int az_lookup_function (const AZImplementation *impl, void *inst, const AZString *key, const AZFunctionSignature *sig, const AZClass **def_class, const AZImplementation **sub_impl, void **sub_inst);

/**
 * @brief Iterate over all visible properties of an instance or type
 *
 * Enumerates the properties in lookup order: own properties, then interface
 * properties (recursively), then the parent class. The callback receives the
 * defining class and the index within it (the property identity - there is no
 * flat props_all), plus the impl/inst location the property is accessed
 * through (the base instance or one of the interface views; inst locations are
 * NULL for type-level calls). A property belongs to an interface iff its
 * impl/inst matches that interface view from az_foreach_interface.
 *
 * Duplicates are not suppressed: a property reachable by multiple paths is
 * reported once per path, with the respective location.
 *
 * @param impl the implementation (and class) to enumerate
 * @param inst the instance (may be NULL for type-level enumeration)
 * @param cb the callback (AZ_OK to continue, positive to stop, negative error)
 * @param data user data passed to the callback
 * @return AZ_OK if the enumeration ran to completion, the callback's positive stop token, or the negative error code
 */
int az_foreach_property (const AZImplementation *impl, void *inst, AZPropertyForeachFunc cb, void *data);
/**
 * @brief Iterate over all visible interfaces of an instance or type
 *
 * Enumerates the interface tree in pre-order: direct interfaces (level 0),
 * then under each its declared sub-interfaces and super-interfaces (increasing
 * levels). The callback receives the resolved interface view
 * (iface_impl/iface_inst; inst locations are NULL for type-level calls).
 * Super-interfaces share the impl/inst of the sub-interface (offset 0).
 * The same interface type may appear multiple times through different paths;
 * the occurrences are distinguished by their locations.
 *
 * @param impl the implementation (and class) to enumerate
 * @param inst the instance (may be NULL for type-level enumeration)
 * @param cb the callback (AZ_OK to continue, positive to stop, negative error)
 * @param data user data passed to the callback
 * @return AZ_OK if the enumeration ran to completion, the callback's positive stop token, or the negative error code
 */
int az_foreach_interface (const AZImplementation *impl, void *inst, AZInterfaceForeachFunc cb, void *data);

/**
 * @brief Get property from instance
 * 
 * 
 * 
 * @param impl The type implementation
 * @param inst The type instance
 * @param key The property name
 * @param dst_impl The result implementation
 * @param dst_val The result value
 * @param dst_size The available result size
 * @return AZ_OK or error code
 */
int az_get_property_by_key (const AZImplementation *impl, void *inst, const uint8_t *key, const AZImplementation **dst_impl, AZValue *dst_val, unsigned int dst_size);
int az_get_function_by_key (const AZImplementation *impl, void *inst, const uint8_t *key, const AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val, unsigned int dst_size);
/**
 * @brief Set property by key and instance
 * 
 * The actual value must be assignable to the property type, no automatic conversion are performed.
 * 
 * @param impl The type implementation
 * @param inst The type instance
 * @param key The property name
 * @param prop_impl The value implementation
 * @param prop_inst The value instance
 * @return AZ_OK or error code
 */
int az_set_property_by_key (const AZImplementation *impl, void *inst, const uint8_t *key, const AZImplementation *prop_impl, void *prop_inst);

/**
 * @brief Get property value by defining class and property index
 *
 * This is the most generic method that can retrieve any kind of property given the defining class
 * and the implementation and instance of the type. For class/implementation level properties the
 * instance can be NULL.
 * The normal order of execution is to first call az_lookup_property and, if the property is
 * found, use def_class, impl, inst and index to fetch the property value.
 *
 * The implementation determines the class and which property definitions can be served:
 * - a class implementation (AZ_IMPL_IS_CLASS) provides class and instance storage only -
 *   getting class defined property
 * - an interface implementation (the class is AZ_CLASS_FROM_IMPL(impl)) additionally provides
 *   implementation storage - getting class/impl defined property
 *
 * @param def_klass the class where property is defined
 * @param impl the actual implementation of the instance
 * @param inst the actual instance (or NULL for class/implementation level properties)
 * @param id property index in class
 * @param prop_impl result implementation
 * @param prop_val result value
 * @param val_size result value size (value is autoboxed if size is bigger)
 * @return AZ_OK or error code
 */
int az_get_property_by_id (const AZClass *def_klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation **prop_impl, AZValue *prop_val, unsigned int val_size);
int az_set_property_by_id (const AZClass *def_klass, const AZImplementation *impl, void *inst, unsigned int id, const AZImplementation *prop_impl, void *prop_inst);

#ifdef __cplusplus
};
#endif

#endif
