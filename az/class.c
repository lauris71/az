#define __AZ_CLASS_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arikkei/arikkei-strlib.h>

#include <az/class.h>
#ifdef AZ_HAS_PROPERTIES
#include <az/field.h>
#include <az/function-value.h>
#include <az/packed-value.h>
#include <az/private.h>
#endif
#include <az/string.h>

static unsigned int
az_ifaces_all_reserve(unsigned int n_ifaces)
{
	unsigned int next = az_ifaces_all_len;
	if ((next + n_ifaces) > az_ifaces_all_size) {
		az_ifaces_all_size = az_ifaces_all_size << 2;
		if ((next + n_ifaces) > az_ifaces_all_size) az_ifaces_all_size = next + n_ifaces;
		if (az_ifaces_all_size < 256) az_ifaces_all_size = 256;
		az_ifaces_all = (AZIFEntry *) realloc(az_ifaces_all, az_ifaces_all_size * sizeof(AZIFEntry));
	}
	az_ifaces_all_len += n_ifaces;
	return next;
}

static void az_class_pre_init (AZClass *klass, unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name);

static unsigned char zero_val[16] = { 0 };

static unsigned int classes_size = 0;

/* Method implementations */
static unsigned int impl_call_setStaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);
static unsigned int impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx);

/* Properties */

enum {
	FUNC_SETSTATICPROPERTY,
	FUNC_GETSTATICPROPERTY,
	NUM_PROPERTIES
};

void
az_classes_init (void)
{
	if (az_classes) return;
	classes_size = AZ_NUM_BASE_TYPES + 32;
	az_classes = (AZClass **) malloc (classes_size * sizeof (AZClass *));
	az_types = (AZTypeInfo *) malloc (classes_size * sizeof (AZTypeInfo));
	az_classes[AZ_TYPE_INDEX(AZ_TYPE_NONE)] = NULL;
	az_num_classes = 1;
	memset (&az_types[AZ_TYPE_INDEX(AZ_TYPE_NONE)], 0, sizeof(AZTypeInfo));
}

static AZClass *impl_class = NULL;
static AZClass *class_class = NULL;

static unsigned int
implementation_to_string (const AZImplementation* impl, void *instance, unsigned char *buf, unsigned int len)
{
	return arikkei_strncpy (buf, len, (const unsigned char *) "Implementation");
}

void
az_init_implementation_class (void)
{
	impl_class = az_class_new_with_type (AZ_TYPE_IMPLEMENTATION, AZ_TYPE_BLOCK, sizeof (AZClass), sizeof (AZImplementation), AZ_FLAG_FINAL, (const uint8_t *) "implementation");
	impl_class->alignment = 3;
	impl_class->to_string = implementation_to_string;
}

void
az_implementation_class_post_init (void)
{
	az_class_set_num_properties (AZ_CLASS_FROM_TYPE(AZ_TYPE_IMPLEMENTATION), NUM_PROPERTIES);
	az_class_define_method_va (AZ_CLASS_FROM_TYPE(AZ_TYPE_IMPLEMENTATION), FUNC_SETSTATICPROPERTY, (const unsigned char *) "setStaticProperty", impl_call_setStaticProperty, AZ_TYPE_NONE, 2, AZ_TYPE_STRING, AZ_TYPE_ANY);
	az_class_define_method_va (AZ_CLASS_FROM_TYPE(AZ_TYPE_IMPLEMENTATION), FUNC_GETSTATICPROPERTY, (const unsigned char *) "getStaticProperty", impl_call_getstaticProperty, AZ_TYPE_ANY, 1, AZ_TYPE_STRING);
}

static unsigned int
class_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	unsigned int pos;
	AZClass *inst_class = (AZClass *) inst;
	pos = arikkei_memcpy_str (buf, len, inst_class->name);
	pos += arikkei_strncpy (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " class");
	return pos;
}

static unsigned int
impl_call_setStaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *sub_class;
	const AZImplementation *prop_impl;
	void *prop_inst;
	AZField *prop;
	prop_idx = az_lookup_property (az_type_get_class (impl->type), impl, NULL, key->str, &sub_class, &prop_impl, &prop_inst, &prop);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	arikkei_return_val_if_fail (prop->spec == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (!prop->is_final, 0);
	arikkei_return_val_if_fail (prop->write != AZ_FIELD_WRITE_NONE, 0);
	az_instance_set_property_by_id (sub_class, prop_impl, NULL, prop_idx, arg_impls[2], az_instance_from_value (arg_impls[2], arg_vals[2]), ctx);
	return 1;
}

static unsigned int
impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *sub_class;
	const AZImplementation *prop_impl;
	void *prop_inst;
	AZField *prop;
	prop_idx = az_lookup_property (az_type_get_class (impl->type), impl, NULL, key->str, &sub_class, &prop_impl, &prop_inst, &prop);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	arikkei_return_val_if_fail (prop->spec == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (prop->read != AZ_FIELD_READ_NONE, 0);
	az_instance_get_property_by_id (sub_class, prop_impl, NULL, prop_idx, ret_impl, ret_val, NULL);
	return 1;
}

void
az_class_class_init (void)
{
	class_class = az_class_new_with_type (AZ_TYPE_CLASS, AZ_TYPE_IMPLEMENTATION, sizeof (AZClass), sizeof (AZClass), AZ_FLAG_FINAL, (const uint8_t *) "class");
	class_class->alignment = 7;
	class_class->to_string = class_to_string;
}

void
az_class_class_post_init (void)
{
	az_class_set_num_properties (class_class, 1);
	az_class_define_property (class_class, 0, (const unsigned char *) "parent", AZ_TYPE_CLASS, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZClass, parent), NULL, NULL);
}

AZClass *
az_class_new (uint32_t *type, const unsigned char *name, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags,
	void (*instance_init) (const AZImplementation *, void *),
	void (*instance_finalize) (const AZImplementation *, void *))
{
#ifdef AZ_SAFETY_CHECKS
	if (!az_classes) az_init ();
	arikkei_return_val_if_fail ((parent_type == AZ_TYPE_NONE) || (class_size >= AZ_CLASS_FROM_TYPE(parent_type)->class_size), 0);
	arikkei_return_val_if_fail ((parent_type == AZ_TYPE_NONE) || (instance_size >= AZ_CLASS_FROM_TYPE(parent_type)->instance_size), 0);
#endif
	arikkei_return_val_if_fail (!AZ_TYPE_IS_FINAL(parent_type), 0);
	if (az_num_classes >= classes_size) {
		classes_size += 32;
		az_classes = (AZClass **) realloc (az_classes, classes_size * sizeof (AZClass *));
		az_types = (AZTypeInfo *) realloc (az_types, classes_size * sizeof(AZTypeInfo));
	}
	*type = az_num_classes++;

	AZClass *klass = az_class_new_with_type(*type, parent_type, class_size, instance_size, flags, name);
	klass->instance_init = instance_init;
	klass->instance_finalize = instance_finalize;
	return klass;
}

AZClass *
az_class_new_with_type (unsigned int type, unsigned int parent_type, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail(AZ_TYPE_INDEX(type) < classes_size, NULL);
#endif
	AZClass *klass = (AZClass *) malloc (class_size);
	az_class_pre_init (klass, type, parent_type, class_size, instance_size, flags, name);
	az_classes[AZ_TYPE_INDEX(type)] = klass;
	/* We have to use class flags here because of parent chaining */
	az_types[AZ_TYPE_INDEX(type)].flags = klass->flags;
	az_types[AZ_TYPE_INDEX(type)].pidx = parent_type;
	return klass;
}

void
az_class_new_with_value (AZClass *klass)
{
	unsigned int type = AZ_TYPE_INDEX(klass->implementation.type);
	az_classes[type] = klass;
	/* We have to use class flags here because of parent chaining */
	az_types[type].flags = klass->flags;
	az_types[type].pidx = klass->parent ? klass->parent->implementation.type : AZ_TYPE_NONE;
}

static void
az_class_pre_init (AZClass *klass, unsigned int type, unsigned int parent, unsigned int class_size, unsigned int instance_size, unsigned int flags, const uint8_t *name)
{
	memset (klass, 0, class_size);
	klass->default_val = zero_val;
	if (parent) {
		AZClass *parent_class = AZ_CLASS_FROM_TYPE(parent);
		unsigned int i;
		memcpy (klass, parent_class, parent_class->class_size);
		/* Overwrite values from supertype */
		klass->flags &= ~AZ_FLAG_ABSTRACT;
		klass->parent = parent_class;
		klass->n_ifaces_self = 0;
#ifdef AZ_HAS_PROPERTIES
		klass->n_properties_self = 0;
		klass->properties_self = NULL;
#endif
	}
	klass->implementation.type = type;
	klass->flags |= flags;
	klass->name = name;
	klass->class_size = class_size;
	klass->instance_size = instance_size;
	klass->init_recursive = NULL;
}

static void
az_class_init_recursive (AZClass* klass, AZClass* new_class)
{
	if (klass->parent) az_class_init_recursive (klass->parent, new_class);
	if (klass->init_recursive) {
        klass->init_recursive (new_class);
    }
}

void
az_class_set_num_interfaces (AZClass *klass, unsigned int n_ifaces)
{
	klass->n_ifaces_self = n_ifaces;
	if (n_ifaces > 2) {
		static unsigned int n_allocations = 0, allocated = 0;
		klass->ifaces_self = (AZIFEntry *) malloc(n_ifaces * sizeof(AZIFEntry));
		n_allocations += 1;
		allocated += n_ifaces;
		fprintf(stderr, "az_class_set_num_interfaces(): Allocated %u (%u %u)\n", n_ifaces, n_allocations, allocated);
#ifdef AZ_SAFETY_CHECKS
        memset (klass->ifaces_self, 0, n_ifaces * sizeof (AZIFEntry));
#endif
	} else {
#ifdef AZ_SAFETY_CHECKS
        memset (klass->ifaces, 0, n_ifaces * sizeof (AZIFEntry));
#endif
	}
}

void
az_class_declare_interface (AZClass *klass, unsigned int idx, unsigned int type, unsigned int impl_offset, unsigned int inst_offset)
{
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_ifaces_self);
	arikkei_return_if_fail (az_type_is_a (type, AZ_TYPE_INTERFACE));
	if (klass->n_ifaces_self <= 2) {
#ifdef AZ_SAFETY_CHECKS
	    arikkei_return_if_fail (!klass->ifaces[idx].type);
#endif
		klass->ifaces[idx] = (AZIFEntry) {type, impl_offset, inst_offset};
	} else {
#ifdef AZ_SAFETY_CHECKS
	    arikkei_return_if_fail (!klass->ifaces_self[idx].type);
#endif
		klass->ifaces_self[idx] = (AZIFEntry) {type, impl_offset, inst_offset};
	}
	/* For standalone types implementation is in class and should be initialized */
	/* For interface types sub-implementation is inside implementation and should be initialized later */
	/* fixme: The correct way to do this is to create two methods: */
	/* az_class_declare_interface and */
	/* az_implementation_declare_interface */
	/* And maybe delay implementation initialization to post_init */
	if (!az_type_is_a (klass->implementation.type, AZ_TYPE_INTERFACE)) az_implementation_init ((AZImplementation *) ((char *) klass + impl_offset), type);
}

void
az_class_post_init (AZClass *klass)
{
	unsigned int i;
	arikkei_return_if_fail (!(klass->alignment & (klass->alignment + 1)));
#ifdef AZ_SAFETY_CHECKS
	for (i = 0; i < klass->n_ifaces_self; i++) {
		AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[i] : &klass->ifaces_self[i];
		if (!ifentry->type) {
			fprintf (stderr, "az_class_post_init: Klass %s interface %u is not defined\n", klass->name, i);
		}
	}
#ifdef AZ_HAS_PROPERTIES
	for (i = 0; i < klass->n_properties_self; i++) {
		if (!klass->properties_self[i].key) {
			fprintf (stderr, "az_class_post_init: Klass %s property %u is not defined\n", klass->name, i);
		}
	}
#endif
#endif
	if (klass->n_ifaces_self || klass->instance_init || klass->instance_finalize) {
		klass->flags |= AZ_FLAG_CONSTRUCT;
		az_types[AZ_TYPE_INDEX(klass->implementation.type)].flags |= AZ_FLAG_CONSTRUCT;
	}
	if (klass->n_ifaces_self) {
		/* Count all interfaces */
		klass->n_ifaces_all = klass->parent->n_ifaces_all;
		for (i = 0; i < klass->n_ifaces_self; i++) {
			/* At this stage n_ifaces_all has still parent class value */
			AZIFEntry *ifentry = (klass->n_ifaces_self <= 2) ? &klass->ifaces[i] : &klass->ifaces_self[i];
			AZClass *iface_class = AZ_CLASS_FROM_TYPE(ifentry->type);
			klass->n_ifaces_all += (1 + iface_class->n_ifaces_all);
		}
		/*
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
		if (klass->n_ifaces_self == klass->n_ifaces_all) {
			/* Share interface definitions */
			if (klass->n_ifaces_self > 2) {
				klass->ifaces_all = klass->ifaces_self;
			}
		} else {
			if (klass->n_ifaces_self == 2) {
				/* Have to move self interfaces */
				static unsigned int n_allocations = 0, allocated = 0;
				AZIFEntry *ifaces = (AZIFEntry *) malloc(klass->n_ifaces_self * sizeof(AZIFEntry));
				n_allocations += 1;
				allocated += klass->n_ifaces_self;
				fprintf(stderr, "az_class_post_init(): Allocated self %u (%u %u)\n", klass->n_ifaces_self, n_allocations, allocated);
				memcpy(ifaces, klass->ifaces, 2 * sizeof(AZIFEntry));
				klass->ifaces_self = ifaces;
			}
			AZIFEntry *ifaces;
			if ((klass->n_ifaces_self == 1) && (klass->n_ifaces_all <= 2)) {
				ifaces = klass->ifaces;
			} else {
				static unsigned int n_allocations = 0, allocated = 0;
				klass->ifaces_all = (AZIFEntry *) malloc(klass->n_ifaces_all * sizeof(AZIFEntry));
				n_allocations += 1;
				allocated += klass->n_ifaces_all;
				fprintf(stderr, "az_class_post_init(): Allocated all %u (%u %u)\n", klass->n_ifaces_all, n_allocations, allocated);
				ifaces = klass->ifaces_all;
			}
			unsigned int idx = 0;
			for (i = 0; i < klass->n_ifaces_self; i++) {
				ifaces[idx] = *az_class_iface_self(klass, i);
				AZClass *iface_class = AZ_CLASS_FROM_TYPE(az_class_iface_self(klass, i)->type);
				idx += 1;
				memcpy(&ifaces[idx], az_class_iface_all(iface_class, 0), iface_class->n_ifaces_all * sizeof (AZIFEntry *));
				idx += iface_class->n_ifaces_all;
			}
			memcpy (&ifaces[idx], az_class_iface_all(klass->parent, 0), klass->parent->n_ifaces_all * sizeof (AZIFEntry *));
		}
	}
	if (klass->n_ifaces_all) {
		fprintf (stderr, "Class %s\n", klass->name);
		fprintf (stderr, "  Self %u\n", klass->n_ifaces_self);
		for (uint16_t i = 0; i < klass->n_ifaces_self; i++) {
			const AZIFEntry *ifentry = az_class_iface_self(klass, i);
			fprintf (stderr, "    %d: type %d\n", i, ifentry->type);
		}
		fprintf (stderr, "  All %u\n", klass->n_ifaces_all);
		for (uint16_t i = 0; i < klass->n_ifaces_all; i++) {
			const AZIFEntry *ifentry = az_class_iface_all(klass, i);
			fprintf (stderr, "    %d: type %d\n", i, ifentry->type);
		}
	}
	/* Init recursively */
	az_class_init_recursive (klass, klass);
}

#ifdef AZ_HAS_PROPERTIES
void
az_class_set_num_properties (AZClass *klass, unsigned int nproperties)
{
	klass->n_properties_self = nproperties;
	klass->properties_self = (AZField *) malloc (nproperties * sizeof (AZField));
	memset (klass->properties_self, 0, nproperties * sizeof (AZField));
}

void az_class_define_property (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset,
	const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_properties_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
	arikkei_return_if_fail (!impl || (az_type_is_assignable_to (impl->type, type)));
#endif
	az_field_setup (klass->properties_self + idx, key, type, is_final, spec, read, write, offset, impl, inst);
}

void
az_class_define_property_function_val (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write,
	const AZFunctionSignature *sig, const AZImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_properties_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_function (klass->properties_self + idx, key, is_final, spec, read, write, sig, impl, inst);
}

void
az_class_define_property_function_packed (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int is_final, unsigned int spec, unsigned int read, unsigned int write, unsigned int offset, const AZFunctionSignature *sig)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_properties_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (!((write != AZ_FIELD_WRITE_NONE) && is_final));
#endif
	az_field_setup_function_packed (klass->properties_self + idx, key, is_final, spec, read, write, sig, offset);
}

void
az_class_property_setup (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int type,
unsigned int is_static, unsigned int can_read, unsigned int can_write, unsigned int is_final, unsigned int is_value,
unsigned int value_type, void *inst)
{
	unsigned int spec, read, write;
	AZImplementation *impl;
	arikkei_return_if_fail (klass != NULL);
	arikkei_return_if_fail (idx < klass->n_properties_self);
	arikkei_return_if_fail (key != NULL);
	arikkei_return_if_fail (type != AZ_TYPE_NONE);
	arikkei_return_if_fail (!(can_write && is_final));
	arikkei_return_if_fail (!is_value || is_static);
	if (!((value_type == AZ_TYPE_NONE) || (az_type_is_assignable_to (value_type, type)))) {
		return;
	}
	arikkei_return_if_fail ((value_type == AZ_TYPE_NONE) || (az_type_is_assignable_to (value_type, type)));
	spec = (is_static) ? AZ_FIELD_CLASS : AZ_FIELD_INSTANCE;
	if (!can_read) {
		read = AZ_FIELD_READ_NONE;
	} else if (is_value) {
		read = AZ_FIELD_READ_STORED_STATIC;
	} else {
		read = AZ_FIELD_READ_METHOD;
	}
	write = (can_write) ? AZ_FIELD_WRITE_METHOD : AZ_FIELD_WRITE_NONE;
	impl = (value_type != AZ_TYPE_NONE) ? (AZImplementation *) az_type_get_class (value_type) : NULL;
	az_field_setup (klass->properties_self + idx, key, type, is_final, spec, read, write, 0, impl, inst);
}

void
az_class_define_method (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *))
{
	AZFunctionSignature *sig;
	AZFunctionValue fval;
	sig = az_function_signature_new (klass->implementation.type, ret_type, n_args, arg_types);
	az_function_value_setup (&fval, sig, invoke);
	az_class_define_property_function_val (klass, idx, key, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_STORED_STATIC, AZ_FIELD_WRITE_NONE, sig,
		(AZImplementation *) az_type_get_class (AZ_TYPE_FUNCTION_VALUE), &fval);
}

void
az_class_define_method_va(AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail(n_args < 64);
	va_start(ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg(ap, unsigned int);
	}
	va_end(ap);
	az_class_define_method(klass, idx, key, ret_type, n_args, arg_types, invoke);
}

void
az_class_define_static_method (AZClass *klass, unsigned int idx, const unsigned char *key, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[],
unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *))
{
	AZFunctionSignature *sig;
	AZFunctionValue fval;
	sig = az_function_signature_new (AZ_TYPE_NONE, ret_type, n_args, arg_types);
	az_function_value_setup (&fval, sig, invoke);
	az_class_define_property_function_val (klass, idx, key, 1, AZ_FIELD_CLASS, AZ_FIELD_READ_STORED_STATIC, AZ_FIELD_WRITE_NONE, sig,
		(AZImplementation *) az_type_get_class (AZ_TYPE_FUNCTION_VALUE), &fval);
}

void az_class_define_static_method_va (AZClass *klass, unsigned int idx, const unsigned char *key,
	unsigned int (*invoke) (const AZImplementation **, const AZValue **, const AZImplementation **, AZValue64 *, AZContext *),
	unsigned int ret_type, unsigned int n_args, ...)
{
	va_list ap;
	unsigned int arg_types[64], i;
	arikkei_return_if_fail (n_args < 64);

	//uint64_t *p = (uint64_t *) &n_args + 1;

	va_start (ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg (ap, unsigned int);
		//fprintf (stderr, "%u %llu\n", arg_types[i], p[i] & 0xffffffff);
	}
	va_end (ap);
	az_class_define_static_method (klass, idx, key, ret_type, n_args, arg_types, invoke);
}

#endif
