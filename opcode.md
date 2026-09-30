# Vixen Opcode Reference

This document consolidates the current Vixen CPU design: the register model, the 32-bit instruction encoding, and the canonical opcode assignments.

## 1. Registers

Vixen is a 64-bit hybrid accumulator/stack virtual CPU.

| Register | Name | Role |
|---|---|---|
| A | Accumulator | Main arithmetic register |
| B | Base | Base pointer for memory addressing |
| C | Counter | Loop/counter register |
| D | Data | Auxiliary accumulator / save slot |
| E | Extra | Scratch / bookmark register |
| F | Flags | Status flags |
| S | Source | Source pointer for stream memory ops |
| T | Target | Target pointer for stream memory ops |
| I | Instruction | Internal instruction register |

## 2. 32-bit instruction encoding

The newer Vixen encoding uses 32-bit instruction words.

```text
31-29 : register selector
28-27 : operand size
26-25 : memory data size
24    : memory reference flag
23    : address mode (offset vs absolute)
22-0  : opcode payload
```

### Register selector: bits 31-29

| Value | Meaning |
|---:|---|
| 000 | immediate, not a register |
| 001 | A |
| 010 | B |
| 011 | C |
| 100 | D |
| 101 | E |
| 110 | S |
| 111 | T |

### Operand size: bits 28-27

| Value | Meaning |
|---:|---|
| 00 | 8-bit operand |
| 01 | 16-bit operand |
| 10 | 32-bit operand |
| 11 | 64-bit operand |

### Memory data size: bits 26-25

| Value | Meaning |
|---:|---|
| 00 | byte |
| 01 | word |
| 10 | double word |
| 11 | quad word |

### Memory reference flag: bit 24

| Bit | Meaning |
|---:|---|
| 0 | operand is not a memory reference |
| 1 | operand is a memory reference |

### Address mode: bit 23

| Bit | Meaning |
|---:|---|
| 0 | offset relative to B |
| 1 | absolute address |

### Instruction field: bits 22-0

This gives a 23-bit opcode payload.

```text
2^23 = 8,388,608 possible instruction codes
```

The all-ones value is used for halt:

```text
0x7FFFFF = halt
```

`nop` is defined as:

```text
0x000000 = nop
```

## 3. Canonical opcode table

### Control

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000000 | `nop` | no operation |
| 0x7FFFFF | `halt` | stop execution |

### Stack

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000001 | `push` | push A |
| 0x000002 | `pop` | pop into A |
| 0x000003 | `peek` | read top without popping |
| 0x000004 | `dup` | duplicate top |
| 0x000005 | `swap` | swap top two |
| 0x000006 | `popy` | pop second item into A |
| 0x000007 | `peeky` | view second item |
| 0x000008 | `dupy` | duplicate second item |
| 0x000009 | `poke` | duplicate second onto top |
| 0x00000A | `drop` | discard top |
| 0x00000B | `dropy` | discard second |
| 0x00000C | `cycle` | rotate top three |
| 0x00000D | `clear` | clear stack |
| 0x00000E | `dump` | copy stack to memory |
| 0x00000F | `count` | count stack items |
| 0x000010 | `cap` | return stack capacity |

### Register loads

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000011 | `acc` | A = op |
| 0x000012 | `base` | B = op |
| 0x000013 | `counter` | C = op |
| 0x000014 | `extra` | E = op |
| 0x000015 | `source` | S = op |
| 0x000016 | `target` | T = op |

### Memory operations

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000017 | `load` | read memory into transfer target |
| 0x000018 | `store` | write A to memory |

### Addressing syntax

Vixen distinguishes two memory-reference forms:

- `@addr` = absolute memory address
- `%addr` = offset relative to the value in `B`

Examples:

```asm
load @0x1000
load %32
store @0x2000
store %8
```

This is the intended distinction from the older design notes: `@` is absolute, `%` is base-relative.

### Arithmetic and logic

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000019 | `add` | A = A + op |
| 0x00001A | `sub` | A = A - op |
| 0x00001B | `rsub` | A = op - A |
| 0x00001C | `neg` | A = 0 - A |
| 0x00001D | `mul` | A = A * op |
| 0x00001E | `div` | A = A / op |
| 0x00001F | `rdiv` | A = op / A |
| 0x000020 | `inv` | A = 1 / A |
| 0x000021 | `and` | bitwise AND |
| 0x000022 | `or` | bitwise OR |
| 0x000023 | `xor` | bitwise XOR |
| 0x000024 | `not` | bitwise NOT |
| 0x000025 | `shl` | shift left |
| 0x000026 | `shr` | shift right |
| 0x000027 | `rol` | rotate left |
| 0x000028 | `ror` | rotate right |
| 0x000029 | `xchg` | swap A with operand |
| 0x00002A | `cmp` | compare A to op |
| 0x00002B | `rcmp` | compare op to A |
| 0x00002C | `sign` | copy MSB of A to sign flag |

### Save / restore / bookmark

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x00002D | `pres` | D = A |
| 0x00002E | `rest` | A = D |
| 0x00002F | `mark` | preserve B in E |
| 0x000030 | `reset` | restore B from E |

### Flag operations

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000031 | `setx` | set x flag |
| 0x000032 | `clearx` | clear x flag |
| 0x000033 | `togglex` | toggle x flag |
| 0x000034 | `sety` | set y flag |
| 0x000035 | `cleary` | clear y flag |
| 0x000036 | `toggley` | toggle y flag |

### Counter / loop

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x000037 | `inc` | C = C + 1 |
| 0x000038 | `dec` | C = C - 1 |
| 0x000039 | `loop` | decrement C and jump if nonzero |

### Jumps / branches

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x00003A | `jmp` | unconditional jump |
| 0x00003B | `jz` | jump if zero flag set |
| 0x00003C | `jnz` | jump if zero flag clear |
| 0x00003D | `js` | jump if sign flag set |
| 0x00003E | `jns` | jump if sign flag clear |
| 0x00003F | `jx` | jump if x set |
| 0x000040 | `jnx` | jump if x clear |
| 0x000041 | `jy` | jump if y set |
| 0x000042 | `jny` | jump if y clear |
| 0x000043 | `jcz` | jump if C == 0 |
| 0x000044 | `jcnz` | jump if C != 0 |
| 0x000045 | `jaz` | jump if A == 0 |
| 0x000046 | `janz` | jump if A != 0 |
| 0x000047 | `jse` | jump if stack error |
| 0x000048 | `jc` | jump if carry flag set |
| 0x000049 | `jnc` | jump if carry clear |
| 0x00004A | `jemoderr` | jump if E-mod error |

### Call / return

| Opcode | Mnemonic | Meaning |
|---:|---|---|
| 0x00004B | `call` | call subroutine |
| 0x00004C | `ret` | return from subroutine |

## 4. Notes

- This is the canonical instruction set assembled from the Vixen design notes.
- Named older variants like `prsv`/`pres`, `source`, and `target` are treated as conceptually valid but should be normalized to the canonical names.
- The opcode space remains mostly free for future instructions and extensions.
- This is a design reference, not a fully implemented decoder.
