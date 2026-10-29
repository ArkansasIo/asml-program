# Tiny8 — 8-bit CPU emulator

Tiny8 is a small educational 8-bit CPU architecture implemented as a NASM x86-64 program. It emulates an 8-bit processor in software; it is not a fabricated physical CPU or a hardware-control interface.

## Architecture

- 8-bit accumulator register **A**
- 8-bit general-purpose register **B**
- 8-bit program counter (**PC**)
- 256 bytes of byte-addressable RAM
- 8-bit arithmetic wraps modulo 256
- Zero flag updated by accumulator load/arithmetic/increment/decrement operations
- Unknown opcodes stop execution safely

## Instruction set

| Opcode | Instruction | Operand | Behavior |
|---|---|---|---|
| `00` | HALT | — | Stop emulator |
| `10` | LDA_IMM | byte | A = immediate |
| `11` | LDB_IMM | byte | B = immediate |
| `20` | ADD_AB | — | A = (A + B) mod 256 |
| `21` | SUB_AB | — | A = (A - B) mod 256 |
| `30` | STA | address | RAM[address] = A |
| `31` | LDA_MEM | address | A = RAM[address] |
| `40` | OUT_A | — | Print A as unsigned decimal |
| `50` | JMP | address | PC = address |
| `51` | JZ | address | Jump if zero flag is set |
| `60` | INC_A | — | A = (A + 1) mod 256 |
| `61` | DEC_A | — | A = (A - 1) mod 256 |

Each instruction and operand occupies one byte. The program counter wraps at 256.

## Build and run

Requirements: Linux x86-64, NASM, and GNU `ld`.

From the repository root:

```sh
nasm -f elf64 asm/tiny8/tiny8.asm -o tiny8.o
ld -o tiny8 tiny8.o
./tiny8
```

The built-in demonstration computes 19 + 23 and should print:

```text
Tiny8 8-bit CPU simulator
A = 42
```

## Limitations

This is a starter emulator, not a cycle-accurate hardware model. It currently has no assembler for Tiny8 machine code, interrupts, stack, memory-mapped devices, or physical hardware implementation. The NASM source targets a Linux x86-64 host.
