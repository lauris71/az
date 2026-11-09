#define __AZ_SIGNATURE_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2026
*/

#include <stdlib.h>
#include <stdarg.h>

#include <arikkei/arikkei-strlib.h>
#include <arikkei/arikkei-dict.h>
#include <az/base.h>
#include <az/instance.h>
#include <az/private.h>
#include <az/signature.h>
#include <az/types.h>

#ifdef AZ_MT_REFERENCES
#include <arikkei/arikkei-threads.h>
#endif

typedef struct _AZFunctionSignature255 AZFunctionSignature255;

struct _AZFunctionSignature255 {
	uint32_t ret_type;
	uint32_t n_args;
	uint32_t arg_types[255];
};

#ifdef AZ_MT_REFERENCES
static mtx_t dict_mutex;
#define AZ_SIGNATURE_LOCK() mtx_lock (&dict_mutex)
#define AZ_SIGNATURE_UNLOCK() mtx_unlock (&dict_mutex)
#else
#define AZ_SIGNATURE_LOCK()
#define AZ_SIGNATURE_UNLOCK()
#endif

static unsigned int
sig_hash (const void *key)
{
    AZFunctionSignature *sig = *((AZFunctionSignature **) key);
    unsigned int size = sizeof(AZFunctionSignature) + AZ_SIGNATURE_N_ARG_TYPES(sig) * sizeof(uint32_t);
    return arikkei_memory_hash (sig, size);
}

static unsigned int
sig_equal (const void *lhs, const void *rhs)
{
    AZFunctionSignature *sig1 = *((AZFunctionSignature **) lhs);
    AZFunctionSignature *sig2 = *((AZFunctionSignature **) rhs);
    if (sig1->n_args != sig2->n_args) return 0;
    if (sig1->ret_type != sig2->ret_type) return 0;
    for (unsigned int i = 0; i < sig1->n_args; i++) {
        if (sig1->arg_types[i] != sig2->arg_types[i]) return 0;
    }
    return 1;
}

static void
sig_copy_value (void *dst, const void *src)
{
	/* An interned signature is both a key and a value, no need to double-copy */
}

static ArikkeiDict sig_dict = {
    .key_offset = 8,
    .key_size = 8,
    .val_offset = 8,
    .val_size = 8,
    .hash = sig_hash,
    .equal = sig_equal,
    .copy_key = NULL,
    .free_key = NULL,
    .copy_value = sig_copy_value,
    .free_value = NULL,
    .root_size = 0,
    .size = 0,
    .free = 0,
    .entry_size = 0,
    .entries = NULL
};

static unsigned int
function_signature_to_string(const AZImplementation *impl, void *inst, uint8_t *d, unsigned int d_len)
{
	AZFunctionSignature *sig = (AZFunctionSignature *) inst;
	if (!d) d_len = 0;
	unsigned int len = 0, l = 0;
	len += arikkei_strncpy(d + l, d_len - l, (const uint8_t *) "(");
	l = (len <= d_len) ? len : d_len;
	for (unsigned int i = 0; i < sig->n_args; i++) {
		if (i > 0) {
			len += arikkei_strncpy(d + l, d_len - l, (const uint8_t *) ", ");
			l = (len <= d_len) ? len : d_len;
		}
		AZClass *klass = AZ_CLASS_FROM_TYPE(sig->arg_types[i]);
		len += arikkei_strncpy(d + l, d_len - l, klass->name);
		l = (len <= d_len) ? len : d_len;
	}
	len += arikkei_strncpy(d + l, d_len - l, (const uint8_t *) ") => ");
	l = (len <= d_len) ? len : d_len;
	if (sig->ret_type) {
		AZClass *klass = AZ_CLASS_FROM_TYPE(sig->ret_type);
		len += arikkei_strncpy(d + l, d_len - l, klass->name);
	} else {
		len += arikkei_strncpy(d + l, d_len - l, (const uint8_t *) "void");
	}
	return len;
}

AZ_CLASS_ALIGN AZClass AZFunctionSignatureKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_FUNCTION_SIGNATURE },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "signature",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = function_signature_to_string
};

void
az_init_signature_class (void)
{
#ifdef AZ_MT_REFERENCES
	mtx_init (&dict_mutex, mtx_plain);
#endif
	az_class_new_with_value(&AZFunctionSignatureKlass);
    arikkei_dict_setup(&sig_dict, 701);
}

#define SIGNATURE_POOL_SIZE 4096

static AZFunctionSignature *
signature_new_from_template(AZFunctionSignature255 *sig_255)
{
    AZ_SIGNATURE_LOCK();
    AZFunctionSignature **known = (AZFunctionSignature **) arikkei_dict_lookup(&sig_dict, &sig_255);
    if (!known) {
        static uint8_t *pool = NULL;
        static uint8_t *pool_end = NULL;
        unsigned int sig_size = sizeof(AZFunctionSignature) + AZ_SIGNATURE_N_ARG_TYPES(sig_255) * sizeof(uint32_t);
        if (!pool || pool + sig_size > pool_end) {
            pool = (uint8_t *) malloc(SIGNATURE_POOL_SIZE);
            pool_end = pool + SIGNATURE_POOL_SIZE;
        }
        AZFunctionSignature *sig = (AZFunctionSignature *) pool;
        pool += sig_size;
        memcpy(sig, sig_255, sig_size);
        arikkei_dict_insert(&sig_dict, &sig, &sig);
	    AZ_SIGNATURE_UNLOCK();
		return sig;
    }
    AZ_SIGNATURE_UNLOCK();
    return *known;
}

AZFunctionSignature *
az_function_signature_new(unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[])
{
	arikkei_return_val_if_fail(n_args <= 255, NULL);
    AZFunctionSignature255 sig_255;
	sig_255.ret_type = ret_type;
	sig_255.n_args = n_args;
	for (unsigned int i = 0; i < n_args; i++) sig_255.arg_types[i] = arg_types[i];
    return signature_new_from_template(&sig_255);
}

AZFunctionSignature *
az_function_signature_new_va (unsigned int ret_type, unsigned int n_args, ...)
{
	arikkei_return_val_if_fail(n_args <= 255, NULL);
    AZFunctionSignature255 sig_255;
	sig_255.ret_type = ret_type;
	sig_255.n_args = n_args;
	va_list ap;
	va_start (ap, n_args);
	for (unsigned int i = 0; i < n_args; i++) {
		sig_255.arg_types[i] = va_arg (ap, unsigned int);
	}
	va_end (ap);
	return signature_new_from_template(&sig_255);
}

AZFunctionSignature *
az_function_signature_new_with_this(unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[])
{
	arikkei_return_val_if_fail(n_args < 255, NULL);
    AZFunctionSignature255 sig_255;
	unsigned int n_total = (this_type) ? n_args + 1 : n_args;
	unsigned int i;
	sig_255.ret_type = ret_type;
	sig_255.n_args = 0;
	if (this_type) sig_255.arg_types[sig_255.n_args++] = this_type;
	for (i = 0; i < n_args; i++) sig_255.arg_types[sig_255.n_args++] = arg_types[i];
    return signature_new_from_template(&sig_255);
}

AZFunctionSignature*
az_function_signature_new_with_this_any(unsigned int this_type, unsigned int ret_type, unsigned int n_args)
{
	arikkei_return_val_if_fail(n_args < 255, NULL);
    AZFunctionSignature255 sig_255;
	unsigned int n_total = (this_type) ? n_args + 1 : n_args;
	unsigned int i;
	sig_255.ret_type = ret_type;
	sig_255.n_args = 0;
	if (this_type) sig_255.arg_types[sig_255.n_args++] = this_type;
	for (i = 0; i < n_args; i++) sig_255.arg_types[sig_255.n_args++] = AZ_TYPE_ANY;
	return signature_new_from_template(&sig_255);
}
