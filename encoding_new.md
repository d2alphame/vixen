# New ideas for the instruction encoding
Instructions are encoded in *32-bit* words. The following bits encode the operand. If the instruction does not take an
operand, these bits are ignored. If there are any immediate operands, no matter the size, they must follow the 
instructions.
```
31-29 (3 bits) - Register operands

000 - Operand is immediate and not a register
001 - The A register (Accumulator)
010 - The B register (Base)
011 - The C register (Counter)
100 - The D register (Data)
101 - The E register (Extra)
110 - The S register (Source)
111 - The T register (Target)

28-27 (2 bits) - Operand sizes
00 - 8 bits (low 8 bits if operand is register)
01 - 16 bits (low 16 bits if operand is a register)
10 - 32 bits (low 32 bits if operand is a register)
11 - 64 bits (all 64 bits if operand is a register)
```

For instructions that read from and write to memory, the following bits (26-25) determine the data size.
```
26-25 (2 bits) - Data sizes
00 - read/write a byte (8 bits)
01 - read/write a word (16 bits)
10 - read/write a double word (32 bits)
11 - read/write a quad word (64 bits)
```

For instructions targeting memory, bit 24 determines how to calculate the memory address
```
24 (1 bit) - Memory address calculation
0 - Calculate offset (Base address in the B register)
1 - Absolute
```

Bits 23-0 Is the actual encoding for the instructions. This gives room for over 16 million instructions. That's plenty.
```
23-0 (24 bits) - The actual instruction
```
