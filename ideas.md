# Ideas for Vixen Virtual Machine
This file documents ideas I have for a simple virtual machine. It's a personal project of mine that I think would be fun.

## Architecture
The vixen vm is a hybrid accumulator machine and stack machine.

### Registers
The registers are the A to G registers.  
_A register:_ Accumulator  
_B register:_ Base  
_C register:_ Counter  
_D register:_ Data  
_E register:_ Extra  
_F register:_ Flags  
_G register:_ Global  

The `A` register is the accumulator register. This register stores the results of all arithmetic and logic operations. It is the first operand of all binary operations and the only operand of unary operations. The following are the arithmetic and logic intructions. For example `add x` instruction adds x to the accumulator and leaves the result in the accumulator. All arithmetic and logic instructions set the flags depending on the result of the operation.

#### Arithmetic and Logic Instructions
1.  **add:** Adds its operand to the accumulator.
2.  **sub:** Subtracts its operand from the accumulator.
3.  **rsub:** Subtracts the accumulator from the operand.
4.  **neg:** Negates the accumulator, i.e. acc = 0 - acc
5.  **mul:** Multiplies the accumulator by its operand
6.  **div:** Divides the accumulator by its operand
7.  **rdiv:** Divides the operand by the accumulator
8.  **inv:** Get the multiplicative inverse of the accumulator, i.e. acc = 1 / acc
9.  **and:** Bitwise `and` of accumulator with operand
10. **or:** Bitwise `or` of accumulator with operand
11. **xor:** Bitwise `xor` of accumulator with operand
12. **not:** Bitwise `not` of accumulator
13. **shl:** Shifts accumulator left by operand amount
14. **shr:** Shifts accumulator right by operand amount
15. **rol:** Rotate accumulator left by operand amount
16. **ror:** Rotate accumulator right by operand amount
17. **xchg:** Exchange operand with accumulator
18. **cmp:** Compares the accumulator to the operand. Sets registers according to results without affecting operands
19. **rcmp:** Compares the operand to the accumulator 

#### Miscallenious Instructions Targetting the Accumulator
1.  **sign:** Sets the sign flag to the most significant bit of the accumulator

### The Stack
The stack is a special scratch space with 8 slots in it - kept so small intentionally. It should be noted that the stack does not live in main memory. The stack instructions operate on the stack. Stack operations that return value place the return value in the accumulator register.

1.  **push:** Pushes the operand on to the stack
2.  **pop:** Removes the item on top of the stack and returns it in the accumulator
3.  **peek:** Returns the item on top of the stack without removing it
4.  **dup:** Duplicates the item on top of the stack on to the stack
5.  **swap:** Swaps the top 2 items on the stack
6.  **popy:** Removes the next-to-top item on the stack and returns it in the accumulator
7.  **peeky:** Returns the next-to-top item without removing it
8.  **dupy:** Duplicate the next-to-top item on to the stack. The top of stack remains on top of the stack
9.  **poke:** Duplicate the next-to-top item on to the top of the stack
10. **drop:** Removes and throws away the item on top of the stack
11. **dropy:** Removes and throws away the next-to-top item
12. **cycle:** Cycles the top 3 items on the stack. The deepest item comes up to the top of the stack
13. **clear:** Clears the stack
14. **dump:** Dumps the entire stack into memory
15. **count:** Returns the number of item on stack
16. **cap:** Return the total number of items that the stack can hold

Attempting to add items to a full stack would lead to an overflow error, and attempting to remove from an empty stack would cause an underflow error. When any of the stack errors occurs, the `Stack Error` flag is set.

### The Flags Register
The flags register has the following flags available.

- **sign:** Set according to the most significant bit of the result of an arithmetic and logic operation
- **zero:** Set if result of an operation is zero, clear if otherwise
- **carry:** Set when there's a carry or borrow from an operation
- **stack-error:** Set if a stack operation results in an error
- **x:** The `x` flag is available for programmers' use
- **y:** The `y` flag is available for programmers' use

The following instructions are that make use of the flags register. I've also added the jump instructions here.
1.  **setx:** Sets the x flag
2.  **clearx:** Clears the x flag
3.  **togglex:** Toggles the x flag
4.  **sety:** Sets the y flag
5.  **cleary:** Clears the y flag
6.  **toggley:** Toggles the y flag
7.  **jmp:** Unconditional jump
8.  **js:** Jump if sign flag is set
9.  **jns:** Jump if sign flag is clear
10. **jz:** Jump if zero flag is set
11. **jnz:** Jump if zero flag is clear
12. **jx:** Jump if x flag is set
13. **jnx:** Jump if x flag is clear
14. **jy:** Jump if y flag is set
15. **jny:** Jump if y flag is clear
16. **jcz:** Jump if counter register is zero
17. **jcnz:** Jump if counter register is not zero
18. **jaz:** Jump if accumulator is zero
19. **janz:** Jump if accumulator is not zero
20. **loop:** Decrement the counter and jump
21. **jse:** Jump on stack error. Jumps if stack-error flag is set

### The Counter Register
The counter register is meant for counting. Besides the `jcz`, `jcnz`, and `loop` instructions as specified above, the increment and decrement instructions affect the counter register.

1.  **inc:** Increment the value in the counter regiter
2.  **dec:** Decrement the value in the counter register

### Instruction For Loading Registers
The following are instructions for loading and storing values with the registers.

1.  **acc:** Loads the operand into the accumulator
2.  **base:** Loads the operand into the base register
3.  **counter:** Loads the operand into the counter register
4.  **store:** Writes the operand into memory. The address is in the B register
5.  

### Miscalleneous Instructions
1.  **nop:** Do nothing
2.  **prsv:** Preserve A. Copies the A register into the D register
3.  **rest:** Restore A. Copies the D register into the A register
4.  **mark:** Preserve B. For bookmarking a point in memory
5.  **reset:** Restore B. Restores the base register to the last bookmark

### Memory Access
Memory can be accessed by specifying an offset from the value in the base (or B) register. The offset value is a signed integer. For example:
```
add @1234  ; This adds the value at (B + 1234) to the value in the accumulator
mul @C     ; Multiply the accumulator by value at memory address B + C (the counter register)
```
When accessing memory with the offset mode, the value in the base register is not affected.

