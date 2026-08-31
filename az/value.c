#define __AZ_VALUE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arikkei/arikkei-iolib.h>

#include <az/boxed-interface.h>
#include <az/boxed-value.h>
#include <az/class.h>
#include <az/primitives.h>
#include <az/private.h>
#include <az/reference-of.h>

#include <az/value.h>

void
az_value_set_from_inst (const AZImplementation *impl, AZValue *dst, void *inst)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (dst != NULL);
#endif
	if (!impl) {
		/* Untyped null */
		az_value_set_null (dst);
		return;
	}
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (inst != NULL);
#endif
	klass = AZ_CLASS_FROM_IMPL(impl);
	if (klass->impl.flags & AZ_FLAG_BLOCK) {
		dst->block = inst;
		if (klass->impl.flags & AZ_FLAG_REFERENCE) {
			if (inst) az_reference_ref (dst->reference);
		}
	} else {
		if (klass->instance_size) memcpy (dst, inst, klass->instance_size);
	}
}

unsigned int
az_value_deserialize (const AZImplementation *impl, AZValue *val, const unsigned char *s, unsigned int slen, AZContext *ctx)
{
	AZClass *klass;
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (val != NULL, 0);
#endif
	klass = AZ_CLASS_FROM_IMPL(impl);
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (klass != NULL, 0);
#endif
	return (klass->deserialize) ? klass->deserialize (impl, val, s, slen, ctx) : 0;
}

unsigned int
az_value_equals (const AZImplementation *impl, const AZValue *lhs, const AZValue *rhs)
{
	AZClass *klass;
	/* Untyped nulls (and the same block) are equal */
	if (!impl) return lhs->block == rhs->block;
	if (AZ_IMPL_IS_BLOCK(impl)) {
		return lhs->block == rhs->block;
	} else if (AZ_IMPL_IS_VALUE(impl)) {
		klass = AZ_CLASS_FROM_IMPL(impl);
		if (klass->instance_size) return !memcmp (lhs, rhs, klass->instance_size);
	}
	return 0;
}

unsigned int
az_value_equals_instance (const AZImplementation *impl, const AZValue *lhs, const void *rhs)
{
	AZClass *klass;
	klass = AZ_CLASS_FROM_IMPL(impl);
	if (klass->impl.flags & AZ_FLAG_BLOCK) {
		return lhs->block == rhs;
	}
	if (AZ_CLASS_IS_VALUE(klass) && klass->instance_size) {
		return !memcmp (lhs, rhs, klass->instance_size);
	}
	return 0;
}

void *
az_value_new_array(const AZImplementation *impl, unsigned int length)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	void *data = arikkei_aligned_alloc(length * az_class_element_size(klass), klass->alignment + 1);
	if (impl) {
		for (unsigned int i = 0; i < length; i++) {
			az_value_init(impl, (AZValue *) ((char *) data + i * az_class_element_size(klass)));
		}
	}
	return data;
}

void az_value_delete_array(const AZImplementation *impl, void *data, unsigned int length)
{
	AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
	for (unsigned int i = 0; i < length; i++) {
		az_value_clear(impl, (AZValue *) ((char *) data + i * az_class_element_size(klass)));
	}
	arikkei_aligned_free(data);
}

static AZConversionResult
az_value_convert_internal (const AZImplementation **dst_impl, AZValue *dst_val, unsigned int dst_size, const AZImplementation *src_impl, const AZValue *src_val, unsigned int to_type, AZConversionType conversion_type, unsigned int autobox)
{
	/* Nothing can be converted to None */
	if (!to_type) return AZ_CONVERSION_FAILED;
	/* None is converted to typed null (the value carries the target type) */
	/* fixme: temporarily automatic, should require AZ_CONVERT_EXPLICIT (has to be fixed in Aosora first) */
	if (!src_impl) {
		if (to_type == AZ_TYPE_ANY) {
			*dst_impl = NULL;
			dst_val->reference = NULL;
			return AZ_CONVERSION_EXACT;
		}
		if (AZ_TYPE_IS_BLOCK(to_type)) {
			*dst_impl = AZ_IMPL_FROM_TYPE(to_type);
			dst_val->block = NULL;
			return AZ_CONVERSION_EXACT;
		}
		return AZ_CONVERSION_FAILED;
	}
	unsigned int src_type = AZ_IMPL_TYPE(src_impl);
	if (autobox) {
		/* Boxed values are treated transparently: unboxed at input; if the content fits
		 * into dst it is copied directly, otherwise az_value_copy_autobox re-boxes it.
		 * A boxed value can only hold a struct and a struct is converted to its
		 * supertype automatically, so the plain supertype check below suffices */
		if (src_type == AZ_TYPE_BOXED_VALUE) {
			AZBoxedValue *box = (AZBoxedValue *) src_val->reference;
			src_impl = &box->klass->impl;
			src_val = &box->val;
			src_type = AZ_CLASS_TYPE(box->klass);
		}
		/* Nothing is converted to AZ_TYPE_BOXED_VALUE: a boxed value is a storage detail
		 * created by az_value_copy_autobox when the content does not fit, never a
		 * conversion target (no struct converts to boxed value) */
	}
	/* Same type or supertype (with autobox also the box types themselves: Any, Reference, ...) */
	if (az_type_is_a (src_type, to_type)) {
		if (autobox) {
			*dst_impl = az_value_copy_autobox (src_impl, dst_val, src_val, dst_size);
		} else {
			az_value_copy (src_impl, dst_val, src_val);
			*dst_impl = src_impl;
		}
		return AZ_CONVERSION_EXACT;
	}
	if (autobox && (src_type == AZ_TYPE_BOXED_INTERFACE)) {
		/*
		 * A boxed interface is a storage detail: its type is the contained interface
		 * type, the value inside is just the stored container (kept alive for the
		 * interface lifecycle). Only the interface is converted, never the value.
		 *
		 * The target interface has to be resolved from the presented view, NOT from
		 * the contained value: the view may be narrower (e.g. the keyset of a map is
		 * a collection, while the map itself is also a collection).
		 */
		AZBoxedInterface *box = (AZBoxedInterface *) src_val->reference;
		if (box->impl) {
			/* The interface type currently presented by the box */
			unsigned int if_type = AZ_CLASS_TYPE(AZ_CLASS_FROM_IMPL(box->impl));
			if (az_type_is_a (if_type, to_type)) {
				/* The presented interface is already a subtype of to_type - keep the box */
				az_boxed_interface_ref (box);
				dst_val->reference = &box->reference;
				*dst_impl = src_impl;
				return AZ_CONVERSION_EXACT;
			}
			if (AZ_TYPE_IS_INTERFACE(to_type)) {
				/* Resolve to_type from the presented interface */
				void *sub_inst;
				const AZImplementation *sub_impl = az_instance_get_interface (box->impl, box->inst, to_type, &sub_inst);
				if (sub_impl) {
					/* Re-box the original containing value (lifecycle) with the resolved interface */
					AZBoxedInterface *new_box = az_boxed_interface_new (box->val.impl, az_value_get_inst (box->val.impl, &box->val.v), sub_impl, sub_inst);
					dst_val->reference = &new_box->reference;
					*dst_impl = src_impl;
					return AZ_CONVERSION_EXACT;
				}
			}
		}
		/* The contained value is never converted */
		return AZ_CONVERSION_FAILED;
	}
	if (autobox) {
		/* Anything can be converted to implemented interface */
		if (AZ_TYPE_IS_INTERFACE(to_type) && az_type_implements (src_type, to_type)) {
			dst_val->reference = (AZReference *) az_boxed_interface_new_from_impl_value (src_impl, src_val, to_type);
			*dst_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_INTERFACE);
			return AZ_CONVERSION_EXACT;
		}
	}
	/* Arithmetic types */
	if (AZ_TYPE_IS_ARITHMETIC (to_type) && AZ_TYPE_IS_ARITHMETIC (src_type)) {
		if (az_primitive_can_convert (to_type, src_type) <= conversion_type) {
			AZClass *to_klass = AZ_CLASS_FROM_TYPE(to_type);
			*dst_impl = &to_klass->impl;
			return (AZConversionResult) az_convert_arithmetic_type (to_type, dst_val, src_type, src_val);
		}
		return AZ_CONVERSION_FAILED;
	}
	/* Explicit conversions change the value or type semantics */
	if (conversion_type >= AZ_CONVERT_EXPLICIT) {
		if ((src_type == AZ_TYPE_UINT64) && (to_type == AZ_TYPE_POINTER)) {
			*dst_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_POINTER);
			dst_val->pointer_v = (void *) (uintptr_t) src_val->uint64_v;
			return AZ_CONVERSION_EXACT;
		}
		if ((src_type == AZ_TYPE_POINTER) && (to_type == AZ_TYPE_UINT64)) {
			*dst_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_UINT64);
			dst_val->uint64_v = (uint64_t) (uintptr_t) src_val->pointer_v;
			return AZ_CONVERSION_EXACT;
		}
		if (AZ_TYPE_IS_BLOCK(src_type) && (to_type == AZ_TYPE_POINTER)) {
			*dst_impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_POINTER);
			dst_val->pointer_v = src_val->block;
			return AZ_CONVERSION_EXACT;
		}
	}
	/* Nothing else can be converted */
	return AZ_CONVERSION_FAILED;
}

AZConversionResult
az_value_convert (const AZImplementation **dst_impl, AZValue *dst_val, const AZImplementation *src_impl, const AZValue *src_val, unsigned int to_type, AZConversionType conversion_type)
{
	/* No boxing: boxed values and boxed interfaces are plain reference types */
	return az_value_convert_internal (dst_impl, dst_val, 0, src_impl, src_val, to_type, conversion_type, 0);
}

AZConversionResult
az_value_convert_autobox (const AZImplementation **dst_impl, AZValue *dst_val, unsigned int dst_size, const AZImplementation *src_impl, const AZValue *src_val, unsigned int to_type, AZConversionType conversion_type)
{
	return az_value_convert_internal (dst_impl, dst_val, dst_size, src_impl, src_val, to_type, conversion_type, 1);
}

unsigned int
az_value_convert_in_place (const AZImplementation **impl, AZValue *val, unsigned int to_type)
{
	/* Nothing can be converted to None */
	if (!to_type) return 0;
	AZClass* to_klass = az_type_get_class (to_type);
	/* None can be converted to null reference */
	if (!*impl) {
		if ((to_type == AZ_TYPE_ANY) || (to_klass->impl.flags & AZ_FLAG_BLOCK)) {
			*impl = NULL;
			val->reference = NULL;
			return 1;
		}
		return 0;
	}
	/* Anything can be converted to supertype (this includes Any) */
	if (az_type_is_a (AZ_IMPL_TYPE(*impl), to_type)) {
		return 1;
	}
	/* Anything can be converted to implemented interface */
	if ((to_klass->impl.flags & AZ_FLAG_INTERFACE) && az_type_implements (AZ_IMPL_TYPE(*impl), to_type)) {
		val->reference = (AZReference *) az_boxed_interface_new_from_impl_value (*impl, val, to_type);
		*impl = AZ_IMPL_FROM_TYPE(AZ_TYPE_BOXED_INTERFACE);
		return 1;
	}
	/* Arithmetic types */
	if (AZ_TYPE_IS_ARITHMETIC (to_type) && AZ_TYPE_IS_ARITHMETIC (AZ_IMPL_TYPE(*impl))) {
		if (az_primitive_can_convert (to_type, AZ_IMPL_TYPE(*impl)) <= AZ_CONVERT_CONDITIONAL) {
			az_convert_arithmetic_type (to_type, val, AZ_IMPL_TYPE(*impl), val);
			*impl = &to_klass->impl;
			return 1;
		}
		return 0;
	}
	/* Nothing else can be converted */
	return 0;
}
