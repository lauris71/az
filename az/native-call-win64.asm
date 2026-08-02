;
; A run-time type library
;
; Copyright (C) Lauris Kaplinski 2026
;
; Windows x64 (Microsoft x64 calling convention) native call trampoline
;
; void az_native_call_frame_win64 (void (*func) (void), const AZNativeCallFrame *frame,
;                                  uint64_t stack_bytes, AZNativeCallResult *result)
;
; AZNativeCallFrame layout:
;   offset  0: 4 general purpose argument registers (rcx, rdx, r8, r9)
;   offset 32: 4 floating point argument registers (xmm0 - xmm3)
;   offset 64: the stack overflow area
; AZNativeCallResult layout:
;   offset  0: rax
;   offset  8: xmm0
;

PUBLIC az_native_call_frame_win64

.code

az_native_call_frame_win64 PROC
	; rcx = function, rdx = frame, r8 = stack_bytes, r9 = result
	push rbp
	mov rbp, rsp
	push rsi
	sub rsp, 8				; 16-byte alignment
	mov r10, rcx			; function
	mov r11, rdx			; frame
	mov rsi, r9				; result (callee-saved)
	; Allocate the shadow space (32 bytes) and the stack argument area (16-byte aligned)
	lea rax, [r8 + 47]
	and rax, -16
	sub rsp, rax
	; Copy stack arguments
	test r8, r8
	jz args_copied
	lea rcx, [r11 + 64]
	lea rdx, [rsp + 32]		; after the shadow space
copy_loop:
	mov rax, [rcx]
	mov [rdx], rax
	add rcx, 8
	add rdx, 8
	sub r8, 8
	jnz copy_loop
args_copied:
	; Load floating point argument registers
	movq xmm0, qword ptr [r11 + 32]
	movq xmm1, qword ptr [r11 + 40]
	movq xmm2, qword ptr [r11 + 48]
	movq xmm3, qword ptr [r11 + 56]
	; Load general purpose argument registers
	mov rcx, [r11]
	mov rdx, [r11 + 8]
	mov r8, [r11 + 16]
	mov r9, [r11 + 24]
	; Call the native function
	call r10
	; Store the result registers
	mov [rsi], rax
	movq qword ptr [rsi + 8], xmm0
	; Tear down the frame
	lea rsp, [rbp - 8]
	pop rsi
	pop rbp
	ret
az_native_call_frame_win64 ENDP

END
