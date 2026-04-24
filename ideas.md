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

The `A` register is the accumulator register. This register stores the results of all arithmetic and logic operations. It is the first operand of all binary operations and the only operand of unary operations. The following are the arithmetic and logic intructions. For example `add x` instruction adds x to the accumulator and leaves the result in the accumulator. 

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
13.  
