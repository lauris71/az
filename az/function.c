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
#include <az/instance.h>
#include <az/object.h>
#include <az/packed-value.h>
#include <az/private.h>

#include <az/function.h>

//static AZClass *function_signature_class = NULL;
//static AZClass *function_class = NULL;

AZ_CLASS_ALIGN AZClass AZFunctionSignatureKlass = {
	.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_FINAL | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_FUNCTION_SIGNATURE },
	.parent = &AZBlockKlass,
	.name = (const uint8_t *) "signature",
	.alignment = 7,
	.class_size = sizeof(AZClass),
	.instance_size = 0,
	.to_string = az_any_to_string
};

AZ_CLASS_ALIGN AZInterfaceClass AZFunctionKlass = {
	.klass = {
		.impl = { .flags = AZ_FLAG_BLOCK | AZ_FLAG_ABSTRACT | AZ_FLAG_INTERFACE | AZ_FLAG_IMPL_IS_CLASS, .type = AZ_TYPE_FUNCTION },
		.parent = &AZInterfaceKlass.klass,
		.name = (const uint8_t *) "function",
		.alignment = 3,
		.class_size = sizeof(AZInterfaceKlass),
		.instance_size = 0,
		.to_string = az_any_to_string
	},
	.implementation_size = sizeof(AZFunctionImplementation),
	.implementation_init = NULL
};

void
az_init_function_classes (void)
{
	az_class_new_with_value(&AZFunctionSignatureKlass);
	az_class_new_with_value(&AZFunctionKlass.klass);
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
		/* fixme: For now we have to accept conditional conversion (should be fixed in Aosora first) */
		if (az_type_get_conversion_to (other->arg_types[i], sig->arg_types[i]) > AZ_CONVERT_CONDITIONAL) return 0;
	}
	return 1;
}

const AZFunctionSignature *
az_function_get_signature (const AZFunctionImplementation *impl, void *inst)
{
#ifdef AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, NULL);
	arikkei_return_val_if_fail (impl->signature != NULL, NULL);
#endif
	return impl->signature (impl, inst);
}

unsigned int
az_function_invoke (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], const AZValue *arg_vals[], const AZImplementation **ret_impl, AZValue64 *ret_val, AZContext *ctx)
{
#if AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	unsigned int result = impl->invoke (impl, inst, arg_impls, arg_vals, ret_impl, ret_val, ctx);
	return result;
}

unsigned int
az_function_convert_args_in_place (const AZFunctionImplementation *impl, void *inst, const AZImplementation *arg_impls[], AZValue *arg_vals[])
{
	unsigned int i;
#if AZ_SAFETY_CHECKS
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
#endif
	const AZFunctionSignature *sig = az_function_get_signature(impl, inst);
	/* fixme: Not sure whether converting this is meaningful or not */
	for (i = 0; i < sig->n_args; i++) {
		if (az_value_convert_in_place (&arg_impls[i], arg_vals[i], sig->arg_types[i], AZ_CONVERT_CONDITIONAL) == AZ_CONVERSION_FAILED) return 0;
	}
	return 1;
}

unsigned int
az_function_invoke_packed (const AZFunctionImplementation *impl, void *inst, AZPackedValue *thisval, AZPackedValue64 *retval, AZPackedValue *args, unsigned int checktypes)
{
	unsigned int s, d;
	arikkei_return_val_if_fail (impl != NULL, 0);
	arikkei_return_val_if_fail (az_type_is_a (AZ_IMPL_TYPE(&impl->implementation), AZ_TYPE_FUNCTION), 0);
	arikkei_return_val_if_fail (inst != NULL, 0);
	const AZFunctionSignature *sig = az_function_get_signature(impl, inst);
	if (checktypes) {
		s = d = 0;
		if (thisval->impl) {
			if (!az_type_is_a (AZ_IMPL_TYPE(thisval->impl), sig->arg_types[d])) {
				fprintf (stderr, ".");
			}
			arikkei_return_val_if_fail (az_type_is_a(AZ_IMPL_TYPE(thisval->impl), sig->arg_types[d]), 0);
			d += 1;
		}
		while (d < sig->n_args) {
			if (!args[s].impl && AZ_TYPE_IS_BLOCK (sig->arg_types[d])) {
				s += 1;
				d += 1;
				continue;
			}
			if (!az_type_is_a (AZ_PACKED_VALUE_TYPE(&args[s]), sig->arg_types[d])) {
				fprintf (stderr, "az_function_invoke: Invalid argument type (%u) %u is not %u\n", d, AZ_PACKED_VALUE_TYPE(&args[s]), sig->arg_types[d]);
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
	while (d < sig->n_args) {
		arg_impls[d] = args[s].impl;
		arg_vals[d] = &args[s].v;
		s += 1;
		d += 1;
	}
	if (retval) {
		if (sig->ret_type) az_packed_value_clear (&retval->packed_val);
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
	impl = (AZFunctionImplementation *) az_instance_get_interface (&klass->impl, instance, AZ_TYPE_FUNCTION, (void **) &inst);
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
	func_impl = (AZFunctionImplementation *) az_instance_get_interface (impl, inst, AZ_TYPE_FUNCTION, (void **) &func_inst);
	return az_function_invoke_packed (func_impl, func_inst, this_val, ret_val, args, check_types);
}

#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_X86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_ARM_64
#endif

static void
function_build_arguments_arm64 (const AZFunctionSignature *sig, va_list ap, const AZImplementation *arg_impls[], AZValue arg_vals[], const AZValue *arg_ptrs[])
{
	unsigned int i;
	for (i = 0; i < sig->n_args; i++) {
		AZClass *klass = AZ_CLASS_FROM_TYPE(sig->arg_types[i]);
		if (AZ_TYPE_IS_OBJECT(sig->arg_types[i])) {
			/* Objects - [pointer] */
			AZObject *obj = (AZObject *) va_arg(ap, AZObject *);
			arg_impls[i] = (obj) ? (AZImplementation *) obj->klass : &klass->impl;
			arg_vals[i].block = obj;
		} else if (AZ_TYPE_IS_FINAL(sig->arg_types[i])) {
			/* Final type, no implementation */
			arg_impls[i] = &klass->impl;
			if (AZ_TYPE_IS_PRIMITIVE(sig->arg_types[i])) {
				/* Primitive types - [value] */
				switch(sig->arg_types[i]) {
					case AZ_TYPE_BOOLEAN:
					case AZ_TYPE_INT8:
					case AZ_TYPE_UINT8:
					case AZ_TYPE_INT16:
					case AZ_TYPE_UINT16:
					case AZ_TYPE_INT32:
					case AZ_TYPE_UINT32:
						arg_vals[i].int32_v = va_arg(ap, int32_t);
						break;
					case AZ_TYPE_INT64:
					case AZ_TYPE_UINT64:
						arg_vals[i].int64_v = va_arg(ap, int64_t);
						break;
					case AZ_TYPE_FLOAT:
					case AZ_TYPE_DOUBLE:
						arg_vals[i].double_v = va_arg(ap, double);
						break;
					case AZ_TYPE_COMPLEX_FLOAT:
						arg_vals[i].cfloat_v = va_arg(ap, AZComplexFloat);
						break;
					case AZ_TYPE_COMPLEX_DOUBLE:
						arg_vals[i].cdouble_v = va_arg(ap, AZComplexDouble);
						break;
					case AZ_TYPE_POINTER:
						arg_vals[i].pointer_v = (void *) va_arg(ap, void *);
						break;
				}
			} else {
				/* Final types - [pointer] */
				arg_vals[i].block = (void *) va_arg(ap, void *);
			}
		} else {
			/* Non-final types - [impl, pointer] */
			arg_impls[i] = (const AZImplementation *) va_arg(ap, AZImplementation *);
			arg_vals[i].pointer_v = (void *) va_arg(ap, void *);
		}
		arg_ptrs[i] = &arg_vals[i];
	}
}

unsigned int
az_function_invoke_va (const AZFunctionImplementation *impl, void *inst, const AZImplementation **ret_impl, AZValue64 *ret_val, ...)
{
	const AZImplementation *arg_impls[64];
	const AZValue *arg_ptrs[64];

	const AZFunctionSignature *sig = az_function_get_signature(impl, inst);
	arikkei_return_val_if_fail (sig->n_args < 64, 0);

	AZValue arg_vals[64];
	va_list ap;
	va_start(ap, ret_val);
	function_build_arguments_arm64 (sig, ap, arg_impls, arg_vals, arg_ptrs);
	va_end(ap);

	return az_function_invoke (impl, inst, arg_impls, arg_ptrs, ret_impl, ret_val, NULL);
}

unsigned int
az_function_invoke_by_signature_va (const AZFunctionImplementation *impl, void *inst, const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, ...)
{
	const AZImplementation *arg_impls[64];
	const AZValue *arg_ptrs[64];

	arikkei_return_val_if_fail (sig->n_args < 64, 0);

	AZValue arg_vals[64];
	va_list ap;
	va_start(ap, ret_val);
	function_build_arguments_arm64 (sig, ap, arg_impls, arg_vals, arg_ptrs);
	va_end(ap);

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
	f_impl = (AZFunctionImplementation *) az_instance_get_interface (impl, az_value_get_inst(impl, val), AZ_TYPE_FUNCTION, (void **) &f_inst);
	arikkei_return_val_if_fail (f_impl != NULL, 0);

	AZValue arg_vals[64];
	va_list ap;
	va_start(ap, ret_val);
	function_build_arguments_arm64 (sig, ap, arg_impls, arg_vals, arg_ptrs);
	va_end(ap);

	return f_impl->invoke (f_impl, f_inst, arg_impls, arg_ptrs, ret_impl, ret_val, NULL);
}

#if defined(ARCH_ARM_64) && defined(__GNUC__)

/*
 * ARM64 (AAPCS64) native call
 *
 * The C code marshals the arguments into an AZNativeCallFrame (8 general purpose
 * registers, 8 FP/SIMD registers and the stack overflow area) and the assembly
 * trampoline loads the registers, copies the stack area and performs the call.
 * After the call the trampoline stores x0, d0 and d1 into the result buffer - this
 * covers all return types of the native calling rules (integers, pointers, floats,
 * doubles and 2-element homogeneous FP aggregates).
 */

#include <stddef.h>

#define AZ_NATIVE_CALL_MAX_STACK 1024

typedef struct {
	uint64_t gprs[8];
	uint64_t fprs[8];
	uint64_t stack[AZ_NATIVE_CALL_MAX_STACK / 8];
} AZNativeCallFrame;

typedef struct {
	uint64_t gpr;
	uint64_t fprs[2];
} AZNativeCallResult;

/* The trampoline expects the stack area at offset 128 (8 gprs + 8 fprs) */
_Static_assert (offsetof (AZNativeCallFrame, stack) == 128, "AZNativeCallFrame stack offset has to be 128");

extern void az_native_call_frame_arm64 (void (*func) (void), const AZNativeCallFrame *frame, uint64_t stack_bytes, AZNativeCallResult *result);

#if defined(__APPLE__)
#define AZ_NATIVE_CALL_FRAME_SYMBOL "_az_native_call_frame_arm64"
#else
#define AZ_NATIVE_CALL_FRAME_SYMBOL "az_native_call_frame_arm64"
#endif

__asm__(
	".text\n"
	".p2align 4\n"
	".globl " AZ_NATIVE_CALL_FRAME_SYMBOL "\n"
	AZ_NATIVE_CALL_FRAME_SYMBOL ":\n"
	/* x0 = function, x1 = frame, x2 = stack_bytes, x3 = result */
	"stp	x29, x30, [sp, #-32]!\n"
	"mov	x29, sp\n"
	"str	x19, [sp, #16]\n"
	"mov	x9, x0\n"		/* function */
	"mov	x10, x1\n"		/* frame */
	"mov	x19, x3\n"		/* result (callee-saved) */
	/* Allocate stack space for stack arguments (16-byte aligned) */
	"add	x12, x2, #15\n"
	"and	x12, x12, #-16\n"
	"sub	sp, sp, x12\n"
	/* Copy stack arguments */
	"cbz	x2, 2f\n"
	"add	x13, x10, #128\n"
	"mov	x14, sp\n"
	"1:\n"
	"ldr	x15, [x13], #8\n"
	"str	x15, [x14], #8\n"
	"sub	x2, x2, #8\n"
	"cbnz	x2, 1b\n"
	"2:\n"
	/* Load floating point argument registers */
	"ldp	d0, d1, [x10, #64]\n"
	"ldp	d2, d3, [x10, #80]\n"
	"ldp	d4, d5, [x10, #96]\n"
	"ldp	d6, d7, [x10, #112]\n"
	/* Load general purpose argument registers */
	"ldp	x0, x1, [x10, #0]\n"
	"ldp	x2, x3, [x10, #16]\n"
	"ldp	x4, x5, [x10, #32]\n"
	"ldp	x6, x7, [x10, #48]\n"
	/* Call the native function */
	"blr	x9\n"
	/* Store the result registers */
	"str	x0, [x19, #0]\n"
	"stp	d0, d1, [x19, #8]\n"
	/* Tear down the frame */
	"ldr	x19, [x29, #16]\n"
	"mov	sp, x29\n"
	"ldp	x29, x30, [sp], #32\n"
	"ret\n"
);

static unsigned int
native_frame_push_gpr (AZNativeCallFrame *frame, unsigned int *n_gpr, unsigned int *stack_bytes, uint64_t val)
{
	if (*n_gpr < 8) {
		frame->gprs[(*n_gpr)++] = val;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		frame->stack[*stack_bytes / 8] = val;
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr32 (AZNativeCallFrame *frame, unsigned int *n_fpr, unsigned int *stack_bytes, float val)
{
	if (*n_fpr < 8) {
		uint32_t u;
		memcpy (&u, &val, 4);
		frame->fprs[(*n_fpr)++] = u;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 4);
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr64 (AZNativeCallFrame *frame, unsigned int *n_fpr, unsigned int *stack_bytes, double val)
{
	if (*n_fpr < 8) {
		memcpy (&frame->fprs[(*n_fpr)++], &val, 8);
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 8);
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_stack (AZNativeCallFrame *frame, unsigned int *stack_bytes, const void *data, unsigned int size)
{
	unsigned int n = (size + 7) & ~7u;
	if (*stack_bytes + n > AZ_NATIVE_CALL_MAX_STACK) return 0;
	memcpy ((uint8_t *) frame->stack + *stack_bytes, data, size);
	*stack_bytes += n;
	return 1;
}

unsigned int
az_function_call_native (void (*func) (void), const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation *arg_impls[], const AZValue *arg_vals[])
{
	AZNativeCallFrame frame;
	AZNativeCallResult result;
	AZValue64 tmp_ret;
	unsigned int n_gpr = 0, n_fpr = 0, stack_bytes = 0;
	unsigned int i, rtype;
	AZClass *rklass;

	arikkei_return_val_if_fail (func != NULL, 0);
	arikkei_return_val_if_fail (sig != NULL, 0);
	arikkei_return_val_if_fail (sig->n_args < 64, 0);

	if (!ret_val) ret_val = &tmp_ret;

	/* Hidden return storage argument */
	if (sig->ret_type && !AZ_TYPE_IS_OBJECT (sig->ret_type) && !AZ_TYPE_IS_PRIMITIVE (sig->ret_type)) {
		if (!AZ_TYPE_IS_FINAL (sig->ret_type) || !AZ_TYPE_IS_BLOCK (sig->ret_type)) {
			/* Final values and all non-final types: the first argument is a pointer to the return storage */
			void *storage = (AZ_TYPE_IS_BLOCK (sig->ret_type)) ? (void *) &ret_val->value.block : (void *) ret_val;
			if (!AZ_TYPE_IS_BLOCK (sig->ret_type)) {
				arikkei_return_val_if_fail (AZ_CLASS_FROM_TYPE (sig->ret_type)->instance_size <= AZ_FUNCTION_MAX_RETURN_VALUE_SIZE, 0);
			}
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) storage)) return 0;
		}
	}

	/* Arguments */
	for (i = 0; i < sig->n_args; i++) {
		unsigned int type = sig->arg_types[i];
		AZClass *klass = AZ_CLASS_FROM_TYPE (type);
		if (AZ_TYPE_IS_OBJECT (type)) {
			/* Objects - [pointer] */
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->block)) return 0;
		} else if (AZ_TYPE_IS_PRIMITIVE (type)) {
			/* Primitive types - [value] */
			switch (type) {
			case AZ_TYPE_BOOLEAN:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->boolean_v)) return 0;
				break;
			case AZ_TYPE_INT8:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int8_v)) return 0;
				break;
			case AZ_TYPE_UINT8:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint8_v)) return 0;
				break;
			case AZ_TYPE_INT16:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int16_v)) return 0;
				break;
			case AZ_TYPE_UINT16:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint16_v)) return 0;
				break;
			case AZ_TYPE_INT32:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) arg_vals[i]->int32_v)) return 0;
				break;
			case AZ_TYPE_UINT32:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint32_v)) return 0;
				break;
			case AZ_TYPE_INT64:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) arg_vals[i]->int64_v)) return 0;
				break;
			case AZ_TYPE_UINT64:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint64_v)) return 0;
				break;
			case AZ_TYPE_FLOAT:
				if (!native_frame_push_fpr32 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->float_v)) return 0;
				break;
			case AZ_TYPE_DOUBLE:
				if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->double_v)) return 0;
				break;
			case AZ_TYPE_COMPLEX_FLOAT:
				/* HFA of 2 floats - either both members in registers or everything on the stack */
				if (n_fpr + 2 <= 8) {
					if (!native_frame_push_fpr32 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cfloat_v.c[0])) return 0;
					if (!native_frame_push_fpr32 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cfloat_v.c[1])) return 0;
				} else {
					if (!native_frame_push_stack (&frame, &stack_bytes, &arg_vals[i]->cfloat_v, 8)) return 0;
				}
				break;
			case AZ_TYPE_COMPLEX_DOUBLE:
				/* HFA of 2 doubles - either both members in registers or everything on the stack */
				if (n_fpr + 2 <= 8) {
					if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cdouble_v.c[0])) return 0;
					if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cdouble_v.c[1])) return 0;
				} else {
					if (!native_frame_push_stack (&frame, &stack_bytes, &arg_vals[i]->cdouble_v, 16)) return 0;
				}
				break;
			case AZ_TYPE_POINTER:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->pointer_v)) return 0;
				break;
			}
		} else if (AZ_TYPE_IS_FINAL (type)) {
			/* Final types - [pointer] */
			uint64_t p = (AZ_TYPE_IS_BLOCK (type)) ? (uint64_t) (uintptr_t) arg_vals[i]->block : (uint64_t) (uintptr_t) arg_vals[i];
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, p)) return 0;
		} else {
			/* Non-final types - [impl, pointer] */
			const AZImplementation *impl = arg_impls[i];
			if (!impl) impl = &klass->impl;
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) impl)) return 0;
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) az_value_get_inst (impl, arg_vals[i]))) return 0;
		}
	}

	az_native_call_frame_arm64 (func, &frame, stack_bytes, &result);

	if (!sig->ret_type) {
		if (ret_impl) *ret_impl = NULL;
		return 1;
	}
	rtype = sig->ret_type;
	rklass = AZ_CLASS_FROM_TYPE (rtype);
	if (AZ_TYPE_IS_OBJECT (rtype)) {
		/* Objects - returned by pointer */
		AZObject *obj = (AZObject *) (uintptr_t) result.gpr;
		ret_val->value.block = obj;
		if (ret_impl) *ret_impl = (obj) ? (const AZImplementation *) obj->klass : &rklass->impl;
	} else if (AZ_TYPE_IS_PRIMITIVE (rtype)) {
		/* Primitives - returned by value */
		if (ret_impl) *ret_impl = &rklass->impl;
		switch (rtype) {
		case AZ_TYPE_BOOLEAN:
			ret_val->value.boolean_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT8:
			ret_val->value.int8_v = (int8_t) result.gpr;
			break;
		case AZ_TYPE_UINT8:
			ret_val->value.uint8_v = (uint8_t) result.gpr;
			break;
		case AZ_TYPE_INT16:
			ret_val->value.int16_v = (int16_t) result.gpr;
			break;
		case AZ_TYPE_UINT16:
			ret_val->value.uint16_v = (uint16_t) result.gpr;
			break;
		case AZ_TYPE_INT32:
			ret_val->value.int32_v = (int32_t) result.gpr;
			break;
		case AZ_TYPE_UINT32:
			ret_val->value.uint32_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT64:
			ret_val->value.int64_v = (int64_t) result.gpr;
			break;
		case AZ_TYPE_UINT64:
			ret_val->value.uint64_v = result.gpr;
			break;
		case AZ_TYPE_FLOAT: {
			uint32_t u = (uint32_t) result.fprs[0];
			memcpy (&ret_val->value.float_v, &u, 4);
			break;
		}
		case AZ_TYPE_DOUBLE:
			memcpy (&ret_val->value.double_v, &result.fprs[0], 8);
			break;
		case AZ_TYPE_COMPLEX_FLOAT: {
			uint32_t r = (uint32_t) result.fprs[0];
			uint32_t j = (uint32_t) result.fprs[1];
			memcpy (&ret_val->value.cfloat_v.c[0], &r, 4);
			memcpy (&ret_val->value.cfloat_v.c[1], &j, 4);
			break;
		}
		case AZ_TYPE_COMPLEX_DOUBLE:
			memcpy (&ret_val->value.cdouble_v.c[0], &result.fprs[0], 8);
			memcpy (&ret_val->value.cdouble_v.c[1], &result.fprs[1], 8);
			break;
		case AZ_TYPE_POINTER:
			ret_val->value.pointer_v = (void *) (uintptr_t) result.gpr;
			break;
		}
	} else if (AZ_TYPE_IS_FINAL (rtype)) {
		/* Final blocks - returned by pointer, final values were written to the hidden storage */
		if (ret_impl) *ret_impl = &rklass->impl;
		if (AZ_TYPE_IS_BLOCK (rtype)) ret_val->value.block = (void *) (uintptr_t) result.gpr;
	} else {
		/* Non-final types - implementation returned, value was written to the hidden storage */
		if (ret_impl) *ret_impl = (const AZImplementation *) (uintptr_t) result.gpr;
	}
	return 1;
}

#elif defined(ARCH_X86_64) && defined(__GNUC__) && !defined(_WIN32) && !defined(__CYGWIN__)

/*
 * x86-64 System V (AMD64) native call
 *
 * The C code marshals the arguments into an AZNativeCallFrame (6 general purpose
 * registers, 8 SSE registers and the stack overflow area) and the assembly
 * trampoline loads the registers, copies the stack area and performs the call.
 * Two-float aggregates (complex float) occupy a single SSE register, two-double
 * aggregates (complex double) two consecutive SSE registers - if the remaining
 * registers are not sufficient everything goes to the stack. After the call the
 * trampoline stores rax, xmm0 and xmm1 into the result buffer - this covers all
 * return types of the native calling rules (integers, pointers, floats, doubles
 * and 2-element homogeneous FP aggregates). The number of SSE argument registers
 * used is passed in al to accommodate variadic native functions.
 */

#include <stddef.h>

#define AZ_NATIVE_CALL_MAX_STACK 1024

typedef struct {
	uint64_t gprs[6];
	uint64_t fprs[8];
	uint64_t n_fpr;
	uint64_t stack[AZ_NATIVE_CALL_MAX_STACK / 8];
} AZNativeCallFrame;

typedef struct {
	uint64_t gpr;
	uint64_t fprs[2];
} AZNativeCallResult;

/* The trampoline expects the stack area at offset 120 (6 gprs + 8 fprs + n_fpr) */
_Static_assert (offsetof (AZNativeCallFrame, stack) == 120, "AZNativeCallFrame stack offset has to be 120");

extern void az_native_call_frame_sysv64 (void (*func) (void), const AZNativeCallFrame *frame, uint64_t stack_bytes, AZNativeCallResult *result);

#if defined(__APPLE__)
#define AZ_NATIVE_CALL_FRAME_SYMBOL "_az_native_call_frame_sysv64"
#else
#define AZ_NATIVE_CALL_FRAME_SYMBOL "az_native_call_frame_sysv64"
#endif

__asm__(
	".text\n"
	".p2align 4\n"
	".globl " AZ_NATIVE_CALL_FRAME_SYMBOL "\n"
	AZ_NATIVE_CALL_FRAME_SYMBOL ":\n"
	/* rdi = function, rsi = frame, rdx = stack_bytes, rcx = result */
	"pushq	%rbp\n"
	"movq	%rsp, %rbp\n"
	"pushq	%rbx\n"
	"subq	$8, %rsp\n"	/* 16-byte alignment */
	"movq	%rdi, %r10\n"	/* function */
	"movq	%rsi, %r11\n"	/* frame */
	"movq	%rcx, %rbx\n"	/* result (callee-saved) */
	/* Allocate the stack argument area (16-byte aligned) */
	"leaq	15(%rdx), %rax\n"
	"andq	$-16, %rax\n"
	"subq	%rax, %rsp\n"
	/* Copy stack arguments */
	"testq	%rdx, %rdx\n"
	"jz	2f\n"
	"leaq	120(%r11), %rcx\n"
	"movq	%rsp, %rsi\n"
	"1:\n"
	"movq	(%rcx), %rax\n"
	"movq	%rax, (%rsi)\n"
	"addq	$8, %rcx\n"
	"addq	$8, %rsi\n"
	"subq	$8, %rdx\n"
	"jnz	1b\n"
	"2:\n"
	/* Load floating point argument registers */
	"movq	48(%r11), %xmm0\n"
	"movq	56(%r11), %xmm1\n"
	"movq	64(%r11), %xmm2\n"
	"movq	72(%r11), %xmm3\n"
	"movq	80(%r11), %xmm4\n"
	"movq	88(%r11), %xmm5\n"
	"movq	96(%r11), %xmm6\n"
	"movq	104(%r11), %xmm7\n"
	/* Load general purpose argument registers */
	"movq	0(%r11), %rdi\n"
	"movq	8(%r11), %rsi\n"
	"movq	16(%r11), %rdx\n"
	"movq	24(%r11), %rcx\n"
	"movq	32(%r11), %r8\n"
	"movq	40(%r11), %r9\n"
	/* al = the number of SSE registers used */
	"movq	112(%r11), %rax\n"
	/* Call the native function */
	"call	*%r10\n"
	/* Store the result registers */
	"movq	%rax, 0(%rbx)\n"
	"movq	%xmm0, 8(%rbx)\n"
	"movq	%xmm1, 16(%rbx)\n"
	/* Tear down the frame */
	"leaq	-8(%rbp), %rsp\n"
	"popq	%rbx\n"
	"popq	%rbp\n"
	"ret\n"
);

static unsigned int
native_frame_push_gpr (AZNativeCallFrame *frame, unsigned int *n_gpr, unsigned int *stack_bytes, uint64_t val)
{
	if (*n_gpr < 6) {
		frame->gprs[(*n_gpr)++] = val;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		frame->stack[*stack_bytes / 8] = val;
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr32 (AZNativeCallFrame *frame, unsigned int *n_fpr, unsigned int *stack_bytes, float val)
{
	if (*n_fpr < 8) {
		uint32_t u;
		memcpy (&u, &val, 4);
		frame->fprs[(*n_fpr)++] = u;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 4);
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr64 (AZNativeCallFrame *frame, unsigned int *n_fpr, unsigned int *stack_bytes, double val)
{
	if (*n_fpr < 8) {
		memcpy (&frame->fprs[(*n_fpr)++], &val, 8);
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 8);
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_stack (AZNativeCallFrame *frame, unsigned int *stack_bytes, const void *data, unsigned int size)
{
	unsigned int n = (size + 7) & ~7u;
	if (*stack_bytes + n > AZ_NATIVE_CALL_MAX_STACK) return 0;
	memcpy ((uint8_t *) frame->stack + *stack_bytes, data, size);
	*stack_bytes += n;
	return 1;
}

unsigned int
az_function_call_native (void (*func) (void), const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation *arg_impls[], const AZValue *arg_vals[])
{
	AZNativeCallFrame frame;
	AZNativeCallResult result;
	AZValue64 tmp_ret;
	unsigned int n_gpr = 0, n_fpr = 0, stack_bytes = 0;
	unsigned int i, rtype;
	AZClass *rklass;

	arikkei_return_val_if_fail (func != NULL, 0);
	arikkei_return_val_if_fail (sig != NULL, 0);
	arikkei_return_val_if_fail (sig->n_args < 64, 0);

	if (!ret_val) ret_val = &tmp_ret;

	/* Hidden return storage argument */
	if (sig->ret_type && !AZ_TYPE_IS_OBJECT (sig->ret_type) && !AZ_TYPE_IS_PRIMITIVE (sig->ret_type)) {
		if (!AZ_TYPE_IS_FINAL (sig->ret_type) || !AZ_TYPE_IS_BLOCK (sig->ret_type)) {
			/* Final values and all non-final types: the first argument is a pointer to the return storage */
			void *storage = (AZ_TYPE_IS_BLOCK (sig->ret_type)) ? (void *) &ret_val->value.block : (void *) ret_val;
			if (!AZ_TYPE_IS_BLOCK (sig->ret_type)) {
				arikkei_return_val_if_fail (AZ_CLASS_FROM_TYPE (sig->ret_type)->instance_size <= AZ_FUNCTION_MAX_RETURN_VALUE_SIZE, 0);
			}
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) storage)) return 0;
		}
	}

	/* Arguments */
	for (i = 0; i < sig->n_args; i++) {
		unsigned int type = sig->arg_types[i];
		AZClass *klass = AZ_CLASS_FROM_TYPE (type);
		if (AZ_TYPE_IS_OBJECT (type)) {
			/* Objects - [pointer] */
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->block)) return 0;
		} else if (AZ_TYPE_IS_PRIMITIVE (type)) {
			/* Primitive types - [value] */
			switch (type) {
			case AZ_TYPE_BOOLEAN:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->boolean_v)) return 0;
				break;
			case AZ_TYPE_INT8:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int8_v)) return 0;
				break;
			case AZ_TYPE_UINT8:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint8_v)) return 0;
				break;
			case AZ_TYPE_INT16:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int16_v)) return 0;
				break;
			case AZ_TYPE_UINT16:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint16_v)) return 0;
				break;
			case AZ_TYPE_INT32:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uint32_t) arg_vals[i]->int32_v)) return 0;
				break;
			case AZ_TYPE_UINT32:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint32_v)) return 0;
				break;
			case AZ_TYPE_INT64:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) arg_vals[i]->int64_v)) return 0;
				break;
			case AZ_TYPE_UINT64:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, arg_vals[i]->uint64_v)) return 0;
				break;
			case AZ_TYPE_FLOAT:
				if (!native_frame_push_fpr32 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->float_v)) return 0;
				break;
			case AZ_TYPE_DOUBLE:
				if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->double_v)) return 0;
				break;
			case AZ_TYPE_COMPLEX_FLOAT:
				/* Two-float aggregate - both members in a single SSE register or on the stack */
				if (n_fpr < 8) {
					memcpy (&frame.fprs[n_fpr++], &arg_vals[i]->cfloat_v, 8);
				} else {
					if (!native_frame_push_stack (&frame, &stack_bytes, &arg_vals[i]->cfloat_v, 8)) return 0;
				}
				break;
			case AZ_TYPE_COMPLEX_DOUBLE:
				/* Two-double aggregate - members in two consecutive SSE registers or everything on the stack */
				if (n_fpr + 2 <= 8) {
					if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cdouble_v.c[0])) return 0;
					if (!native_frame_push_fpr64 (&frame, &n_fpr, &stack_bytes, arg_vals[i]->cdouble_v.c[1])) return 0;
				} else {
					if (!native_frame_push_stack (&frame, &stack_bytes, &arg_vals[i]->cdouble_v, 16)) return 0;
				}
				break;
			case AZ_TYPE_POINTER:
				if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->pointer_v)) return 0;
				break;
			}
		} else if (AZ_TYPE_IS_FINAL (type)) {
			/* Final types - [pointer] */
			uint64_t p = (AZ_TYPE_IS_BLOCK (type)) ? (uint64_t) (uintptr_t) arg_vals[i]->block : (uint64_t) (uintptr_t) arg_vals[i];
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, p)) return 0;
		} else {
			/* Non-final types - [impl, pointer] */
			const AZImplementation *impl = arg_impls[i];
			if (!impl) impl = &klass->impl;
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) impl)) return 0;
			if (!native_frame_push_gpr (&frame, &n_gpr, &stack_bytes, (uint64_t) (uintptr_t) az_value_get_inst (impl, arg_vals[i]))) return 0;
		}
	}

	frame.n_fpr = n_fpr;
	az_native_call_frame_sysv64 (func, &frame, stack_bytes, &result);

	if (!sig->ret_type) {
		if (ret_impl) *ret_impl = NULL;
		return 1;
	}
	rtype = sig->ret_type;
	rklass = AZ_CLASS_FROM_TYPE (rtype);
	if (AZ_TYPE_IS_OBJECT (rtype)) {
		/* Objects - returned by pointer */
		AZObject *obj = (AZObject *) (uintptr_t) result.gpr;
		ret_val->value.block = obj;
		if (ret_impl) *ret_impl = (obj) ? (const AZImplementation *) obj->klass : &rklass->impl;
	} else if (AZ_TYPE_IS_PRIMITIVE (rtype)) {
		/* Primitives - returned by value */
		if (ret_impl) *ret_impl = &rklass->impl;
		switch (rtype) {
		case AZ_TYPE_BOOLEAN:
			ret_val->value.boolean_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT8:
			ret_val->value.int8_v = (int8_t) result.gpr;
			break;
		case AZ_TYPE_UINT8:
			ret_val->value.uint8_v = (uint8_t) result.gpr;
			break;
		case AZ_TYPE_INT16:
			ret_val->value.int16_v = (int16_t) result.gpr;
			break;
		case AZ_TYPE_UINT16:
			ret_val->value.uint16_v = (uint16_t) result.gpr;
			break;
		case AZ_TYPE_INT32:
			ret_val->value.int32_v = (int32_t) result.gpr;
			break;
		case AZ_TYPE_UINT32:
			ret_val->value.uint32_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT64:
			ret_val->value.int64_v = (int64_t) result.gpr;
			break;
		case AZ_TYPE_UINT64:
			ret_val->value.uint64_v = result.gpr;
			break;
		case AZ_TYPE_FLOAT: {
			uint32_t u = (uint32_t) result.fprs[0];
			memcpy (&ret_val->value.float_v, &u, 4);
			break;
		}
		case AZ_TYPE_DOUBLE:
			memcpy (&ret_val->value.double_v, &result.fprs[0], 8);
			break;
		case AZ_TYPE_COMPLEX_FLOAT: {
			/* Two-float aggregate - returned in a single SSE register (r in the low dword, i in the high) */
			uint32_t r = (uint32_t) result.fprs[0];
			uint32_t j = (uint32_t) (result.fprs[0] >> 32);
			memcpy (&ret_val->value.cfloat_v.c[0], &r, 4);
			memcpy (&ret_val->value.cfloat_v.c[1], &j, 4);
			break;
		}
		case AZ_TYPE_COMPLEX_DOUBLE:
			memcpy (&ret_val->value.cdouble_v.c[0], &result.fprs[0], 8);
			memcpy (&ret_val->value.cdouble_v.c[1], &result.fprs[1], 8);
			break;
		case AZ_TYPE_POINTER:
			ret_val->value.pointer_v = (void *) (uintptr_t) result.gpr;
			break;
		}
	} else if (AZ_TYPE_IS_FINAL (rtype)) {
		/* Final blocks - returned by pointer, final values were written to the hidden storage */
		if (ret_impl) *ret_impl = &rklass->impl;
		if (AZ_TYPE_IS_BLOCK (rtype)) ret_val->value.block = (void *) (uintptr_t) result.gpr;
	} else {
		/* Non-final types - implementation returned, value was written to the hidden storage */
		if (ret_impl) *ret_impl = (const AZImplementation *) (uintptr_t) result.gpr;
	}
	return 1;
}

#elif defined(AZ_NATIVE_CALL_WIN64)

/*
 * Windows x64 (Microsoft x64 calling convention) native call
 *
 * The C code marshals the arguments into an AZNativeCallFrame (4 general purpose
 * registers, 4 FP registers and the stack overflow area) and the MASM trampoline
 * loads the registers, copies the stack area and performs the call. Integer and
 * floating point arguments share the same four argument slots - the register used
 * (rcx/rdx/r8/r9 or xmm0-xmm3) depends on the position and type of the argument.
 * Aggregates of 1, 2, 4 or 8 bytes (complex float) are passed and returned by value
 * as if they were integers, larger aggregates (complex double) are passed by pointer
 * and returned through a hidden storage pointer (the first argument). After the call
 * the trampoline stores rax and xmm0 into the result buffer - this covers all return
 * types of the native calling rules (integers, pointers, floats, doubles and 8-byte
 * aggregates).
 */

#include <stddef.h>

#define AZ_NATIVE_CALL_MAX_STACK 1024

typedef struct {
	uint64_t gprs[4];
	uint64_t fprs[4];
	uint64_t stack[AZ_NATIVE_CALL_MAX_STACK / 8];
} AZNativeCallFrame;

typedef struct {
	uint64_t gpr;
	uint64_t fpr;
} AZNativeCallResult;

/* The trampoline expects the stack area at offset 64 (4 gprs + 4 fprs) */
_Static_assert (offsetof (AZNativeCallFrame, stack) == 64, "AZNativeCallFrame stack offset has to be 64");

extern void az_native_call_frame_win64 (void (*func) (void), const AZNativeCallFrame *frame, uint64_t stack_bytes, AZNativeCallResult *result);

static unsigned int
native_frame_push_gpr (AZNativeCallFrame *frame, unsigned int *n_slots, unsigned int *stack_bytes, uint64_t val)
{
	if (*n_slots < 4) {
		frame->gprs[(*n_slots)++] = val;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		frame->stack[*stack_bytes / 8] = val;
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr32 (AZNativeCallFrame *frame, unsigned int *n_slots, unsigned int *stack_bytes, float val)
{
	if (*n_slots < 4) {
		uint32_t u;
		memcpy (&u, &val, 4);
		frame->fprs[(*n_slots)++] = u;
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memset ((uint8_t *) frame->stack + *stack_bytes, 0, 8);
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 4);
		*stack_bytes += 8;
	}
	return 1;
}

static unsigned int
native_frame_push_fpr64 (AZNativeCallFrame *frame, unsigned int *n_slots, unsigned int *stack_bytes, double val)
{
	if (*n_slots < 4) {
		memcpy (&frame->fprs[(*n_slots)++], &val, 8);
	} else {
		if (*stack_bytes + 8 > AZ_NATIVE_CALL_MAX_STACK) return 0;
		memcpy ((uint8_t *) frame->stack + *stack_bytes, &val, 8);
		*stack_bytes += 8;
	}
	return 1;
}

unsigned int
az_function_call_native (void (*func) (void), const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation *arg_impls[], const AZValue *arg_vals[])
{
	AZNativeCallFrame frame;
	AZNativeCallResult result;
	AZValue64 tmp_ret;
	unsigned int n_slots = 0, stack_bytes = 0;
	unsigned int i, rtype;
	AZClass *rklass;

	arikkei_return_val_if_fail (func != NULL, 0);
	arikkei_return_val_if_fail (sig != NULL, 0);
	arikkei_return_val_if_fail (sig->n_args < 64, 0);

	if (!ret_val) ret_val = &tmp_ret;

	/* Hidden return storage argument */
	if (sig->ret_type) {
		if (sig->ret_type == AZ_TYPE_COMPLEX_DOUBLE) {
			/* 16-byte aggregate - returned through a hidden storage pointer by the ABI */
			if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) ret_val)) return 0;
		} else if (!AZ_TYPE_IS_OBJECT (sig->ret_type) && !AZ_TYPE_IS_PRIMITIVE (sig->ret_type)) {
			if (!AZ_TYPE_IS_FINAL (sig->ret_type) || !AZ_TYPE_IS_BLOCK (sig->ret_type)) {
				/* Final values and all non-final types: the first argument is a pointer to the return storage */
				void *storage = (AZ_TYPE_IS_BLOCK (sig->ret_type)) ? (void *) &ret_val->value.block : (void *) ret_val;
				if (!AZ_TYPE_IS_BLOCK (sig->ret_type)) {
					arikkei_return_val_if_fail (AZ_CLASS_FROM_TYPE (sig->ret_type)->instance_size <= AZ_FUNCTION_MAX_RETURN_VALUE_SIZE, 0);
				}
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) storage)) return 0;
			}
		}
	}

	/* Arguments */
	for (i = 0; i < sig->n_args; i++) {
		unsigned int type = sig->arg_types[i];
		AZClass *klass = AZ_CLASS_FROM_TYPE (type);
		if (AZ_TYPE_IS_OBJECT (type)) {
			/* Objects - [pointer] */
			if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->block)) return 0;
		} else if (AZ_TYPE_IS_PRIMITIVE (type)) {
			/* Primitive types - [value] */
			switch (type) {
			case AZ_TYPE_BOOLEAN:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, arg_vals[i]->boolean_v)) return 0;
				break;
			case AZ_TYPE_INT8:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int8_v)) return 0;
				break;
			case AZ_TYPE_UINT8:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, arg_vals[i]->uint8_v)) return 0;
				break;
			case AZ_TYPE_INT16:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uint32_t) (int32_t) arg_vals[i]->int16_v)) return 0;
				break;
			case AZ_TYPE_UINT16:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, arg_vals[i]->uint16_v)) return 0;
				break;
			case AZ_TYPE_INT32:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uint32_t) arg_vals[i]->int32_v)) return 0;
				break;
			case AZ_TYPE_UINT32:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, arg_vals[i]->uint32_v)) return 0;
				break;
			case AZ_TYPE_INT64:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) arg_vals[i]->int64_v)) return 0;
				break;
			case AZ_TYPE_UINT64:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, arg_vals[i]->uint64_v)) return 0;
				break;
			case AZ_TYPE_FLOAT:
				if (!native_frame_push_fpr32 (&frame, &n_slots, &stack_bytes, arg_vals[i]->float_v)) return 0;
				break;
			case AZ_TYPE_DOUBLE:
				if (!native_frame_push_fpr64 (&frame, &n_slots, &stack_bytes, arg_vals[i]->double_v)) return 0;
				break;
			case AZ_TYPE_COMPLEX_FLOAT: {
				/* 8-byte aggregate - passed by value as if it was an integer */
				uint64_t u;
				memcpy (&u, &arg_vals[i]->cfloat_v, 8);
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, u)) return 0;
				break;
			}
			case AZ_TYPE_COMPLEX_DOUBLE:
				/* 16-byte aggregate - passed by pointer */
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) &arg_vals[i]->cdouble_v)) return 0;
				break;
			case AZ_TYPE_POINTER:
				if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) arg_vals[i]->pointer_v)) return 0;
				break;
			}
		} else if (AZ_TYPE_IS_FINAL (type)) {
			/* Final types - [pointer] */
			uint64_t p = (AZ_TYPE_IS_BLOCK (type)) ? (uint64_t) (uintptr_t) arg_vals[i]->block : (uint64_t) (uintptr_t) arg_vals[i];
			if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, p)) return 0;
		} else {
			/* Non-final types - [impl, pointer] */
			const AZImplementation *impl = arg_impls[i];
			if (!impl) impl = &klass->impl;
			if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) impl)) return 0;
			if (!native_frame_push_gpr (&frame, &n_slots, &stack_bytes, (uint64_t) (uintptr_t) az_value_get_inst (impl, arg_vals[i]))) return 0;
		}
	}

	az_native_call_frame_win64 (func, &frame, stack_bytes, &result);

	if (!sig->ret_type) {
		if (ret_impl) *ret_impl = NULL;
		return 1;
	}
	rtype = sig->ret_type;
	rklass = AZ_CLASS_FROM_TYPE (rtype);
	if (AZ_TYPE_IS_OBJECT (rtype)) {
		/* Objects - returned by pointer */
		AZObject *obj = (AZObject *) (uintptr_t) result.gpr;
		ret_val->value.block = obj;
		if (ret_impl) *ret_impl = (obj) ? (const AZImplementation *) obj->klass : &rklass->impl;
	} else if (AZ_TYPE_IS_PRIMITIVE (rtype)) {
		/* Primitives - returned by value */
		if (ret_impl) *ret_impl = &rklass->impl;
		switch (rtype) {
		case AZ_TYPE_BOOLEAN:
			ret_val->value.boolean_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT8:
			ret_val->value.int8_v = (int8_t) result.gpr;
			break;
		case AZ_TYPE_UINT8:
			ret_val->value.uint8_v = (uint8_t) result.gpr;
			break;
		case AZ_TYPE_INT16:
			ret_val->value.int16_v = (int16_t) result.gpr;
			break;
		case AZ_TYPE_UINT16:
			ret_val->value.uint16_v = (uint16_t) result.gpr;
			break;
		case AZ_TYPE_INT32:
			ret_val->value.int32_v = (int32_t) result.gpr;
			break;
		case AZ_TYPE_UINT32:
			ret_val->value.uint32_v = (uint32_t) result.gpr;
			break;
		case AZ_TYPE_INT64:
			ret_val->value.int64_v = (int64_t) result.gpr;
			break;
		case AZ_TYPE_UINT64:
			ret_val->value.uint64_v = result.gpr;
			break;
		case AZ_TYPE_FLOAT: {
			uint32_t u = (uint32_t) result.fpr;
			memcpy (&ret_val->value.float_v, &u, 4);
			break;
		}
		case AZ_TYPE_DOUBLE:
			memcpy (&ret_val->value.double_v, &result.fpr, 8);
			break;
		case AZ_TYPE_COMPLEX_FLOAT: {
			/* 8-byte aggregate - returned in rax (r in the low dword, i in the high) */
			uint32_t r = (uint32_t) result.gpr;
			uint32_t j = (uint32_t) (result.gpr >> 32);
			memcpy (&ret_val->value.cfloat_v.c[0], &r, 4);
			memcpy (&ret_val->value.cfloat_v.c[1], &j, 4);
			break;
		}
		case AZ_TYPE_COMPLEX_DOUBLE:
			/* 16-byte aggregate - written to the hidden storage */
			break;
		case AZ_TYPE_POINTER:
			ret_val->value.pointer_v = (void *) (uintptr_t) result.gpr;
			break;
		}
	} else if (AZ_TYPE_IS_FINAL (rtype)) {
		/* Final blocks - returned by pointer, final values were written to the hidden storage */
		if (ret_impl) *ret_impl = &rklass->impl;
		if (AZ_TYPE_IS_BLOCK (rtype)) ret_val->value.block = (void *) (uintptr_t) result.gpr;
	} else {
		/* Non-final types - implementation returned, value was written to the hidden storage */
		if (ret_impl) *ret_impl = (const AZImplementation *) (uintptr_t) result.gpr;
	}
	return 1;
}

#else

unsigned int
az_function_call_native (void (*func) (void), const AZFunctionSignature *sig, const AZImplementation **ret_impl, AZValue64 *ret_val, const AZImplementation *arg_impls[], const AZValue *arg_vals[])
{
	fprintf (stderr, "az_function_call_native is not implemented for this architecture\n");
	return 0;
}

#endif
