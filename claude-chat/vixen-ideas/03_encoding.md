# Vixen Virtual Machine — Instruction Encoding

---

## Overview

Instructions are **16-bit words**. An instruction may span multiple 16-bit
words depending on the operand size required.

---

## Single Instruction Word Layout

```
bit  15-12   bits 11-9    bits 8-2    bits 1-0
[ operand  ] [ register ] [ opcode  ] [ reserved ]
  (4 bits)     (3 bits)    (7 bits)    (2 bits)
```

---

## Operand Type Field (bits 15-12)

Describes what follows the instruction word and the addressing mode.

```
0000 — no operand, next word is next instruction
0001 — 8-bit immediate follows  (1 extra 16-bit word, value in low 8 bits)
0010 — 16-bit immediate follows (1 extra 16-bit word)
0011 — 32-bit immediate follows (2 extra 16-bit words)
0100 — 64-bit immediate follows (4 extra 16-bit words)
0101 — @register   memory[B + register]  (register in bits 11-9)
0110 — @8-bit immediate  memory[B + 8-bit offset]
0111 — @16-bit immediate memory[B + 16-bit offset]
1000 — @32-bit immediate memory[B + 32-bit offset]
1001 — @64-bit immediate memory[B + 64-bit offset]
1010 — (reserved)
1011 — (reserved)
1100 — (reserved)
1101 — (reserved)
1110 — (reserved)
1111 — continuation — next 16-bit word extends the instruction further
```

---

## Register Field (bits 11-9)

Encodes a register operand. Used when operand type is a register reference
or a memory reference via register (`@register`).

```
000 — none (no register operand)
001 — A (accumulator)
010 — B (base)
011 — C (counter)
100 — D (data)
101 — E (extra)
110 — (reserved)
111 — (reserved)
```

---

## Opcode Field (bits 8-2)

7 bits = 128 possible opcodes.

### Stack (0x01 - 0x10)
```
0000001 = 0x01  push
0000010 = 0x02  pop
0000011 = 0x03  peek
0000100 = 0x04  dup
0000101 = 0x05  swap
0000110 = 0x06  popy
0000111 = 0x07  peeky
0001000 = 0x08  dupy
0001001 = 0x09  poke
0001010 = 0x0A  drop
0001011 = 0x0B  dropy
0001100 = 0x0C  cycle
0001101 = 0x0D  clear
0001110 = 0x0E  dump
0001111 = 0x0F  count
0010000 = 0x10  cap
```

### Load Family (0x11 - 0x14)
```
0010001 = 0x11  acc
0010010 = 0x12  counter
0010011 = 0x13  base
0010100 = 0x14  extra
```

### Arithmetic and Logic (0x20 - 0x3D)
```
0100000 = 0x20  add
0100001 = 0x21  sub
0100010 = 0x22  rsub
0100011 = 0x23  mul
0100100 = 0x24  div
0100101 = 0x25  rdiv
0110000 = 0x30  neg
0110001 = 0x31  inv
0110010 = 0x32  and
0110011 = 0x33  or
0110100 = 0x34  not
0110101 = 0x35  xor
0110110 = 0x36  shl
0110111 = 0x37  shr
0111000 = 0x38  rol
0111001 = 0x39  ror
0111010 = 0x3A  xchg
0111011 = 0x3B  cmp
0111100 = 0x3C  rcmp
0111101 = 0x3D  sign
```

### Jumps (0x40 - 0x4C)
```
1000000 = 0x40  jmp
1000001 = 0x41  jz
1000010 = 0x42  jnz
1000011 = 0x43  js
1000100 = 0x44  jns
1000101 = 0x45  jx
1000110 = 0x46  jnx
1000111 = 0x47  jy
1001000 = 0x48  jny
1001001 = 0x49  jcz
1001010 = 0x4A  jcnz
1001011 = 0x4B  jaz
1001100 = 0x4C  janz
```

### Flag Control (0x50 - 0x55)
```
1010000 = 0x50  setx
1010001 = 0x51  clearx
1010010 = 0x52  togglex
1010011 = 0x53  sety
1010100 = 0x54  cleary
1010101 = 0x55  toggley
```

### Counter (0x60 - 0x62)
```
1100000 = 0x60  inc
1100001 = 0x61  dec
1100010 = 0x62  loop
```

### Memory (0x70 - 0x71)
```
1110000 = 0x70  load
1110001 = 0x71  store
```

### Accumulator Save/Restore (0x80 - 0x81)
```
0x80  pres
0x81  rest
```

### Base Save/Restore (0x82 - 0x83)
```
0x82  mark
0x83  reset
```

### Subroutines (0x90 - 0x91)
```
0x90  call
0x91  ret
```

### Miscellaneous
```
1111110 = 0x7E  nop
1111111 = 0x7F  halt   (or extended opcode escape)
```

---

## Encoding Examples

### `halt` — no operand
```
bits: 0000 000 1111111 00
hex:  0x01FC
```

### `push` — no operand
```
bits: 0000 000 0000001 00
hex:  0x0004
```

### `acc 42` — 8-bit immediate
```
Word 1: 0001 000 0010001 00  =  0x1044
Word 2: 0x002A               (42)
```

### `acc B` — register operand
```
bits: 0000 010 0010001 00
hex:  0x0444
```

### `acc @C` — memory via register
```
bits: 0101 011 0010001 00
hex:  0x56C4  (approximate, verify bit layout)
```

### `add @42` — memory via 8-bit immediate
```
Word 1: 0110 000 0100000 00  =  0x6080
Word 2: 0x002A               (offset 42)
```

### `loop label` — 16-bit target address
```
Word 1: 0010 000 1100010 00  =  0x2188
Word 2: <target address>
```

### `call 0xF000`
```
Word 1: 0010 000 1001000 00  =  0x2120  (approximate)
Word 2: 0xF000
```

---

## Absolute Value Program — Full Encoding

```asm
cmp 0       ; compare A with 0
jns done    ; skip neg if positive
neg         ; negate A
done: halt
```

```
Offset  Words           Instruction
0x0000  0x1??? 0x0000   cmp 0     (8-bit immediate 0)
0x0004  0x2??? 0x000A   jns 0x000A
0x0008  0x????          neg
0x000A  0x01FC          halt
```

---

## Future Extension

If 128 opcodes prove insufficient, opcode `0x7F` (all ones in 7-bit field)
serves as an **extended opcode escape** — the next word contains an extended
opcode, giving another 128+ opcodes without breaking existing encodings.

This mirrors x86's `0x0F` prefix escape mechanism.
