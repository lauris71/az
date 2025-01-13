#ifndef __AZ_OBJECT_H__
#define __AZ_OBJECT_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016
*/

typedef struct _AZObjectClass AZObjectClass;

#define AZ_TYPE_OBJECT (az_object_get_type ())
#define AZ_OBJECT(o) (AZ_CHECK_INSTANCE_CAST ((o), AZ_TYPE_OBJECT, AZObject))
#define AZ_IS_OBJECT(o) (AZ_CHECK_INSTANCE_TYPE ((o), AZ_TYPE_OBJECT))

#include <stdlib.h>

#include <az/reference.h>
#ifdef AZ_HAS_PACKED_VALUE
#include <az/packed-value.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Indicates that object is set up and not yet disposed */
#define AZ_OBJECT_ALIVE 1
#define AZ_OBJECT_NUM_FLAGS 1

struct _AZObject {
	AZReference reference;
	/* As we want header to be 64 bits we can as well have flags */
	unsigned int flags;
	AZObjectClass *klass;
};

struct _AZObjectClass {
	AZReferenceClass reference_klass;
	/* Subclasses should not override dispose but use lifecycle-obeying shutdown instead */
	/* Frontend to dispose that is called only once per lifecycle */
	void (*shutdown) (AZObject *obj);
};

unsigned int az_object_get_type (void);

ARIKKEI_INLINE void
az_object_ref (AZObject *obj)
{
	az_reference_ref (&obj->reference);
}

ARIKKEI_INLINE void
az_object_unref (AZObject *obj)
{
	az_reference_unref (&obj->klass->reference_klass, &obj->reference);
}

void az_object_shutdown (AZObject *obj);

AZObject *az_object_new (unsigned int type);

#ifndef AZ_DISABLE_CAST_CHECKS
#define AZ_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) az_object_check_instance_cast (ip, tc))
#else
#define AZ_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) ip)
#endif

#define AZ_CHECK_INSTANCE_TYPE(ip, tc) az_object_check_instance_type (ip, tc)

void *az_object_check_instance_cast (void *inst, unsigned int type);
unsigned int az_object_check_instance_type (void *inst, unsigned int type);

#define az_object_is_a(obj, type) az_object_check_instance_type (obj, type)
unsigned int az_object_implements (AZObject *obj, unsigned int type);

/* Convenience frontend to az_get_interface */
const AZImplementation *az_object_get_interface (AZObject *obj, unsigned int type, void **inst);

ARIKKEI_INLINE void
az_object_set_flags (AZObject* obj, unsigned int flags)
{
	obj->flags |= flags;
}

ARIKKEI_INLINE void
az_object_clear_flags (AZObject *obj, unsigned int flags)
{
	obj->flags &= ~flags;
}

ARIKKEI_INLINE unsigned int
az_object_flags (AZObject *obj, unsigned int flags)
{
	return obj->flags & flags;
}

ARIKKEI_INLINE unsigned int
az_object_is_alive (AZObject *obj) {
	return obj->flags & AZ_OBJECT_ALIVE;
}

#ifdef AZ_HAS_PACKED_VALUE
void az_packed_value_set_object (AZPackedValue *val, AZObject *obj);
void az_packed_value_transfer_object (AZPackedValue *val, AZObject *obj);
/* Convenience method for setting value to given object */
ARIKKEI_INLINE void
az_value_set_object (const AZImplementation **impl, AZValue *val, AZObject *obj)
{
	if (obj) {
		*impl = (AZImplementation *) obj->klass;
		val->reference = (AZReference *) obj;
		az_object_ref (obj);
	} else {
		*impl = NULL;
	}
}
ARIKKEI_INLINE void
az_value_transfer_object (const AZImplementation **impl, AZValue *val, AZObject *obj)
{
	if (obj) {
		*impl = (AZImplementation *) obj->klass;
		val->reference = (AZReference *) obj;
	} else {
		*impl = NULL;
	}
}
#endif

#ifdef __cplusplus
};
#endif

#endif
