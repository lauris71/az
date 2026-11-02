#define __AZ_TESTS_INTERFACE_C__

#include <az/az.h>
#include <az/interface.h>
#include <az/extend.h>
#include <az/instance.h>
#include <az/boxed-interface.h>

#include "unity/unity.h"

/*
 * Test interface derivation
 *
 * The structure
 * 
 * IF_123
 *   IF_456
 * 
 * STRUCT_12
 *   IF_123
 *   IF_789
 * 
 * BLOCK_34
 *   IF_123
 *   IF_789
 */
#define TYPE_STRUCT_12 struct_12_get_type()
#define TYPE_BLOCK_34 block_34_get_type()
#define TYPE_IF_123 if_123_get_type()
#define TYPE_IF_456 if_456_get_type()
#define TYPE_IF_789 if_789_get_type()

typedef struct {
    unsigned int val;
} If789;

typedef struct {
    AZImplementation impl;
    unsigned int val;
} If789Impl;

static void
if_789_impl_init(If789Impl *impl)
{
    impl->val = 1789;
}

unsigned int
if_789_get_type()
{
    static unsigned int type = 0;
    if (!type) {
        az_register_interface_type(&type, (const uint8_t *) "IF_789", AZ_TYPE_INTERFACE, sizeof(AZInterfaceClass), sizeof(If789Impl), sizeof(If789),
        0, 0, 0,
        NULL, (void (*) (AZImplementation *)) if_789_impl_init, NULL, NULL);
    }
    return type;
}

typedef struct {
    unsigned int val;
} If456;

typedef struct {
    AZImplementation impl;
    unsigned int val;
} If456Impl;

static void
if_456_impl_init(If456Impl *impl)
{
    impl->val = 1456;
}

unsigned int
if_456_get_type()
{
    static unsigned int type = 0;
    if (!type) {
        az_register_interface_type(&type, (const uint8_t *) "IF_456", AZ_TYPE_INTERFACE, sizeof(AZInterfaceClass), sizeof(If456Impl), sizeof(If456),
        0, 0, 0,
        NULL, (void (*) (AZImplementation *)) if_456_impl_init, NULL, NULL);
    }
    return type;
}

typedef struct {
    unsigned int val;
    If456 if456;
} If123;

typedef struct {
    AZImplementation impl;
    unsigned int val;
    If456Impl impl456;
} If123Impl;

static void
if_123_impl_init(If123Impl *impl)
{
    impl->val = 1123;
}

static void
if_123_class_init(AZClass *klass)
{
    az_class_declare_interface(klass, 0, TYPE_IF_456, offsetof(If123Impl,impl456), offsetof(If123,if456));
}

unsigned int
if_123_get_type()
{
    static unsigned int type = 0;
    if (!type) {
        az_register_interface_type(&type, (const uint8_t *) "IF_123", AZ_TYPE_INTERFACE, sizeof(AZInterfaceClass), sizeof(If123Impl), sizeof(If123),
        0, 1, 0,
        if_123_class_init,
        (void (*) (AZImplementation *)) if_123_impl_init, NULL, NULL);
    }
    return type;
}

typedef struct {
    unsigned int val;
    If123 if123;
    If789 if789;
} Struct12;

typedef struct {
    AZClass klass;
    If123Impl if123_impl;
    If456Impl if789_impl;
} Struct12Class;

static void
struct_12_class_init(AZClass *klass)
{
    az_class_declare_interface(klass, 0, TYPE_IF_123, offsetof(Struct12Class, if123_impl), offsetof(Struct12, if123));
    az_class_declare_interface(klass, 1, TYPE_IF_789, offsetof(Struct12Class, if789_impl), offsetof(Struct12, if789));
}

static void
struct_12_init(AZClass *klass, Struct12 *s_12)
{
    s_12->val = 12;
    s_12->if123.val = 12123;
    s_12->if123.if456.val = 12456;
    s_12->if789.val = 12789;
}

unsigned int
struct_12_get_type()
{
    static unsigned int type = 0;
    if (!type) {
        az_register_type(&type, (const uint8_t *) "STRUCT_12", AZ_TYPE_STRUCT, sizeof(Struct12Class), sizeof(Struct12),
        0, 2, 0,
        (void (*) (AZClass *)) struct_12_class_init,
        (void (*) (const AZImplementation *, void *)) struct_12_init,
        NULL);
    }
    return type;
}

typedef struct {
    unsigned int val;
    If123 if123;
    If789 if789;
} Block34;

typedef struct {
    AZClass klass;
    If123Impl if123_impl;
    If789Impl if789_impl;
} Block34Class;

static void
block_34_class_init(AZClass *klass)
{
    az_class_declare_interface(klass, 0, TYPE_IF_123, offsetof(Block34Class, if123_impl), offsetof(Block34, if123));
    az_class_declare_interface(klass, 1, TYPE_IF_789, offsetof(Block34Class, if789_impl), offsetof(Block34, if789));
}

static void
block_34_init(AZClass *klass, Block34 *b_34)
{
    b_34->val = 34;
    b_34->if123.val = 34123;
    b_34->if123.if456.val = 3456;
    b_34->if789.val = 34789;
}

unsigned int
block_34_get_type()
{
    static unsigned int type = 0;
    if (!type) {
        az_register_type(&type, (const uint8_t *) "BLOCK_34", AZ_TYPE_BLOCK, sizeof(Block34Class), sizeof(Block34),
        0, 2, 0,
        (void (*) (AZClass *)) block_34_class_init,
        (void (*) (const AZImplementation *, void *)) block_34_init,
        NULL);
    }
    return type;
}

void
test_interface(void)
{
    az_init();
    AZClass *bif_class = AZ_CLASS_FROM_TYPE(AZ_TYPE_BOXED_INTERFACE);
    Struct12Class *s12kl = (Struct12Class *) AZ_CLASS_FROM_TYPE(TYPE_STRUCT_12);
    TEST_ASSERT_EQUAL_UINT32(s12kl->if123_impl.val, 1123);
    TEST_ASSERT_EQUAL_UINT32(s12kl->if123_impl.impl456.val, 1456);
    TEST_ASSERT_EQUAL_UINT32(s12kl->if789_impl.val, 1789);

    Struct12 s12;
    az_instance_init(AZ_IMPL_FROM_TYPE(TYPE_STRUCT_12), &s12);
    TEST_ASSERT_EQUAL_UINT32(s12.val, 12);
    TEST_ASSERT_EQUAL_UINT32(s12.if123.val, 12123);
    TEST_ASSERT_EQUAL_UINT32(s12.if123.if456.val, 12456);
    TEST_ASSERT_EQUAL_UINT32(s12.if789.val, 12789);
    /* Get naked 123 interface from struct */
    const AZImplementation *if_impl;
    void *if_inst;
    if_impl = az_instance_get_interface(AZ_IMPL_FROM_TYPE(TYPE_STRUCT_12), &s12, TYPE_IF_123, &if_inst);
    TEST_ASSERT_NOT_NULL(if_impl);
    TEST_ASSERT_NOT_NULL(if_inst);
    TEST_ASSERT_EQUAL_PTR(if_impl, &s12kl->if123_impl);
    TEST_ASSERT_EQUAL_PTR(if_inst, &s12.if123);
    TEST_ASSERT_EQUAL_UINT32(((If123 *)if_inst)->val, 12123);
    /* Get the same interface from itself */
    const AZImplementation *if123_impl;
    void *if123_inst;
    if123_impl = az_instance_get_interface(if_impl, if_inst, TYPE_IF_123, &if123_inst);
    TEST_ASSERT_EQUAL_PTR(if123_impl, if_impl);
    TEST_ASSERT_EQUAL_PTR(if123_inst, if_inst);
    /* Get sub-interface from naked interface */
    const AZImplementation *if456_impl;
    void *if456_inst;
    if456_impl = az_instance_get_interface(if_impl, if_inst, TYPE_IF_456, &if456_inst);
    TEST_ASSERT_NOT_NULL(if456_impl);
    TEST_ASSERT_NOT_NULL(if456_inst);
    TEST_ASSERT_EQUAL_PTR(if456_impl, &s12kl->if123_impl.impl456);
    TEST_ASSERT_EQUAL_PTR(if456_inst, &s12.if123.if456);
    TEST_ASSERT_EQUAL_UINT32(AZ_IMPL_TYPE(if456_impl), TYPE_IF_456);

    /* Get boxed interface from struct */
    if_impl = az_instance_get_interface_autobox(AZ_IMPL_FROM_TYPE(TYPE_STRUCT_12), &s12, TYPE_IF_123, &if_inst);
    TEST_ASSERT_NOT_NULL(if_impl);
    TEST_ASSERT_NOT_NULL(if_inst);
    TEST_ASSERT_EQUAL_PTR(((AZBoxedInterface *) if_inst)->inst, &((Struct12 *) &((AZBoxedInterface *) if_inst)->val)->if123);
    TEST_ASSERT_EQUAL_UINT32(AZ_IMPL_TYPE(if_impl), AZ_TYPE_BOXED_INTERFACE);
    /* Get the same interface from boxed interface */
    if123_impl = az_instance_get_interface_autobox(if_impl, if_inst, TYPE_IF_123, &if123_inst);
    TEST_ASSERT_EQUAL_PTR(if123_impl, if_impl);
    TEST_ASSERT_EQUAL_PTR(if123_inst, if_inst);
    /* Get sub-interface from boxed interface */
    if456_impl = az_instance_get_interface(if_impl, if_inst, TYPE_IF_456, &if456_inst);
    TEST_ASSERT_NOT_NULL(if456_impl);
    TEST_ASSERT_NOT_NULL(if456_inst);
    TEST_ASSERT_EQUAL_PTR(if456_inst, &((Struct12 *) &((AZBoxedInterface *) if_inst)->val)->if123.if456);
    TEST_ASSERT_EQUAL_UINT32(AZ_IMPL_TYPE(if456_impl), TYPE_IF_456);
    TEST_ASSERT_EQUAL_UINT32(((If456 *)if456_inst)->val, 12456);
    /* Get boxed sub-interface from boxed interface */
    if456_impl = az_instance_get_interface_autobox(if_impl, if_inst, TYPE_IF_456, &if456_inst);
    TEST_ASSERT_NOT_NULL(if456_impl);
    TEST_ASSERT_NOT_NULL(if456_inst);
    TEST_ASSERT_EQUAL_UINT32(AZ_IMPL_TYPE(if456_impl), AZ_TYPE_BOXED_INTERFACE);
    TEST_ASSERT_EQUAL_UINT32(((If456 *) ((AZBoxedInterface *) if456_inst)->inst)->val, 12456);
}
