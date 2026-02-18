; System Call Benchmark Test Program (v1.0, February 1 2026)
; Written by Ian Seyler
;
; BareMetal compile:
; nasm b_bench.asm -o b_bench.app

[BITS 64]
DEFAULT ABS

%INCLUDE "libBareMetal.asm"

start:					; Start of program label

	; Set variables
	mov r11, 1000000		; Number of iterations
	mov r12, r11			; Iteration decrement counter

	; Display iterations
	lea rsi, [rel msg_start]
	call output
	mov rax, r11
	mov rdi, msg_val
	call os_int_to_string
	mov rsi, msg_val
	call output

	; Gather start time of loop
	mov ecx, TIMECOUNTER
	call [b_system]
	mov r8, rax			; t0

loop1:
;-------------------------
; Call a kernel function
;	mov ecx, FREE_MEMORY
;	call [b_system]
;-------------------------
;	nop
;-------------------------
	xor eax, eax
	xor ecx, ecx
	cpuid
;-------------------------
	dec r12
	jnz loop1

	; Gather end time of loop
	mov ecx, TIMECOUNTER
	call [b_system]
	mov r9, rax			; t1

	; Calculate elapsed time
	sub r9, r8			; elapsed = end time (t1) - start time (t0)

	; Divide cumulative time by iterations
	xor edx, edx
	mov rax, r9
	mov rcx, r11
	div rcx				; RDX:RAX / RCX (quotient in RAX, remainder in RDX)
	push rax

	; Display average
	lea rsi, [rel msg_avg]
	call output
	pop rax
	mov rdi, msg_val
	call os_int_to_string
	mov rsi, msg_val
	call output
	lea rsi, [rel msg_ns]
	call output

	ret


; -----------------------------------------------------------------------------
; os_int_to_string -- Convert a binary integer into an string
;  IN:	RAX = binary integer
;	RDI = location to store string
; OUT:	RDI = points to end of string
;	All other registers preserved
; Min return value is 0 and max return value is 18446744073709551615 so the
; string needs to be able to store at least 21 characters (20 for the digits
; and 1 for the string terminator).
; Adapted from http://www.cs.usfca.edu/~cruse/cs210s09/rax2uint.s
os_int_to_string:
	push rdx
	push rcx
	push rbx
	push rax

	mov rbx, 10					; base of the decimal system
	xor ecx, ecx					; number of digits generated
os_int_to_string_next_divide:
	xor edx, edx					; RAX extended to (RDX,RAX)
	div rbx						; divide by the number-base
	push rdx					; save remainder on the stack
	inc rcx						; and count this remainder
	test rax, rax					; was the quotient zero?
	jnz os_int_to_string_next_divide		; no, do another division

os_int_to_string_next_digit:
	pop rax						; else pop recent remainder
	add al, '0'					; and convert to a numeral
	stosb						; store to memory-buffer
	loop os_int_to_string_next_digit		; again for other remainders
	xor al, al
	stosb						; Store the null terminator at the end of the string

	pop rax
	pop rbx
	pop rcx
	pop rdx
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
; os_string_length -- Return length of a string
;  IN:	RSI = string location
; OUT:	RCX = length (not including the NULL terminator)
;	All other registers preserved
os_string_length:
	push rdi
	push rax

	xor ecx, ecx
	xor eax, eax
	mov rdi, rsi
	not rcx
	cld
	repne scasb			; compare byte at RDI to value in AL
	not rcx
	dec rcx

	pop rax
	pop rdi
	ret
; -----------------------------------------------------------------------------


; -----------------------------------------------------------------------------
output:
	push rcx
	call os_string_length
	call [b_output]
	pop rcx
	ret
; -----------------------------------------------------------------------------


msg_start: db "Iterations: ", 0
msg_avg: db 13, 10, "Average: ", 0
msg_ns: db " ns", 0
msg_val: db 0
