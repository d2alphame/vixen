# Vixen Virtual Machine — Instruction Set Reference

The accumulator (A) is the implicit first operand and destination for all
binary arithmetic and logic operations. All arithmetic and logic instructions
automatically set the **zero** and **sign** flags.

Load instructions (`acc`, `base`, `counter`, `extra`) do **not** touch flags.

---

## Stack Instructions

Stack operations that return a value place the result in **A**.

| Instruction | Description                                                        |
|-------------|--------------------------------------------------------------------|
| `push`      | Push A onto the stack                                              |
| `pop`       | Remove top of stack, return in A                                   |
| `peek`      | Return top of stack in A without removing it                       |
| `dup`       | Duplicate top of stack onto stack                                  |
| `swap`      | Swap top two items on stack                                        |
| `popy`      | Remove next-to-top item, return in A                               |
| `peeky`     | Return next-to-top item in A without removing it                   |
| `dupy`      | Duplicate next-to-top item onto stack, top remains on top          |
| `poke`      | Duplicate next-to-top item onto top of stack                       |
| `drop`      | Remove and discard top of stack                                    |
| `dropy`     | Remove and discard next-to-top item                                |
| `cycle`     | Cycle top 3 items — deepest comes to top                           |
| `clear`     | Clear the entire stack                                             |
| `dump`      | Copy entire stack into memory (non-destructive)                    |
| `count`     | Return number of items on stack in A                               |
| `cap`       | Return total stack capacity in A                                   |

---

## Load Instructions

Load any operand into the destination register.
Operand can be a literal, a register, or a memory reference `@offset`.

| Instruction     | Description              |
|-----------------|--------------------------|
| `acc <op>`      | A = op                   |
| `base <op>`     | B = op                   |
| `counter <op>`  | C = op                   |
| `extra <op>`    | E = op                   |

**Examples:**
```asm
acc 42          ; A = 42             (literal)
acc B           ; A = B              (register)
acc @42         ; A = memory[B + 42] (memory, literal offset)
acc @C          ; A = memory[B + C]  (memory, register offset)
base A          ; B = A
counter 10      ; C = 10
extra 4         ; E = 4
```

---

## Memory Instructions

| Instruction | Description                          |
|-------------|--------------------------------------|
| `load`      | E = memory[B]  (absolute dereference)|
| `store`     | memory[B] = A                        |

---

## Arithmetic and Logic Instructions

A is always the first operand and the destination.

| Instruction  | Description                                         |
|--------------|-----------------------------------------------------|
| `add <op>`   | A = A + op                                          |
| `sub <op>`   | A = A - op                                          |
| `rsub <op>`  | A = op - A  (reverse subtract)                      |
| `neg`        | A = 0 - A                                           |
| `mul <op>`   | A = A * op                                          |
| `div <op>`   | A = A / op                                          |
| `rdiv <op>`  | A = op / A  (reverse divide)                        |
| `inv`        | A = 1 / A  (multiplicative inverse)                 |
| `and <op>`   | A = A & op  (bitwise AND)                           |
| `or <op>`    | A = A \| op  (bitwise OR)                           |
| `xor <op>`   | A = A ^ op  (bitwise XOR)                           |
| `not`        | A = ~A  (bitwise NOT)                               |
| `shl <op>`   | A = A << op                                         |
| `shr <op>`   | A = A >> op                                         |
| `rol <op>`   | Rotate A left by op bits                            |
| `ror <op>`   | Rotate A right by op bits                           |
| `xchg <op>`  | Exchange A with op                                  |
| `cmp <op>`   | Compare A with op, set flags, no change to operands |
| `rcmp <op>`  | Compare op with A (reverse)                         |
| `sign`       | Copy MSB of A into sign flag explicitly             |

---

## Counter Instructions

| Instruction | Description          |
|-------------|----------------------|
| `inc`       | C = C + 1            |
| `dec`       | C = C - 1            |

---

## Accumulator Save / Restore

| Instruction | Description     |
|-------------|-----------------|
| `pres`      | D = A           |
| `rest`      | A = D           |

---

## Base Save / Restore (Bookmark)

| Instruction | Description                                                       |
|-------------|-------------------------------------------------------------------|
| `mark`      | E = B, set frozen flag. Fails if frozen flag already set.        |
| `reset`     | B = E, clear frozen flag. Always succeeds.                       |

**Notes:**
- While the frozen flag is set, E is locked — writes to E are an error
- A nested `mark` (while frozen) sets the mark error flag and does nothing
- `reset` always works regardless of frozen flag state

---

## Flag Control Instructions

| Instruction | Description          |
|-------------|----------------------|
| `setx`      | Set x flag           |
| `clearx`    | Clear x flag         |
| `togglex`   | Toggle x flag        |
| `sety`      | Set y flag           |
| `cleary`    | Clear y flag         |
| `toggley`   | Toggle y flag        |

---

## Jump Instructions

All jumps take a target address as operand.

| Instruction  | Condition                              |
|--------------|----------------------------------------|
| `jmp <addr>` | Unconditional                          |
| `jz <addr>`  | Zero flag set                          |
| `jnz <addr>` | Zero flag clear                        |
| `js <addr>`  | Sign flag set                          |
| `jns <addr>` | Sign flag clear                        |
| `jx <addr>`  | X flag set                             |
| `jnx <addr>` | X flag clear                           |
| `jy <addr>`  | Y flag set                             |
| `jny <addr>` | Y flag clear                           |
| `jcz <addr>` | Counter register is zero               |
| `jcnz <addr>`| Counter register is not zero           |
| `jaz <addr>` | Accumulator is zero (bypasses flags)   |
| `janz <addr>`| Accumulator is not zero (bypasses flags)|

---

## Loop Instruction

| Instruction    | Description                                    |
|----------------|------------------------------------------------|
| `loop <label>` | Decrement C, jump to label if C is not zero    |

Equivalent to `dec` followed by `jcnz` but in one instruction.

---

## Subroutine Instructions

| Instruction    | Description                                              |
|----------------|----------------------------------------------------------|
| `call <addr>`  | Push PC onto call stack, jump to addr                    |
| `ret`          | Pop return address from call stack, jump to it           |

---

## Miscellaneous

| Instruction | Description              |
|-------------|--------------------------|
| `nop`       | Do nothing               |
| `halt`      | Stop execution           |
