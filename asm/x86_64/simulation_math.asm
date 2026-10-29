; Lithography Control Studio — x86-64 NASM simulation math helpers
; ABI: System V AMD64 (Linux/macOS), NASM syntax.
; Scope: pure numerical helpers for offline simulation only.
; These routines do not access hardware, ports, devices, or vendor protocols.
;
; Build object on x86-64 Linux:
;   nasm -f elf64 asm/x86_64/simulation_math.asm -o simulation_math.o
;
; C ABI:
;   double lcs_asm_dot3(const double* a, const double* b);
;   double lcs_asm_norm3(const double* v);
;   double lcs_asm_clamp(double value, double lower, double upper);
;
; NOTE: norm3 uses sqrtss/sqrtsd and assumes SSE2, which is baseline on x86-64.
; Inputs are expected to point to three consecutive IEEE-754 doubles.

default rel
bits 64

section .text
global lcs_asm_dot3
global lcs_asm_norm3
global lcs_asm_clamp

; double lcs_asm_dot3(const double* a [RDI], const double* b [RSI])
; Returns a[0]*b[0] + a[1]*b[1] + a[2]*b[2] in XMM0.
lcs_asm_dot3:
    movsd xmm0, [rdi]
    mulsd xmm0, [rsi]

    movsd xmm1, [rdi + 8]
    mulsd xmm1, [rsi + 8]
    addsd xmm0, xmm1

    movsd xmm1, [rdi + 16]
    mulsd xmm1, [rsi + 16]
    addsd xmm0, xmm1
    ret

; double lcs_asm_norm3(const double* v [RDI])
; Returns sqrt(v[0]^2 + v[1]^2 + v[2]^2) in XMM0.
; This is a simulation convenience function, not a safety-rated calculation.
lcs_asm_norm3:
    movsd xmm0, [rdi]
    mulsd xmm0, xmm0

    movsd xmm1, [rdi + 8]
    mulsd xmm1, xmm1
    addsd xmm0, xmm1

    movsd xmm1, [rdi + 16]
    mulsd xmm1, xmm1
    addsd xmm0, xmm1

    sqrtsd xmm0, xmm0
    ret

; double lcs_asm_clamp(double value [XMM0], double lower [XMM1],
;                      double upper [XMM2])
; Returns lower when value < lower, upper when value > upper, otherwise value.
; NaN is passed through unchanged by the unordered comparisons.
lcs_asm_clamp:
    ucomisd xmm0, xmm0
    jp .done                 ; preserve NaN
    ucomisd xmm0, xmm1
    jb .use_lower
    ucomisd xmm0, xmm2
    ja .use_upper
.done:
    ret
.use_lower:
    movapd xmm0, xmm1
    ret
.use_upper:
    movapd xmm0, xmm2
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
