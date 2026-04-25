# Vixen Virtual Machine — Architecture

Vixen is a hybrid accumulator machine and stack machine. It is designed to be
constrained but expressive — the limitations are intentional and breed creative
programming solutions.

---

## Registers

| Register | Name        | Role                                              |
|----------|-------------|---------------------------------------------------|
| A        | Accumulator | Source and destination of all arithmetic/logic    |
| B        | Base        | Base address for memory operations                |
| C        | Counter     | Loop counter (inc/dec/loop/jcz/jcnz)             |
| D        | Data        | Auxiliary accumulator — save/restore slot for A   |
| E        | Extra       | General purpose scratch / bookmark (see mark/reset)|
| F        | Flags       | Status flags (see Flags section)                  |

---

## The Stack

A dedicated 8-slot scratch space. **Does not live in main memory.**
Stack operations that return a value place the result in the **A register**.

Attempting to push onto a full stack causes **overflow**.
Attempting to pop from an empty stack causes **underflow**.
Either error sets the **stack error flag**.

---

## The Call Stack

A separate 16-slot call stack, **transparent to programmers**.
Used exclusively by `call` and `ret`. Never accessible directly.
Keeps return addresses isolated from the computation stack.

Call stack overflow sets an error flag.

---

## The Flags Register (F)

| Bit | Flag        | Meaning                                          |
|-----|-------------|--------------------------------------------------|
| 7   | sign        | Set to MSB of last arithmetic/logic result       |
| 6   | frozen      | Set when `mark` is active, E is locked           |
| 5   | mark error  | Set on nested `mark` or invalid operation        |
| 4   | stack error | Set on stack overflow or underflow               |
| 3   | x           | Programmer-controlled flag                       |
| 2   | y           | Programmer-controlled flag                       |
| 1   | (reserved)  |                                                  |
| 0   | zero        | Set when result of an operation is zero          |

**Notes:**
- The sign flag occupies the MSB of F — mirroring the result that caused it
- The zero flag occupies the LSB — the most humble bit for the most humble value
- All arithmetic and logic instructions set zero and sign flags automatically
- The `sign` instruction explicitly copies the MSB of A into the sign flag
  (useful after non-arithmetic operations like `pop` or `acc`)
- Load instructions (`acc`, `base`, `counter`, `extra`) do **not** touch flags

---

## Memory Model

- Single flat address space shared by RAM, ROM, and MMIO devices
- Address bus width: TBD (16-bit, 32-bit, or 48-bit)
- Data bus width: **64 bits**
- Word size: **64 bits**

### Memory Regions (example layout)
```
0x0000 - 0xDFFF   RAM (program and data)
0xE000 - 0xEFFF   MMIO (memory mapped devices)
0xF000 - 0xFEFF   Device descriptor table
0xFF00 - 0xFFFF   ROM (reset vector and boot code)
```

### Memory Addressing
The `B` register is the base address for all memory operations.
Offsets are specified using the `@` prefix:

```
@literal    memory[B + literal]
@register   memory[B + register]
```

### Absolute Addressing
Use `load` to dereference B directly into E:
```
base 0x1234   ; B = address
load          ; E = memory[B]
acc E         ; A = E
```

### ROM
- ROM is read-only — writes to ROM set the memory error flag
- The emulator maps a user-supplied binary file into the ROM region
- Execution begins at the reset vector address on startup

---

## Reset Vector

On power-on/reset the PC jumps to the hardcoded reset vector address.
ROM must be mapped at that address. The first bytes of ROM are the first
instructions executed. No indirection required — boot code starts directly
at the reset vector.

---

## Device Discovery

Devices register themselves in a descriptor table at a fixed known address.
Each entry describes the device type and how much address space it needs.
The VM assigns base addresses at startup. Programs read the descriptor table
to find where devices live — no hardcoded device addresses needed.

This is conceptually similar to PCI BAR negotiation.
