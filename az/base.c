#define __AZ_BASE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

#include <arikkei/arikkei-strlib.h>

#include <az/base.h>
#include <az/class.h>
#include <az/private.h>
#include <az/value.h>
#include <az/field.h>
#include <az/extend.h>

#include <az/collections/list.h>

static void az_init_base_classes (void);
static void az_construct_base_classes (void);

void
az_init (void)
{
	static unsigned int initialized = 0;
	if (initialized) return;
	initialized = 1;
	az_globals_init();
	/* AZ_TYPE_ANY - AZ_YPE_POINTER (1-15) */
	az_init_primitive_classes();
	/* AZ_TYPE_STRUCT - AZ_TYPE_CLASS (16-19) */
	az_init_base_classes();
	/* AZ_TYPE_INTERFACE (20) */
	az_init_interface_class();
	/* AZ_TYPE_FIELD (21) */
	az_init_field_class();
	/* AZ_TYPE_FUNCTION_SIGNATURE, AZ_TYPE_FUNCTION (22,23) */
	az_init_function_classes();
	/* AZ_TYPE_REFERENCE (24) */
	az_init_reference_class();
	/* AZ_TYPE_STRING (25) */
	az_init_string_class();
	/* AZ_TYPE_BOXED_VALUE (26) */
	az_init_boxed_value_class();
	/* AZ_TYPE_BOXED_INTERFACE (27) */
	az_init_boxed_interface_class();
	/* AZ_TYPE_PACKED_VALUE (28) */
	az_init_packed_value_class();
	/* AZ_TYPE_OBJECT (29) */
	az_init_object_class();
	/* AZ_TYPE_OUTPUT_STREAM (30) */
	az_init_output_stream_class();
	/* AZ_TYPE_INPUT_STREAM (31) */
	az_init_input_stream_class();

	/* These need separate construct phase because string and function types are not yet defined during their creation */
	az_construct_primitive_classes();
	az_construct_base_classes();
	az_construct_field_class();

	/* Post init everything, i.e. sanity checks, interface lists and construction flags */
	for (unsigned int i = AZ_TYPE_IDX_ANY; i < AZ_NUM_BASE_TYPES; i++) {
		az_class_post_init(AZ_CLASS_FROM_TYPE(i));
	}
}

enum {
	IMPL_FUNC_SETSTATICPROPERTY,
	IMPL_FUNC_GETSTATICPROPERTY,
	IMPL_NUM_PROPERTIES
};

enum {
    CLASS_PROP_PARENT,
	CLASS_PROP_PROPERTIES,
    CLASS_NUM_PROPERTIES
};

static unsigned int
impl_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZImplementation *inst_impl = (AZImplementation *) inst;
	AZClass *inst_class = AZ_CLASS_FROM_IMPL(inst_impl);
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, inst_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " implementation");
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

static unsigned int
class_to_string (const AZImplementation* impl, void *inst, unsigned char *buf, unsigned int len)
{
	AZClass *inst_class = (AZClass *) inst;
	unsigned int pos;
	/* Nothing is written when destination is NULL */
	if (!buf) len = 0;
	pos = arikkei_memcpy_str (buf, len, inst_class->name);
	pos += arikkei_memcpy_str (buf + pos, (len > pos) ? len - pos : 0, (const unsigned char *) " class");
	if (buf && (pos < len)) buf[pos] = 0;
	return pos;
}

static unsigned int class_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx);

AZ_CLASS_ALIGN AZClass AZStructKlass = {
	.impl = { .flags = AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_STRUCT },
	.parent = &AZAnyKlass,
	.name = (const uint8_t *) "struct",
	.alignment = 3,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = az_any_to_string
};

AZ_CLASS_ALIGN AZClass AZBlockKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_BLOCK },
	.parent = &AZAnyKlass,
	.name = (const uint8_t *) "block",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = az_any_to_string
};

AZ_CLASS_ALIGN AZClass AZImplKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_IMPLEMENTATION },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "implementation",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = sizeof(AZImplementation),
	.n_props_self = IMPL_NUM_PROPERTIES,
	.to_string = impl_to_string
};

AZ_CLASS_ALIGN AZClass AZClassKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_CLASS },
	.parent = &AZImplKlass,
	.name = (const uint8_t *) "class",
	/* Class is variable size */
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.n_props_self = CLASS_NUM_PROPERTIES,
	.get_property = class_get_property,
	.to_string = class_to_string
};

void
az_init_base_classes (void)
{
	az_class_new_with_value(&AZStructKlass);
	az_class_new_with_value(&AZBlockKlass);
	az_class_new_with_value(&AZImplKlass);
	az_class_new_with_value(&AZClassKlass);
}

static unsigned int
impl_call_setStaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *def_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &def_class, &sub_impl, &sub_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &def_class->props_self[prop_idx];
	arikkei_return_val_if_fail (AZ_FIELD_SPEC(prop) == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (!AZ_FIELD_IS_FINAL(prop), 0);
	arikkei_return_val_if_fail (AZ_FIELD_WRITE(prop) != AZ_FIELD_WRITE_NONE, 0);
	return !az_instance_set_property_by_id (def_class, AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, NULL, prop_idx, arg_impls[2], az_value_get_inst(arg_impls[2], arg_vals[2]));
}

static unsigned int
impl_call_getstaticProperty (const AZImplementation **arg_impls, const AZValue **arg_vals, const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
	int prop_idx;
	AZImplementation *impl = (AZImplementation *) arg_vals[0]->block;
	AZString *key = arg_vals[1]->string;
	const AZClass *def_class;
	const AZImplementation *sub_impl;
	void *sub_inst;
	prop_idx = az_class_lookup_property (AZ_CLASS_FROM_IMPL(impl), impl, NULL, key, &def_class, &sub_impl, &sub_inst);
	arikkei_return_val_if_fail (prop_idx >= 0, 0);
	AZField *prop = &def_class->props_self[prop_idx];
	arikkei_return_val_if_fail (AZ_FIELD_SPEC(prop) == AZ_FIELD_CLASS, 0);
	arikkei_return_val_if_fail (AZ_FIELD_READ(prop) != AZ_FIELD_READ_NONE, 0);
	return !az_instance_get_property_by_id (def_class, AZ_CLASS_FROM_IMPL(sub_impl), sub_impl, NULL, prop_idx, ret_impl, &ret_val->value, 64);
}

AZListImplementation prop_list_impl;

static unsigned int
prop_list_get_element_type (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	return AZ_TYPE_FIELD;
}

static uint64_t
prop_list_get_size(const AZCollectionImplementation *coll_impl, AZCollection *coll_inst)
{
	AZClass *klass = (AZClass *) coll_inst;
	return klass->n_props_self;

}

static unsigned int
prop_list_contains (const AZCollectionImplementation *coll_impl, AZCollection *coll_inst, const AZImplementation *val_impl, const void *val_inst)
{
	if (val_impl != &AZFieldKlass.impl) return 0;
	AZClass *klass = (AZClass *) coll_inst;
	for (uint16_t i = 0; i < klass->n_props_self; i++) {
		if (&klass->props_self[i] == val_inst) return 1;
	}
	return 0;
}

static const AZImplementation *
prop_list_get_element (const AZListImplementation *list_impl, void *list_inst, unsigned int idx, AZValue *val, unsigned int size)
{
	AZClass *klass = (AZClass *) list_inst;
	if (idx >= klass->n_props_self) return NULL;
	val->block = &klass->props_self[idx];
	return &AZFieldKlass.impl;
}

static void
az_construct_base_classes (void)
{
	az_implementation_init_by_type(&prop_list_impl.collection_impl.impl, AZ_TYPE_LIST);
	prop_list_impl.collection_impl.get_element_type = prop_list_get_element_type;
	prop_list_impl.collection_impl.get_size = prop_list_get_size;
	prop_list_impl.collection_impl.contains = prop_list_contains;
	prop_list_impl.get_element = prop_list_get_element;
	/* Implementation */
	az_class_define_method_va (&AZImplKlass, IMPL_FUNC_SETSTATICPROPERTY, (const unsigned char *) "setStaticProperty", impl_call_setStaticProperty, AZ_TYPE_NONE, 2, AZ_TYPE_STRING, AZ_TYPE_ANY);
	az_class_define_method_va (&AZImplKlass, IMPL_FUNC_GETSTATICPROPERTY, (const unsigned char *) "getStaticProperty", impl_call_getstaticProperty, AZ_TYPE_ANY, 1, AZ_TYPE_STRING);
	/* Class */
	az_class_define_property (&AZClassKlass, CLASS_PROP_PARENT, (const uint8_t *) "parent", AZ_TYPE_CLASS, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZClass, parent), NULL, NULL);
	az_class_define_property (&AZClassKlass, CLASS_PROP_PROPERTIES, (const uint8_t *) "properties", AZ_TYPE_LIST, 1, AZ_FIELD_CLASS, AZ_FIELD_READ_METHOD, 0, 0, NULL, NULL);
}

static unsigned int
class_get_property (const AZImplementation *impl, void *inst, unsigned int idx, const AZImplementation **prop_impl, AZValue *prop_val, AZContext *ctx)
{
	AZClass *klass = (AZClass *) inst;
	switch (idx) {
		case CLASS_PROP_PROPERTIES:
			*prop_impl = &prop_list_impl.collection_impl.impl;
			prop_val->block = klass;
			return 1;
		default:
			return 0;
	}
}
