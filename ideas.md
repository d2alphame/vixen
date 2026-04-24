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
17. **xchg** Exchange operand with accumulator

#### Miscallenious Instructions Targetting the Accumulator
1.  **sign** Sets the sign flag to the most significant bit of the accumulator

### The Stack
The stack instructions operate on the stack. Stack operations that return value place the return value in the accumulator register.
1.  **push** Pushes the operand on to the stack
2.  **pop** Removes the item on top of the stack and returns it in the accumulator
3.  **peek** Returns the item on top of the stack without removing it
4.  **dup** Duplicates the item on top of the stack on to the stack
5.  **swap** Swaps the top 2 items on the stack
6.  **

