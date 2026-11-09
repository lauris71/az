#ifndef __AZ_VALUE_STORE_H__
#define __AZ_VALUE_STORE_H__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2026
*/

typedef struct _AZStoreElement AZStoreElement;

#include <stdint.h>
#include <string.h>

#include <az/az.h>
#include <az/class.h>
#include <az/complex.h>
#include <az/reference.h>
#include <az/value.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A compact storage element for containers.
 * 
 * A datastore consists of an array of AZStoreElements and data buffer.
 * Values up to 8 bytes are stored directly in store elements, larger values as
 * indices into the data buffer.
 * The data buffer has granularity of 16 bytes and must be aligned to 16 bytes.
 */
struct _AZStoreElement {
    const AZImplementation *impl;
    union {
        uint8_t val[8];
        uint32_t val_idx;
        /* Convenience */
		uint32_t boolean_v;
		int8_t int8_v;
		uint8_t uint8_v;
		int16_t int16_v;
		uint16_t uint16_v;
		int32_t int32_v;
		uint32_t uint32_v;
		int64_t int64_v;
		uint64_t uint64_v;
		float float_v;
		double double_v;
		AZComplexFloat cfloat_v;
		void *pointer_v;
		void *block;
		AZReference *reference;
		AZString *string;
		AZObject *object;
    };
};

static inline unsigned int
az_value_store_block_size(const AZImplementation *impl)
{
    if (!impl) return 0;
    AZClass *klass = AZ_CLASS_FROM_IMPL(impl);
    if (AZ_CLASS_IS_BLOCK(klass)|| (klass->instance_size <= 8)) return 0;
    return ((klass->instance_size + 15) / 16);
}

static inline unsigned int
az_value_store_block_size_at(const AZStoreElement elements[], unsigned int idx)
{
    return az_value_store_block_size(elements[idx].impl);
}

static inline void *
az_value_store_instance_at(AZStoreElement elements[], unsigned int idx, AZValue *data)
{
    if (!elements[idx].impl) return NULL;
    AZClass *klass = AZ_CLASS_FROM_IMPL(elements[idx].impl);
    if (AZ_CLASS_IS_BLOCK(klass)) {
        return elements[idx].block;
    } else if (klass->instance_size <= 8) {
        return &elements[idx].val;
    }
    return (data + elements[idx].val_idx);
}

const AZImplementation *az_value_store_instance_at_autobox(AZStoreElement elements[], unsigned int idx, AZValue *data, void **inst);

static inline AZValue *
az_value_store_value_at(const AZStoreElement elements[], unsigned int idx, AZValue *data)
{
    AZClass *klass = AZ_CLASS_FROM_IMPL(elements[idx].impl);
    if (AZ_CLASS_IS_BLOCK(klass) || (klass->instance_size <= 8)) return (AZValue *) &elements[idx].val;
    return data + elements[idx].val_idx;
}

/**
 * @brief Clears value at datastore position
 * 
 * If the element was a large value, the number of data slots occupied the element is returned.
 * The impl is set to NULL but data_idx remains valid after the clear operation.
 * Datablock is not needed because only values can use it and values do not need destruction.
 * 
 * @param elements The datastore elements array
 * @param idx The index of the element to clear
 * @return The number of data slots that were freed
 */
static inline unsigned int
az_value_store_clear_at(AZStoreElement elements[], unsigned int idx)
{
    if (!elements[idx].impl) return 0;
    AZClass *klass = AZ_CLASS_FROM_IMPL(elements[idx].impl);
    if (AZ_IMPL_IS_REFERENCE(elements[idx].impl)) {
        az_reference_unref((AZReferenceClass *) elements[idx].impl, elements[idx].reference);
        elements[idx].impl = NULL;
        return 0;
    }
    elements[idx].impl = NULL;
    if (AZ_CLASS_IS_BLOCK(klass) || (klass->instance_size <= 8)) return 0;
    return ((klass->instance_size + 15) / 16);
}

/**
 * @brief Set uninitialized datastore element
 * 
 * Stores an instance of the given implementation to the uninitalized space in the datastore.
 * For large values, the data_pos has to be set up to point 16-byte aligned free region in the data buffer.
 * It is the job of the caller to ensure, that there is enough free space to store the value.
 * 
 * @param elements The datastore elements array
 * @param idx The index of the element to set
 * @param data The data buffer
 * @param data_pos The current data position in the buffer
 * @param impl The implementation type
 * @param inst The instance data to store
 * @return The next data position after the element
 */
unsigned int az_value_store_set(AZStoreElement elements[], unsigned int idx, AZValue *data, unsigned int data_pos, const AZImplementation *impl, void *inst);
/**
 * @brief Set uniniatialized datastore element, transferring ownership
 * 
 * Transfers an instance of the given implementation to the uninitalized space in the datastore.
 * For large values, the data_pos has to be set up to point 16-byte aligned free region in the data buffer.
 * It is the job of the caller to ensure, that there is enough free space to store the value.
 * 
 * @param elements The datastore elements array
 * @param idx The index of the element to set
 * @param data The data buffer
 * @param data_pos The current data position in the buffer
 * @param impl The implementation type
 * @param inst The instance data to store
 * @return The next data position after the element
 */
unsigned int az_value_store_transfer_in(AZStoreElement elements[], unsigned int idx, AZValue *data, unsigned int data_pos, const AZImplementation *impl, void *inst);

static inline unsigned int
az_value_store_replace(AZStoreElement elements[], unsigned int idx, AZValue *data, unsigned int data_pos, const AZImplementation *impl, void *inst)
{
    if (elements[idx].impl && AZ_IMPL_IS_REFERENCE(elements[idx].impl)) {
        az_reference_unref((AZReferenceClass *) elements[idx].impl, elements[idx].reference);
    }
    return az_value_store_set(elements, idx, data, data_pos, impl, inst);
}

#ifdef __cplusplus
};
#endif

#endif
