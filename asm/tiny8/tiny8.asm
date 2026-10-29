; Tiny8 — educational 8-bit CPU emulator written in NASM x86-64
; This is a software simulator of an 8-bit architecture, not a physical CPU.
; Build (Linux x86-64, NASM):
;   nasm -f elf64 asm/tiny8/tiny8.asm -o tiny8.o
;   ld -o tiny8 tiny8.o
;   ./tiny8
;
; Tiny8 architecture:
;   8-bit A and B registers; 8-bit accumulator arithmetic wraps modulo 256.
;   8-bit PC; 256 bytes of addressable RAM.
;   Instructions:
;     0x00 HALT
;     0x10 LDA_IMM byte
;     0x11 LDB_IMM byte
;     0x20 ADD_AB       A = (A + B) & 0xff
;     0x21 SUB_AB       A = (A - B) & 0xff
;     0x30 STA address  RAM[address] = A
;     0x31 LDA_MEM addr A = RAM[address]
;     0x40 OUT_A        print A as unsigned decimal
;     0x50 JMP address  PC = address
;     0x51 JZ address   jump if zero flag set
;     0x60 INC_A        A = (A + 1) & 0xff
;     0x61 DEC_A        A = (A - 1) & 0xff
;
; Linux x86-64 System V; uses syscalls directly. The guest CPU is 8-bit,
; while the host executing this emulator is x86-64.

bits 64
default rel
global _start

%define OP_HALT   0x00
%define OP_LDAI   0x10
%define OP_LDBI   0x11
%define OP_ADD    0x20
%define OP_SUB    0x21
%define OP_STA    0x30
%define OP_LDAM   0x31
%define OP_OUTA   0x40
%define OP_JMP    0x50
%define OP_JZ     0x51
%define OP_INCA   0x60
%define OP_DECA   0x61

section .data
    banner db "Tiny8 8-bit CPU simulator", 10
    banner_len equ $ - banner
    result_prefix db "A = "
    result_prefix_len equ $ - result_prefix
    newline db 10

    ; Demo program: A=19, B=23, add, output result, halt.
    demo_program:
        db OP_LDAI, 19
        db OP_LDBI, 23
        db OP_ADD
        db OP_OUTA
        db OP_HALT
    demo_program_len equ $ - demo_program

section .bss
    ram resb 256
    reg_a resb 1
    reg_b resb 1
    pc resb 1
    zero_flag resb 1
    output_buffer resb 4

section .text
_start:
    ; Initialize 256-byte RAM and copy demo program into RAM[0..].
    lea rdi, [ram]
    xor eax, eax
    mov ecx, 256
    rep stosb

    lea rsi, [demo_program]
    lea rdi, [ram]
    mov ecx, demo_program_len
    rep movsb

    mov byte [reg_a], 0
    mov byte [reg_b], 0
    mov byte [pc], 0
    mov byte [zero_flag], 0

    mov eax, 1                  ; write banner
    mov edi, 1
    lea rsi, [banner]
    mov edx, banner_len
    syscall

.cpu_loop:
    ; Fetch instruction at RAM[PC], then advance 8-bit PC.
    movzx ebx, byte [pc]
    mov al, [ram + rbx]
    inc byte [pc]

    cmp al, OP_HALT
    je .halt
    cmp al, OP_LDAI
    je .lda_imm
    cmp al, OP_LDBI
    je .ldb_imm
    cmp al, OP_ADD
    je .add_ab
    cmp al, OP_SUB
    je .sub_ab
    cmp al, OP_STA
    je .sta
    cmp al, OP_LDAM
    je .lda_mem
    cmp al, OP_OUTA
    je .out_a
    cmp al, OP_JMP
    je .jmp
    cmp al, OP_JZ
    je .jz
    cmp al, OP_INCA
    je .inc_a
    cmp al, OP_DECA
    je .dec_a

    ; Unknown opcode: stop safely instead of executing arbitrary memory.
    jmp .halt

.lda_imm:
    call .fetch_byte
    mov [reg_a], al
    call .update_zero
    jmp .cpu_loop

.ldb_imm:
    call .fetch_byte
    mov [reg_b], al
    jmp .cpu_loop

.add_ab:
    mov al, [reg_a]
    add al, [reg_b]             ; 8-bit wraparound
    mov [reg_a], al
    call .update_zero
    jmp .cpu_loop

.sub_ab:
    mov al, [reg_a]
    sub al, [reg_b]             ; 8-bit wraparound
    mov [reg_a], al
    call .update_zero
    jmp .cpu_loop

.sta:
    call .fetch_byte
    movzx ebx, al
    mov al, [reg_a]
    mov [ram + rbx], al
    jmp .cpu_loop

.lda_mem:
    call .fetch_byte
    movzx ebx, al
    mov al, [ram + rbx]
    mov [reg_a], al
    call .update_zero
    jmp .cpu_loop

.out_a:
    ; Print "A = " followed by unsigned decimal byte and newline.
    mov eax, 1
    mov edi, 1
    lea rsi, [result_prefix]
    mov edx, result_prefix_len
    syscall

    movzx eax, byte [reg_a]
    lea rdi, [output_buffer + 3]
    mov byte [rdi], 10
    mov ecx, 1
    mov ebx, 10
    test eax, eax
    jnz .convert_digits
    dec rdi
    mov byte [rdi], '0'
    inc ecx
    jmp .write_number

.convert_digits:
    xor edx, edx
    div ebx
    add dl, '0'
    dec rdi
    mov [rdi], dl
    inc ecx
    test eax, eax
    jnz .convert_digits

.write_number:
    mov eax, 1
    mov edi, 1
    mov rsi, rdi                ; overwritten below; set buffer pointer explicitly
    lea rsi, [output_buffer + 4]
    sub rsi, rcx
    mov edx, ecx
    syscall
    jmp .cpu_loop

.jmp:
    call .fetch_byte
    mov [pc], al
    jmp .cpu_loop

.jz:
    call .fetch_byte
    cmp byte [zero_flag], 0
    je .cpu_loop
    mov [pc], al
    jmp .cpu_loop

.inc_a:
    inc byte [reg_a]
    mov al, [reg_a]
    call .update_zero
    jmp .cpu_loop

.dec_a:
    dec byte [reg_a]
    mov al, [reg_a]
    call .update_zero
    jmp .cpu_loop

.halt:
    mov eax, 60                 ; exit(0)
    xor edi, edi
    syscall

; Fetch operand at RAM[PC] into AL and advance PC.
.fetch_byte:
    movzx ebx, byte [pc]
    mov al, [ram + rbx]
    inc byte [pc]
    ret

; Set zero_flag = 1 iff AL == 0.
.update_zero:
    test al, al
    setz byte [zero_flag]
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
