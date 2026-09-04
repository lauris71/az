#define __AZ_INSTANCE_C__

/*
 * A run-time type library
 *
 * Copyright (C) 2016-2025 Lauris Kaplinski <lauris@kaplinski.com>
 * 
 * Licensed under GNU General Public License version 3 or any later version.
 */

#include <stdlib.h>
#include <string.h>

#include <az/base.h>
#include <az/boxed-interface.h>
#include <az/boxed-value.h>
#include <az/class.h>
#include <az/field.h>
#include <az/function.h>
#include <az/instance.h>
#include <az/object.h>
#include <az/private.h>
#include <az/reference.h>
#include <az/string.h>
#include <az/types.h>

 /*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. Initializes parent type
 * 2. Initializes all local interfaces
 * 3. instance_init
 */

void
az_instance_init_recursive (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int zeroed)
{
	uint32_t flags = klass->impl.flags;
	/* A default value replaces the initialization of this class and its whole ancestry */
	if (flags & AZ_FLAG_HAS_DEFAULT) {
		memcpy (inst, klass->default_value, klass->instance_size);
		return;
	}
	/* Recurse into the parent only if the parent chain has work (supersedes the fundamental-type check) */
	if (flags & AZ_FLAG_PARENT_CONSTRUCT) {
		az_instance_init_recursive (klass->parent, impl, inst, zeroed);
	}
	/* Interfaces */
	if (flags & AZ_FLAG_HAS_IFACE_CONSTRUCT) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
		for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
			/* Rejected declarations (type == 0) and construct-free interfaces are skipped;
			 * the typecode CONSTRUCT bit is set for interfaces with init/finalize/zero-memory
			 * (az_type_reserve), so no class lookup is needed for the common case */
			if (AZ_TYPE_FLAGS(ifentry->type) & AZ_FLAG_CONSTRUCT) {
				AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
				AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
				void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
				if (!zeroed && (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY)) memset (sub_inst, 0, sub_class->instance_size);
				az_instance_init_recursive (sub_class, sub_impl, sub_inst, zeroed || (sub_class->impl.flags & AZ_FLAG_ZERO_MEMORY));
			}
			ifentry += 1;
		}
	}
	/* Instance itself */
	if (flags & AZ_FLAG_HAS_INSTANCE_INIT) klass->instance_init (impl, inst);
}

/*
 * class - the current class
 * impl - the actual implementation (same as the original class for non-interface types)
 *
 * 1. instance_finalize
 * 2. Finalizes all local interfaces
 * 3. Finalizes parent type
 */

void
az_instance_finalize_recursive (const AZClass *klass, const AZImplementation *impl, void *inst)
{
	uint32_t flags = klass->impl.flags;
	/* A default value also replaces finalization: nothing in the subtree owns resources */
	if (flags & AZ_FLAG_HAS_DEFAULT) return;
	if (flags & AZ_FLAG_HAS_INSTANCE_FINALIZE) klass->instance_finalize (impl, inst);
	if (flags & AZ_FLAG_HAS_IFACE_CONSTRUCT) {
		const AZIFEntry *ifentry = az_class_iface_self(klass, 0);
		for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
			if (AZ_TYPE_FLAGS(ifentry->type) & AZ_FLAG_CONSTRUCT) {
				AZClass *sub_class = AZ_CLASS_FROM_TYPE(ifentry->type);
				AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
				void *sub_inst = (void *) ((char *) inst + ifentry->inst_offset);
				az_instance_finalize_recursive (sub_class, sub_impl, sub_inst);
			}
			ifentry += 1;
		}
	}
	if (flags & AZ_FLAG_PARENT_FINALIZE) {
		az_instance_finalize_recursive (klass->parent, impl, inst);
	}
}

void
az_instance_init (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail(impl != NULL);
	arikkei_return_if_fail(inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	uint32_t flags = klass->impl.flags;
	if (flags & AZ_FLAG_ZERO_MEMORY) memset (inst, 0, klass->instance_size);
	/* References are counted from one; this is inlined here so reference base
	 * classes do not need constructors (the flag test reads an already-hot word) */
	if (flags & AZ_FLAG_REFERENCE) ((AZReference *) inst)->refcount = 1;
	if (flags & AZ_INIT_WORK_MASK) az_instance_init_recursive (klass, impl, inst, flags & AZ_FLAG_ZERO_MEMORY);
}

void
az_instance_finalize (const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (impl != NULL);
	arikkei_return_if_fail (inst != NULL);
#endif
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (!AZ_CLASS_IS_ABSTRACT(klass));
#endif
	if (klass->impl.flags & AZ_FINALIZE_WORK_MASK) az_instance_finalize_recursive (klass, impl, inst);
}

void *
az_instance_new (unsigned int type)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail(az_type_is_valid(type), NULL);
	arikkei_return_val_if_fail(!AZ_TYPE_IS_INTERFACE(type), NULL);
#endif
	AZClass *klass = AZ_CLASS_FROM_TYPE(type);
	arikkei_return_val_if_fail (!(klass->impl.flags & AZ_FLAG_ABSTRACT), NULL);
	arikkei_return_val_if_fail(klass->instance_size > 0, NULL);
	void *inst;
	const AZInstanceAllocator *allocator = (klass->allocator_idx) ? az_class_allocators[klass->allocator_idx] : NULL;
	if (allocator && allocator->allocate) {
		inst = allocator->allocate (klass);
	} else {
		inst = malloc(klass->instance_size);
	}
	az_instance_init (&klass->impl, inst);
	return inst;
}

void *
az_instance_new_array (unsigned int type, unsigned int n_elements)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail(az_type_is_valid(type), NULL);
	arikkei_return_val_if_fail(!AZ_TYPE_IS_INTERFACE(type), NULL);
#endif
	AZClass *klass = az_type_get_class (type);
	arikkei_return_val_if_fail (!(klass->impl.flags & AZ_FLAG_ABSTRACT), NULL);
	arikkei_return_val_if_fail(klass->instance_size > 0, NULL);
	void *elements;
	const AZInstanceAllocator *allocator = (klass->allocator_idx) ? az_class_allocators[klass->allocator_idx] : NULL;
	if (allocator && allocator->allocate_array) {
		elements = allocator->allocate_array (klass, n_elements);
	} else {
		elements = malloc(n_elements * AZ_CLASS_ELEMENT_SIZE(klass));
	}
	for (unsigned int i = 0; i < n_elements; i++) {
		az_instance_init (&klass->impl, (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass));
	}
	return elements;
}

void
az_instance_delete (unsigned int type, void *instance)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail(az_type_is_valid(type));
	arikkei_return_if_fail(!AZ_TYPE_IS_INTERFACE(type));
#endif
	AZClass *klass = az_type_get_class (type);
	az_instance_finalize (&klass->impl, instance);
	const AZInstanceAllocator *allocator = (klass->allocator_idx) ? az_class_allocators[klass->allocator_idx] : NULL;
	if (allocator && allocator->free) {
		allocator->free (klass, instance);
	} else {
		free (instance);
	}
}

void
az_instance_delete_array (unsigned int type, void *elements, unsigned int nelements)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail(az_type_is_valid(type));
	arikkei_return_if_fail(!AZ_TYPE_IS_INTERFACE(type));
#endif
	AZClass *klass = az_type_get_class (type);
	for (unsigned int i = 0; i < nelements; i++) {
		void *instance = (char *) elements + i * AZ_CLASS_ELEMENT_SIZE(klass);
		az_instance_finalize (&klass->impl, instance);
	}
	const AZInstanceAllocator *allocator = (klass->allocator_idx) ? az_class_allocators[klass->allocator_idx] : NULL;
	if (allocator && allocator->free_array) {
		allocator->free_array (klass, elements, nelements);
	} else {
		free (elements);
	}
}

unsigned int
az_instance_serialize (const AZImplementation *impl, void *inst, unsigned char *d, unsigned int dlen, AZContext *ctx)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	return (klass->serialize) ? klass->serialize (impl, inst, d, dlen, ctx) : 0;
}

unsigned int
az_instance_to_string (const AZImplementation* impl, void *inst, unsigned char *d, unsigned int dlen)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	AZClass* klass = AZ_CLASS_FROM_IMPL(impl);
	return klass->to_string (impl, inst, d, dlen);
}

uint8_t *
az_instance_to_string_new (const AZImplementation *impl, void *inst)
{
	uint8_t buf[256];
	unsigned int len = az_instance_to_string (impl, inst, buf, sizeof (buf));
	uint8_t *str = (uint8_t *) malloc (len + 1);
	if (len < sizeof (buf)) {
		/* The representation fitted the local buffer and is terminated */
		memcpy (str, buf, len + 1);
	} else {
		az_instance_to_string (impl, inst, str, len + 1);
	}
	return str;
}

const AZImplementation *
az_instance_get_interface (const AZImplementation *impl, void *inst, unsigned int if_type, void **if_inst)
{
	arikkei_return_val_if_fail (impl != NULL, NULL);
	if (impl == AZ_BOXED_INTERFACE_IMPL) {
		AZBoxedInterface *boxed = (AZBoxedInterface *) inst;
		impl = boxed->impl;
		inst = boxed->inst;
	}
	if (az_type_is_a (AZ_IMPL_TYPE(impl), if_type)) {
		if (if_inst) *if_inst = inst;
		return impl;
	}
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	const AZIFEntry *ifentry = az_class_iface_all(klass, 0);
	for (uint16_t i = 0; i < klass->n_ifaces_all; i++) {
		if (az_type_is_a(ifentry->type, if_type)) {
			AZImplementation *sub_impl = (AZImplementation *) ((char *) impl + ifentry->impl_offset);
			if (if_inst) *if_inst = (char *) inst + ifentry->inst_offset;
			return sub_impl;
		}
		ifentry += 1;
	}
	if (if_inst) *if_inst = NULL;
	return NULL;
}

// fixme: This is wrong as inst can be NULL
unsigned int
az_instance_get_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation **dst_impl, AZValue64 *dst_val)
{
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	int idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_get_property_by_id (sub_class, AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, sub_inst, idx, dst_impl, &dst_val->value, 64, NULL);
}

unsigned int
az_instance_get_function_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, AZFunctionSignature *sig, const AZImplementation **dst_impl, AZValue *dst_val)
{
	const AZClass *def_class;
	const AZImplementation *def_impl;
	void *def_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	int idx = az_class_lookup_function (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, sig, &def_class, &def_impl, &def_inst);
	if (idx < 0) {
		return 0;
	}
	return az_instance_get_property_by_id (def_class, AZ_CLASS_FROM_IMPL(def_impl), def_impl, def_inst, idx, dst_impl, dst_val, 16, NULL);
}

unsigned int
az_instance_set_property_by_key (const AZImplementation *impl, void *inst, const unsigned char *key, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	int idx;
	const AZClass *sub_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (key != NULL, 0);
	AZString *str = az_string_new(key);
	idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, inst, str, &sub_class, &sub_impl, &sub_inst);
	az_string_unref(str);
	if (idx < 0) return 0;
	return az_instance_set_property_by_id (sub_class, sub_impl, sub_inst, idx, prop_impl, prop_inst, ctx);
}

/*
 * Read the containing integer of a masked (bit-field) property.
 * The containing type (and thus the width read at the field offset) is determined
 * by field->value_type_idx, the value is shifted right, then masked to mask_width bits.
 */
static uint64_t
az_field_masked_read (const AZField *prop, const AZValue *src)
{
	uint64_t v;
	switch (prop->value_type_idx) {
		case AZ_TYPE_IDX_UINT8:
			v = src->uint8_v;
			break;
		case AZ_TYPE_IDX_UINT16:
			v = src->uint16_v;
			break;
		case AZ_TYPE_IDX_UINT64:
			v = src->uint64_v;
			break;
		default:
			/* Legacy masked fields (value_type unset) are stored in uint32 */
			v = src->uint32_v;
			break;
	}
	if (prop->mask_width >= 64) return v >> prop->shift;
	return (v >> prop->shift) & ((1ULL << prop->mask_width) - 1);
}

/*
 * Write a masked (bit-field) property into its containing integer.
 * Only the bits covered by the mask are modified, the value is masked to
 * mask_width bits and shifted left; the result is merged into the containing
 * value whose width is determined by field->value_type_idx.
 */
static void
az_field_masked_write (const AZField *prop, AZValue *dst, uint64_t v)
{
	uint64_t mask = (prop->mask_width >= 64) ? ~0ULL : ((1ULL << prop->mask_width) - 1);
	uint64_t cmask = mask << prop->shift;
	uint64_t shifted = (v & mask) << prop->shift;
	switch (prop->value_type_idx) {
		case AZ_TYPE_IDX_UINT8:
			dst->uint8_v = (uint8_t) ((dst->uint8_v & ~(uint8_t) cmask) | shifted);
			break;
		case AZ_TYPE_IDX_UINT16:
			dst->uint16_v = (uint16_t) ((dst->uint16_v & ~(uint16_t) cmask) | shifted);
			break;
		case AZ_TYPE_IDX_UINT64:
			dst->uint64_v = (dst->uint64_v & ~cmask) | shifted;
			break;
		default:
			/* Legacy masked fields (value_type unset) are stored in uint32 */
			dst->uint32_v = (uint32_t) ((dst->uint32_v & ~(uint32_t) cmask) | shifted);
			break;
	}
}

unsigned int
az_instance_set_property_by_id (const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation *prop_impl, void *prop_inst, AZContext *ctx)
{
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (!AZ_FIELD_IS_FINAL(&klass->props_self[idx]), 0);
	arikkei_return_val_if_fail (AZ_FIELD_WRITE(&klass->props_self[idx]) != AZ_FIELD_WRITE_NONE, 0);
	const AZField *prop = &klass->props_self[idx];
	if (!strcmp((const char *) prop->key->str, "cameraController")) {
		fprintf (stderr, ".");
	}
	if (AZ_TYPE_IS_INTERFACE(prop->type)) {
		arikkei_return_val_if_fail (!prop_impl || az_type_implements(AZ_IMPL_TYPE(prop_impl), prop->type), 0);
		if (AZ_FIELD_IS_FUNCTION(prop) && prop_impl) {
			AZFunctionInstance *func_inst;
			const AZFunctionImplementation *func_impl = (const AZFunctionImplementation *) az_instance_get_interface (prop_impl, prop_inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
			const AZFunctionSignature *sig = az_function_get_signature (func_impl, func_inst);
			if (prop->signature && !az_function_signature_is_assignable_to (sig, prop->signature, 1)) {
				fprintf (stderr, ".");
			}
			arikkei_return_val_if_fail (!prop->signature || az_function_signature_is_assignable_to (sig, klass->props_self[idx].signature, 1), 0);
		}
	} else {
		arikkei_return_val_if_fail (!prop_impl || az_type_is_a(AZ_IMPL_TYPE(prop_impl), prop->type), 0);
	}
	if (AZ_FIELD_WRITE(prop) == AZ_FIELD_WRITE_VALUE) {
		AZValue *val;
		if (AZ_FIELD_SPEC(prop) == AZ_FIELD_INSTANCE) {
			val = (AZValue *) ((char *) inst + prop->offset);
		} else if (AZ_FIELD_SPEC(prop) == AZ_FIELD_IMPLEMENTATION) {
			val = (AZValue *) ((char *) impl + prop->offset);
		} else {
			val = (AZValue *) ((char *) klass + prop->offset);
		}
		if (prop->mask_width) {
			if (prop->type == AZ_TYPE_BOOLEAN) {
				uint64_t v = (prop_inst && ((AZValue *) prop_inst)->boolean_v) ? 1 : 0;
				az_field_masked_write (prop, val, v ^ prop->bits);
			} else if (AZ_TYPE_IS_UNSIGNED(prop->type)) {
				uint64_t v = 0;
				if (prop_inst) {
					switch (prop->type) {
						case AZ_TYPE_UINT8:
							v = ((AZValue *) prop_inst)->uint8_v;
							break;
						case AZ_TYPE_UINT16:
							v = ((AZValue *) prop_inst)->uint16_v;
							break;
						case AZ_TYPE_UINT32:
							v = ((AZValue *) prop_inst)->uint32_v;
							break;
						default:
							v = ((AZValue *) prop_inst)->uint64_v;
							break;
					}
				}
				az_field_masked_write (prop, val, v);
			} else {
				return 0;
			}
		} else {
			az_value_set_from_inst (prop_impl, val, prop_inst);
		}
	} else if (AZ_FIELD_WRITE(prop) == AZ_FIELD_WRITE_PACKED) {
		AZPackedValue *val;
		if (AZ_FIELD_SPEC(prop) == AZ_FIELD_INSTANCE) {
			val = (AZPackedValue *) ((char *) inst + prop->offset);
		} else if (AZ_FIELD_SPEC(prop) == AZ_FIELD_IMPLEMENTATION) {
			val = (AZPackedValue *) ((char *) impl + prop->offset);
		} else {
			val = (AZPackedValue *) ((char *) klass + prop->offset);
		}
		az_packed_value_set_from_impl_instance (val, prop_impl, prop_inst);
	} else if (AZ_FIELD_WRITE(prop) == AZ_FIELD_WRITE_METHOD) {
		return klass->set_property (impl, inst, idx, prop_impl, prop_inst, NULL);
	}
	return 1;
}

unsigned int
az_instance_get_property_by_id (const AZClass *def_klass, const AZClass *klass, const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, unsigned int val_size, AZContext *ctx)
{
	arikkei_return_val_if_fail (def_klass != NULL, 0);
	arikkei_return_val_if_fail (klass != NULL, 0);
	arikkei_return_val_if_fail (prop_impl != NULL, 0);
	arikkei_return_val_if_fail (prop_val != NULL, 0);
	arikkei_return_val_if_fail (AZ_FIELD_READ(&def_klass->props_self[idx]) != AZ_FIELD_READ_NONE, 0);

	const AZField *prop = &def_klass->props_self[idx];

	switch(AZ_FIELD_READ(prop)) {
		case AZ_FIELD_READ_VALUE: {
			/* Bare value inside instance/implementation/class */
			AZValue *src;
			if (AZ_FIELD_SPEC(prop) == AZ_FIELD_INSTANCE) {
				arikkei_return_val_if_fail(inst != NULL, 0);
				src = (AZValue *) ((char *) inst + prop->offset);
			} else if (AZ_FIELD_SPEC(prop) == AZ_FIELD_IMPLEMENTATION) {
				arikkei_return_val_if_fail(impl != NULL, 0);
				src = (AZValue *) ((char *) impl + prop->offset);
			} else {
				src = (AZValue *) ((char *) klass + prop->offset);
			}
			if (prop->mask_width) {
				if (prop->type == AZ_TYPE_BOOLEAN) {
					uint64_t v = az_field_masked_read (prop, src) ^ prop->bits;
					*prop_impl = &AZBooleanKlass.impl;
					prop_val->boolean_v = (v != 0);
					return 1;
				} else if (AZ_TYPE_IS_UNSIGNED(prop->type)) {
					uint64_t v = az_field_masked_read (prop, src);
					AZClass *prop_class = AZ_CLASS_FROM_TYPE(prop->type);
					*prop_impl = &prop_class->impl;
					switch (prop->type) {
						case AZ_TYPE_UINT8:
							prop_val->uint8_v = (uint8_t) v;
							break;
						case AZ_TYPE_UINT16:
							prop_val->uint16_v = (uint16_t) v;
							break;
						case AZ_TYPE_UINT32:
							prop_val->uint32_v = (uint32_t) v;
							break;
						case AZ_TYPE_UINT64:
							prop_val->uint64_v = v;
							break;
						default:
							return 0;
					}
					return 1;
				} else {
					return 0;
				}
			}
			if (AZ_TYPE_IS_OBJECT(prop->type)) {
				az_value_set_object (prop_impl, prop_val, (AZObject *) src->reference);
			} else {
				AZClass *prop_class = AZ_CLASS_FROM_TYPE(prop->type);
				if (!AZ_CLASS_IS_FINAL(prop_class)) {
					fprintf(stderr, ".");
				}
				arikkei_return_val_if_fail (AZ_CLASS_IS_FINAL(prop_class), 0);
				*prop_impl = az_value_copy_autobox (&prop_class->impl, prop_val, src, val_size);
			}
			break;
		}
		case AZ_FIELD_READ_INSTANCE: {
			/* Embedded instance inside instance/implementation/class */
			AZValue *src;
			if (AZ_FIELD_SPEC(prop) == AZ_FIELD_INSTANCE) {
				arikkei_return_val_if_fail(inst != NULL, 0);
				src = (AZValue *) ((char *) inst + prop->offset);
			} else if (AZ_FIELD_SPEC(prop) == AZ_FIELD_IMPLEMENTATION) {
				arikkei_return_val_if_fail(impl != NULL, 0);
				src = (AZValue *) ((char *) impl + prop->offset);
			} else {
				src = (AZValue *) ((char *) klass + prop->offset);
			}
			AZClass *prop_class = AZ_CLASS_FROM_TYPE(prop->type);
			arikkei_return_val_if_fail (AZ_CLASS_IS_FINAL(prop_class), 0);
			*prop_impl = az_value_set_from_inst_autobox (&prop_class->impl, prop_val, val_size, src);
			break;
		}
		case AZ_FIELD_READ_PACKED: {
			/* Packed value inside instance */
			AZPackedValue *src;
			if (AZ_FIELD_SPEC(prop) == AZ_FIELD_INSTANCE) {
				arikkei_return_val_if_fail(inst != NULL, 0);
				src = (AZPackedValue *) ((char *) inst + prop->offset);
			} else if (AZ_FIELD_SPEC(prop) == AZ_FIELD_IMPLEMENTATION) {
				arikkei_return_val_if_fail(impl != NULL, 0);
				src = (AZPackedValue *) ((char *) impl + prop->offset);
			} else {
				AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
				src = (AZPackedValue *) ((char *) klass + prop->offset);
			}
			*prop_impl = az_value_copy_autobox (src->impl, prop_val, &src->v, val_size);
			break;
		}
		case AZ_FIELD_READ_STORED_STATIC:
			/* Packed value inside field definition */
			if (prop->value.impl) {
				*prop_impl = az_value_copy_autobox (prop->value.impl, prop_val, &prop->value.v, val_size);
			} else {
				*prop_impl = NULL;
			}
			break;
		case AZ_FIELD_READ_METHOD:
			return def_klass->get_property (impl, inst, idx, prop_impl, prop_val, ctx);
		default:
			/* Not readable */
			return 0;
	}
	return 1;
}
