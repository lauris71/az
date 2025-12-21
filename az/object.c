#define __AZ_OBJECT_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2025
*/

#include <stdio.h>

#include <az/object.h>
#include <az/packed-value.h>
#include <az/private.h>
#include <az/extend.h>

/* AZInstance implementation */
static void object_init (AZObjectClass *klass, AZObject *object);
/* AZReference implementation */
void object_dispose (AZReferenceClass *klass, AZReference *ref);

enum {
	/* Functions */
	NUM_FUNCTIONS,
	/* Values */
	PROP_CLASS = NUM_FUNCTIONS,
	NUM_PROPERTIES
};

AZObjectClass AZObjectKlass = {
	{{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_REFERENCE | AZ_FLAG_ZERO_MEMORY | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_OBJECT},
			&AZReferenceKlass.klass,
			0, 0, 0, 0,
			/* ifaces / ifaces_self, ifaces_all */
			{0},
			/* props_self */
			NULL,
			(const uint8_t *) "object",
			7, sizeof(AZObjectClass), 0,
			NULL,
			/* instance_init, instance_finalize */
			(void (*) (const AZImplementation *, void *)) object_init, NULL,
			/* serialize, deserialize, to_string */
			NULL, NULL, NULL,
			/* get_property, set_property */
			NULL, NULL},
		/* drop, dispose */
		NULL, object_dispose},
	/* shutdown */
	NULL
};

void
az_init_object_class (void)
{
	az_class_new_with_value(&AZObjectKlass.reference_klass.klass);
	az_class_set_num_properties (&AZObjectKlass.reference_klass.klass, NUM_PROPERTIES);
	az_class_define_property (&AZObjectKlass.reference_klass.klass, PROP_CLASS, (const unsigned char *) "class", AZ_TYPE_CLASS, 1, AZ_FIELD_INSTANCE, AZ_FIELD_READ_VALUE, 0, ARIKKEI_OFFSET (AZObject, klass), NULL, NULL);
}

static void
object_init (AZObjectClass *klass, AZObject *object)
{
	object->flags |= AZ_OBJECT_ALIVE;
}

void
object_dispose (AZReferenceClass *klass, AZReference *ref)
{
	AZObject *obj = AZ_OBJECT (ref);
	if (obj->flags & AZ_OBJECT_ALIVE) {
		if (obj->klass->shutdown) {
			obj->klass->shutdown (obj);
		}
	}
}

AZObject *
az_object_new (unsigned int type)
{
	AZObject *object;
	arikkei_return_val_if_fail (az_type_is_a (type, AZ_TYPE_OBJECT), NULL);
	object = (AZObject *) az_instance_new (type);
	object->klass = (AZObjectClass *) az_type_get_class (type);
	return object;
}

void
az_object_shutdown (AZObject *obj)
{
	arikkei_return_if_fail (obj != NULL);
	arikkei_return_if_fail (AZ_IS_OBJECT (obj));
	arikkei_return_if_fail (obj->flags & AZ_OBJECT_ALIVE);
	obj->flags &= ~AZ_OBJECT_ALIVE;
	if (obj->klass->shutdown) {
		obj->klass->shutdown (obj);
	}
	az_reference_unref (&obj->klass->reference_klass, (AZReference *) obj);
}

void *
az_object_check_instance_cast (void *inst, unsigned int type)
{
	if (inst == NULL) {
		fprintf (stderr, "az_object_check_instance_cast: inst == NULL\n");
		return NULL;
	}
	if (!az_type_is_a(AZ_OBJECT_TYPE(inst), type)) {
		AZClass *klass = az_type_get_class (type);
		fprintf (stderr, "az_object_check_instance_cast: %s is not %s\n", ((AZObject *) inst)->klass->reference_klass.klass.name, klass->name);
		return NULL;
	}
	return inst;
}

unsigned int
az_object_check_instance_type (void *inst, unsigned int type)
{
	if (inst == NULL) return 0;
	return az_type_is_a(AZ_OBJECT_TYPE(inst), type);
}

unsigned int
az_object_implements (AZObject *obj, unsigned int type)
{
	arikkei_return_val_if_fail (obj != NULL, 0);
	arikkei_return_val_if_fail (AZ_IS_OBJECT (obj), 0);
	return az_type_implements(AZ_OBJECT_TYPE(obj), type);
}

const AZImplementation *
az_object_get_interface (AZObject *obj, unsigned int type, void **inst)
{
	arikkei_return_val_if_fail (obj != NULL, NULL);
	arikkei_return_val_if_fail (AZ_IS_OBJECT (obj), NULL);
	return az_get_interface (&obj->klass->reference_klass.klass.impl, obj, type, inst);
}

#ifdef AZ_HAS_PACKED_VALUE
void
az_packed_value_set_object (AZPackedValue *val, AZObject *obj)
{
	if (!obj) {
		az_packed_value_set_reference (val, AZ_TYPE_OBJECT, NULL);
	} else {
		az_packed_value_set_reference(val, AZ_OBJECT_TYPE(obj), (AZReference *) obj);
	}
}

void
az_packed_value_transfer_object (AZPackedValue *val, AZObject *obj)
{
	if (!obj) {
		az_packed_value_set_reference (val, AZ_TYPE_OBJECT, NULL);
	} else {
		az_packed_value_transfer_reference (val, AZ_OBJECT_TYPE(obj), (AZReference *) obj);
	}
}

#endif
