#define __AZ_DATASTORE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

#include <az/value-store.h>

_Static_assert (sizeof(AZStoreElement) == 16, "AZStoreElement size is not 16");

const AZImplementation *
az_value_store_instance_at_autobox(AZStoreElement elements[], unsigned int idx, AZValue *data, void **inst)
{
    if (!elements[idx].impl) {
        *inst = NULL;
        return NULL;
    }
    AZClass *klass = AZ_CLASS_FROM_IMPL(elements[idx].impl);
    if (AZ_CLASS_IS_BLOCK(klass)) {
        return az_value_get_inst_autobox(elements[idx].impl, (const AZValue *) elements[idx].val, inst);
    } else if (klass->instance_size <= 8) {
        *inst = (void *) &elements[idx].val;
        return elements[idx].impl;
    }
    *inst = (void *) (data + elements[idx].val_idx);
    return elements[idx].impl;
}

unsigned int
az_value_store_set(AZStoreElement elements[], unsigned int idx, AZValue *data, unsigned int data_pos, const AZImplementation *impl, void *inst)
{
    elements[idx].impl = impl;
    if (impl) {
        AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
        if (AZ_CLASS_IS_BLOCK(klass)) {
            elements[idx].block = inst;
            if (AZ_CLASS_IS_REFERENCE(klass)) {
                az_reference_ref((AZReference *) inst);
            }
        } else if (klass->instance_size <= 8) {
            memcpy(elements[idx].val, inst, klass->instance_size);
        } else {
            elements[idx].val_idx = data_pos;
            memcpy(data + data_pos, inst, klass->instance_size);
            data_pos += (klass->instance_size + 15) / 16;
        }
    }
    return data_pos;
}

unsigned int
az_value_store_transfer_in(AZStoreElement elements[], unsigned int idx, AZValue *data, unsigned int data_pos, const AZImplementation *impl, void *inst)
{
    elements[idx].impl = impl;
    if (impl) {
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
        if (AZ_CLASS_IS_BLOCK(klass)) {
            elements[idx].block = inst;
        } else if (klass->instance_size <= 8) {
            memcpy(elements[idx].val, inst, klass->instance_size);
        } else {
            elements[idx].val_idx = data_pos;
            memcpy(data + data_pos, inst, klass->instance_size);
            data_pos += (klass->instance_size + 15) / 16;
        }
    }
    return data_pos;
}
