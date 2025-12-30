#define __AZ_FUNCTION_C__

/*
* A run-time type library
*
* Copyright (C) Lauris Kaplinski 2016-2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <az/base.h>
#include <az/convert.h>
#include <az/object.h>
#include <az/packed-value.h>
#include <az/private.h>

#include <az/function.h>

//static AZClass *function_signature_class = NULL;
//static AZClass *function_class = NULL;

AZClass AZFunctionSignatureKlass = {
	{AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_FUNCTION_SIGNATURE},
	&AZBlockKlass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "signature",
	7, sizeof(AZClass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL
};

AZInterfaceClass AZFunctionKlass = {
	{{AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, AZ_TYPE_FUNCTION},
	&AZInterfaceKlass.klass,
	0, 0, 0, 0, {0}, NULL,
	(const uint8_t *) "function",
	3, sizeof(AZInterfaceKlass), 0,
	NULL,
	NULL, NULL,
	NULL, NULL, az_any_to_string,
	NULL, NULL},
	sizeof(AZFunctionImplementation), NULL
};

void
az_init_function_classes (void)
{
	az_class_new_with_value(&AZFunctionSignatureKlass);
	az_class_new_with_value(&AZFunctionKlass.klass);
	//function_signature_class = az_class_new_with_type (AZ_TYPE_FUNCTION_SIGNATURE, AZ_TYPE_BLOCK, sizeof (AZClass), 0, AZ_FLAG_FINAL, (const uint8_t *) "signature");
	//function_class = az_class_new_with_type (AZ_TYPE_FUNCTION, AZ_TYPE_INTERFACE, sizeof (AZFunctionClass), sizeof (AZFunctionInstance), AZ_FLAG_ABSTRACT, (const uint8_t *) "function");
	//((AZInterfaceClass *) function_class)->implementation_size = sizeof (AZFunctionImplementation);
}

AZFunctionSignature *
az_function_signature_new (unsigned int this_type, unsigned int ret_type, unsigned int n_args, const unsigned int arg_types[])
{
	unsigned int n_total = (this_type) ? n_args + 1 : n_args;
	unsigned int i;
	AZFunctionSignature* sig = (AZFunctionSignature*) malloc(sizeof(AZFunctionSignature) - 4 + n_total * 4);
	az_instance_init_by_type (sig, AZ_TYPE_FUNCTION_SIGNATURE);
	sig->ret_type = ret_type;
	sig->n_args = 0;
	if (this_type) sig->arg_types[sig->n_args++] = this_type;
	for (i = 0; i < n_args; i++) sig->arg_types[sig->n_args++] = arg_types[i];
	return sig;
}

AZFunctionSignature*
az_function_signature_new_any(unsigned int this_type, unsigned int ret_type, unsigned int n_args)
{
	unsigned int n_total = (this_type) ? n_args + 1 : n_args;
	unsigned int i;
	AZFunctionSignature* sig = ( AZFunctionSignature*) malloc(sizeof(AZFunctionSignature) - 4 + n_total * 4);
	az_instance_init_by_type(sig, AZ_TYPE_FUNCTION_SIGNATURE);
	sig->ret_type = ret_type;
	sig->n_args = 0;
	if (this_type) sig->arg_types[sig->n_args++] = this_type;
	for (i = 0; i < n_args; i++) sig->arg_types[sig->n_args++] = AZ_TYPE_ANY;
	return sig;
}

AZFunctionSignature *
az_function_signature_new_va (unsigned int ret_type, unsigned int n_args, ...)
{
	AZFunctionSignature *sig;
	unsigned int arg_types[64], i;
	va_list ap;
	arikkei_return_val_if_fail (n_args < 64, NULL);
	va_start (ap, n_args);
	for (i = 0; i < n_args; i++) {
		arg_types[i] = va_arg (ap, unsigned int);
	}
	va_end (ap);
	sig = (AZFunctionSignature *) malloc (sizeof (AZFunctionSignature) - 4 + n_args * 4);
	az_instance_init_by_type (sig, AZ_TYPE_FUNCTION_SIGNATURE);
	sig->ret_type = ret_type;
	sig->n_args = n_args;
	for (i = 0; i < n_args; i++) sig->arg_types[i] = arg_types[i];
	return sig;
}

void
az_function_signature_delete(AZFunctionSignature* sig)
{
	az_instance_delete(AZ_TYPE_FUNCTION_SIGNATURE, sig);
}

unsigned int
az_function_signature_is_assignable_to (const AZFunctionSignature *sig, const AZFunctionSignature *other, unsigned int test_ret_val)
{
	unsigned int i;
	if (sig->n_args != other->n_args) return 0;
	if (test_ret_val && (sig->ret_type || other->ret_type) && !az_type_is_a (sig->ret_type, other->ret_type)) return 0;
	for (i = 0; i < sig->n_args; i++) {
		if (!az_type_is_convertible_to (other->arg_types[i], sig->arg_types[i])) return 0;
	}
	return 1;
}

const AZFunctionSignature *
az_function_get_signature (const AZFunctionImplementation *impl, AZFunctionInstance *inst)
{
#if 1
	return inst->signature;
#else
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, NULL);
	arikkei_return_val_if_fail (impl->signature != NULL, NULL);
#endif
	return impl->signature (impl, inst);
#endif
}

unsigned int
az_function_invoke (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
#if AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	ARIKKEI_CHECK_INTEGRITY ();
	unsigned int result = impl->invoke (impl, inst, arg_impls, arg_vals, ret_impl, ret_val, ctx);
	ARIKKEI_CHECK_INTEGRITY ();
	return result;
}

unsigned int
az_function_convert_args_in_place (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation *arg_impls[], AZValue *arg_vals[])
{
	unsigned int i;
#if AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	/* fixme: Not sure whether converting this is meaningful or not */
	for (i = 0; i < inst->signature->n_args; i++) {
		if (!az_value_convert_in_place (&arg_impls[i], arg_vals[i], inst->signature->arg_types[i])) return 0;
	}
	return 1;
}

unsigned int
az_function_invoke_packed (const AZFunctionImplementation *impl, AZFunctionInstance *inst, AZPackedValue *thisval, AZPackedValue64 *retval, AZPackedValue *args, unsigned int checktypes)
{
	unsigned int s, d;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
	if (checktypes) {
		s = d = 0;
		if (thisval->impl) {
			if (!az_type_is_a (AZ_IMPL_TYPE(thisval->impl), inst->signature->arg_types[d])) {
				fprintf (stderr, ".");
			}
			arikkei_return_val_if_fail (az_type_is_a(AZ_IMPL_TYPE(thisval->impl), inst->signature->arg_types[d]), 0);
			d += 1;
		}
		while (d < inst->signature->n_args) {
			if (!args[s].impl && az_type_is_a (inst->signature->arg_types[d], AZ_TYPE_BLOCK)) {
				s += 1;
				d += 1;
				continue;
			}
			if (!az_type_is_a (AZ_PACKED_VALUE_TYPE(&args[s]), inst->signature->arg_types[d])) {
				fprintf (stderr, "az_function_invoke: Invalid argument type (%u) %u is not %u\n", d, AZ_PACKED_VALUE_TYPE(&args[s]), inst->signature->arg_types[d]);
				return 0;
			}
			s += 1;
			d += 1;
		}
	}
	const AZImplementation *arg_impls[32];
	const AZValue *arg_vals[32];
	s = d = 0;
	if (thisval->impl) {
		arg_impls[d] = thisval->impl;
		arg_vals[d] = &thisval->v;
		d += 1;
	}
	while (d < inst->signature->n_args) {
		arg_impls[d] = args[s].impl;
		arg_vals[d] = &args[s].v;
		s += 1;
		d += 1;
	}
	if (retval) {
		if (inst->signature->ret_type) az_packed_value_clear (&retval->packed_val);
		return (impl->invoke (impl, inst, arg_impls, arg_vals, &retval->impl, &retval->v, NULL));
	} else {
		AZPackedValue64 ret_val;
		ret_val.impl = NULL;
		/* Need to be careful - inst may be destroyed during call */
		if (!impl->invoke (impl, inst, arg_impls, arg_vals, &ret_val.impl, &ret_val.v, NULL)) return 0;
		az_packed_value_clear (&ret_val.packed_val);
	}
	return 1;
}

unsigned int
az_function_invoke_by_type_instance (unsigned int type, void *instance, AZPackedValue *thisval, AZPackedValue64 *retval, AZPackedValue *args, unsigned int checktypes)
{
	AZClass *klass;
	AZFunctionImplementation *impl;
	AZFunctionInstance *inst;
	arikkei_return_val_if_fail (az_type_implements (type, AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (instance != NULL, 0);
	klass = az_type_get_class (type);
	impl = (AZFunctionImplementation *) az_get_interface (&klass->impl, instance, AZ_TYPE_FUNCTION, (void **) &inst);
	return az_function_invoke_packed (impl, inst, thisval, retval, args, checktypes);
}

unsigned int
az_instance_invoke_function (const AZImplementation *impl, void *inst, AZPackedValue *this_val, AZPackedValue64 *ret_val, AZPackedValue *args, unsigned int check_types)
{
	AZFunctionImplementation *func_impl;
	AZFunctionInstance *func_inst;
#if AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (az_type_implements(AZ_IMPL_TYPE(impl), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	func_impl = (AZFunctionImplementation *) az_get_interface (impl, inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
	return az_function_invoke_packed (func_impl, func_inst, this_val, ret_val, args, check_types);
}

#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_ARM_64
#endif

/* We exploit AMD64 calling convention */
/* fixme: Implement checks and fallbacks */

#ifdef ARCH_X86_64
#define ARG_STEP 1
static void
function_build_arguments (const AZFunctionSignature *sig, uint64_t *p, const AZImplementation *arg_impls[], const AZValue *arg_ptrs[])
{
	unsigned int i;
	for (i = 0; i < sig->n_args; i++) {
		AZClass *klass = AZ_CLASS_FROM_TYPE(sig->arg_types[i]);
		if (AZ_CLASS_IS_VALUE(klass)) {
			if (klass->impl.flags & AZ_FLAG_FINAL) {
				/* Final value types */
				arg_impls[i] = &klass->impl;
				if (AZ_CLASS_VALUE_SIZE(klass) <= 8) {
					arg_ptrs[i] = (const AZValue *) p;
				} else {
					arg_ptrs[i] = (const AZValue *) *p;
				}
				p += ARG_STEP;
			} else {
				/* Nonfinal value types */
				arg_impls[i] = (const AZImplementation *) *p;
				p += ARG_STEP;
				if (AZ_CLASS_VALUE_SIZE(klass) <= 8) {
					arg_ptrs[i] = (const AZValue *) p;
				} else {
					arg_ptrs[i] = (const AZValue *) *p;
				}
				p += ARG_STEP;
			}
		} else if (az_type_is_a (sig->arg_types[i], AZ_TYPE_OBJECT)) {
			/* Objects */
			AZObject *obj = (AZObject *) *p;
			arg_impls[i] = (AZImplementation *) obj->klass;
			arg_ptrs[i] = (const AZValue *) p;
			p += ARG_STEP;
		} else {
			/* Non-object block types */
			if (klass->impl.flags & AZ_FLAG_FINAL) {
				arg_impls[i] = &klass->impl;
				arg_ptrs[i] = (const AZValue *) p;
				p += ARG_STEP;
			} else {
				arg_impls[i] = (const AZImplementation *) *p;
				p += ARG_STEP;
				arg_ptrs[i] = (const AZValue *) p;
				p += ARG_STEP;
			}
		}
	}
}
#endif

#ifdef ARCH_ARM_64
static void
function_build_arguments_arm64 (const AZFunctionSignature *sig, va_list ap, const AZImplementation *arg_impls[], AZValue arg_vals[], const AZValue *arg_ptrs[])
{
	unsigned int i;
	for (i = 0; i < sig->n_args; i++) {
		AZClass *klass = AZ_CLASS_FROM_TYPE(sig->arg_types[i]);
		if (AZ_CLASS_IS_VALUE(klass)) {
			if (klass->impl.flags & AZ_FLAG_FINAL) {
				/* Final value type, no implementation */
				arg_impls[i] = &klass->impl;
				if (az_class_value_size(klass) <= 8) {
                                    /* fixme: Do the proper copying */
                                    arg_vals[i].uint64_v = va_arg(ap, uint64_t);
				} else {
                                    /* fixme: Do the proper copying */
                                    arg_vals[i].uint64_v = va_arg(ap, uint64_t);
				}
                                arg_ptrs[i] = &arg_vals[i];
			} else {
				/* Nonfinal value type, add implementation */
				arg_impls[i] = (const AZImplementation *) va_arg(ap, AZImplementation *);
				if (az_class_value_size(klass) <= 8) {
                                    /* fixme: Do the proper copying */
                                    arg_vals[i].uint64_v = va_arg(ap, uint64_t);
				} else {
                                    /* fixme: Do the proper copying */
                                    arg_vals[i].uint64_v = va_arg(ap, uint64_t);
				}
                                arg_ptrs[i] = &arg_vals[i];
			}
		} else if (az_type_is_a (sig->arg_types[i], AZ_TYPE_OBJECT)) {
			/* Object, no implementation, value is pointer to instance */
			AZObject *obj = (AZObject *) va_arg(ap, AZObject *);
			arg_impls[i] = (obj) ? (AZImplementation *) obj->klass : &klass->impl;
                        arg_vals[i].block = obj;
			arg_ptrs[i] = &arg_vals[i];
		} else {
			/* Non-object block types */
			if (klass->impl.flags & AZ_FLAG_FINAL) {
                                /* Implementation is omitted for final types */
				arg_impls[i] = &klass->impl;
                                arg_vals[i].block = (void *) va_arg(ap, void *);
                                arg_ptrs[i] = &arg_vals[i];
			} else {
                                /* Add implementation */
				arg_impls[i] = (const AZImplementation *) va_arg(ap, AZImplementation *);
                                arg_vals[i].block = (void *) va_arg(ap, void *);
                                arg_ptrs[i] = &arg_vals[i];
			}
		}
	}
}
#endif

unsigned int
az_function_invoke_va (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZImplementation **ret_impl, AZValue64 *ret_val, ...)
{
	const AZImplementation *arg_impls[64];
	const AZValue *arg_ptrs[64];

	arikkei_return_val_if_fail (inst->signature->n_args < 64, 0);

#ifdef ARCH_X86_64
	/* We exploit AMD64 calling convention */
	/* fixme: Implement checks and fallbacks */
	uint64_t *p = (uint64_t *) &ret_val + 1;
	function_build_arguments (inst->signature, p, arg_impls, arg_ptrs);
#endif
#ifdef ARCH_ARM_64
        AZValue arg_vals[64];
        va_list ap;
        va_start(ap, ret_val);
	function_build_arguments_arm64 (inst->signature, ap, arg_impls, arg_vals, arg_ptrs);
        va_end(ap);
#endif


	return az_function_invoke (impl, inst, arg_impls, arg_ptrs, ret_impl, ret_val, NULL);
}

unsigned int
az_function_invoke_by_signature_va (const AZFunctionImplementation *impl, AZFunctionInstance *inst, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...)
{
	const AZImplementation *arg_impls[64];
	const AZValue *arg_ptrs[64];

	arikkei_return_val_if_fail (sig->n_args < 64, 0);

#ifdef ARCH_X86_64
	/* We exploit AMD64 calling convention */
	/* fixme: Implement checks and fallbacks */
	uint64_t *p = (uint64_t *) &ret_val + 1;
	function_build_arguments (sig, p, arg_impls, arg_ptrs);
#endif
#ifdef ARCH_ARM_64
        AZValue arg_vals[64];
        va_list ap;
        va_start(ap, ret_val);
	function_build_arguments_arm64 (sig, ap, arg_impls, arg_vals, arg_ptrs);
        va_end(ap);
#endif

	return az_function_invoke (impl, inst, arg_impls, arg_ptrs, ret_impl, ret_val, NULL);
}

unsigned int
az_function_invoke_by_value_signature_va (const AZImplementation *impl, const AZValue *val, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...)
{
	AZFunctionImplementation *f_impl;
	AZFunctionInstance *f_inst;
	const AZImplementation *arg_impls[64];
	const AZValue *arg_ptrs[64];
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (sig->n_args < 64, 0);
	f_impl = (AZFunctionImplementation *) az_get_interface (impl, az_value_get_inst(impl, val), AZ_TYPE_FUNCTION, (void **) &f_inst);
	arikkei_return_val_if_fail (f_impl != NULL, 0);

#ifdef ARCH_X86_64
	/* We exploit AMD64 calling convention */
	/* fixme: Implement checks and fallbacks */
	uint64_t *p = (uint64_t *) &ret_val + 1;
	function_build_arguments (sig, p, arg_impls, arg_ptrs);
#endif
#ifdef ARCH_ARM_64
        AZValue arg_vals[64];
        va_list ap;
        va_start(ap, ret_val);
	function_build_arguments_arm64 (sig, ap, arg_impls, arg_vals, arg_ptrs);
        va_end(ap);
#endif

	return f_impl->invoke (f_impl, f_inst, arg_impls, arg_ptrs, ret_impl, ret_val, NULL);
}
