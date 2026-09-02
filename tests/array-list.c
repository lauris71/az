#define __AZ_ARRAY_LIST_TEST_C__

#include <inttypes.h>
#include <string.h>

#include <az/instance.h>
#include <az/extend.h>
#include <az/base.h>
#include <az/collections/array-list.h>
#include <az/value.h>
#include <az/boxed-value.h>

#include "unity/unity.h"

static void
verify_list(AZArrayList *alist, const unsigned int idx[], const unsigned int types[])
{
    void *coll_inst;
    const AZCollectionImplementation *coll_impl = (AZCollectionImplementation *) az_instance_get_interface((AZImplementation *) AZArrayListKlass, alist, AZ_TYPE_COLLECTION, &coll_inst);
    TEST_ASSERT(coll_impl == &AZArrayListKlass->list_impl.collection_impl);
    TEST_ASSERT(coll_inst == alist);
    unsigned int size = az_collection_get_size(coll_impl, coll_inst);
    TEST_ASSERT(size == alist->list.collection.size);
    for (unsigned int i = 0; i < size; i++) {
        uint8_t buf[256];
        memset(buf, (char) idx[i], 256);
        AZClass *klass = AZ_CLASS_FROM_TYPE(types[idx[i]]);
        AZValue val;
        const AZImplementation *impl = az_list_get_element(&AZArrayListKlass->list_impl, alist, i, &val, 16);
        if (klass->instance_size <= 16) {
            TEST_ASSERT(impl == &klass->impl);
            if (!klass->instance_size) continue;
            TEST_ASSERT(az_value_equals(&klass->impl, &val, (const AZValue *) buf));
        } else {
            TEST_ASSERT(impl == &AZBoxedValueKlass.klass.impl);
            AZBoxedValue *boxed = (AZBoxedValue *) val.block;
            TEST_ASSERT(boxed->klass == klass);
            TEST_ASSERT(az_value_equals(&boxed->klass->impl, &boxed->val, (const AZValue *) buf));
        }
    }
}

static void
print_list(AZArrayList *alist, FILE *ofs)
{
    fprintf(stdout, "List [val_size=%d length=%" PRIu64 "]:", alist->val_size, alist->list.collection.size);
    for (unsigned int i = 0; i < alist->list.collection.size; i++) {
        AZArrayListEntry *entry = az_array_list_get_entry(alist, i);
        fprintf (stdout, " %d", (entry->impl) ? AZ_IMPL_TYPE(entry->impl) : 0);
    }
    fprintf(stdout, "\n");
}

void
test_array_list()
{
    unsigned int types[10];
    az_init();
    for (unsigned int i = 0; i < 10; i++) {
        unsigned int instance_size = 4 * i;
        uint8_t name[32];
        snprintf((char *) name, 32, "struct_%d", instance_size);
        AZClass *klass = az_register_type(&types[i], name, AZ_TYPE_STRUCT, sizeof(AZClass), instance_size, AZ_FLAG_FINAL, 0, 0, NULL, NULL, NULL);
        TEST_ASSERT(klass->instance_size == instance_size);
    }
    AZArrayList *alist = az_array_list_new(AZ_TYPE_ANY, 16);
    for (unsigned int i = 0; i < 10; i++) {
        uint32_t inst = i;
        TEST_ASSERT(az_array_list_append(alist, &AZUint32Klass.impl, &inst));
    }
    for (unsigned int i = 0; i < 10; i++) {
        AZValue val;
        const AZImplementation *impl = az_list_get_element(&AZArrayListKlass->list_impl, alist, i, &val, 16);
        TEST_ASSERT(impl == &AZUint32Klass.impl);
        TEST_ASSERT(val.uint32_v == i);
    }
    uint8_t buf[256] = {0};
    /* Try lists with element size 16...64 */
    for (unsigned int s = 16; s <= 64; s = s << 1) {
        /* Create new list with value_size s */
        alist = az_array_list_new(AZ_TYPE_ANY, s);
        /* Fill it with objects 0..9 */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) i, 256);
            TEST_ASSERT(az_array_list_append(alist, AZ_IMPL_FROM_TYPE(types[i]), &buf));
        }
        //print_list(alist, stdout);
        /* Verify */
        unsigned int *idx = (unsigned int[]) {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        verify_list(alist, idx, types);
        /* Insert 9..0 into position 5 */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) i, 256);
            TEST_ASSERT(az_array_list_insert(alist, 5, AZ_IMPL_FROM_TYPE(types[i]), &buf));
        }
        //print_list(alist, stdout);
        /* Verify */
        idx = (unsigned int[]) {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 5, 6, 7, 8, 9};
        verify_list(alist, idx, types);
        /* Delete positions 1,3,5...*/
        for (unsigned int i = 0; i < 10; i++) {
            az_array_list_remove(alist, i + 1);
        }
        //print_list(alist, stdout);
        idx = (unsigned int[]) {0, 2, 4, 8, 6, 4, 2, 0, 6, 8};
        verify_list(alist, idx, types);
        /* Replace */
        for (unsigned int i = 0; i < 10; i++) {
            memset(buf, (char) (9 - i), 256);
            az_array_list_set_element(alist, i, AZ_IMPL_FROM_TYPE(types[9 - i]), &buf);
        }
        //print_list(alist, stdout);
        idx = (unsigned int[]) {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        verify_list(alist, idx, types);
        az_array_list_delete(alist);
    }
}
