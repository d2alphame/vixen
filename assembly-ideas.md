# Assembly Ideas
This file documents random scraps of ideas for assembly code for vixen. I'm just doing this so I don't forget.

```
; Standard instruction no operand
push

; Instruction specify a register (specify A, B, C, D, E, S, or T)
acc C    ; The operand is the counter regiter

; Instruction that specify memory offsets
counter %1234  ; Load value at memory address B + 1234 into the counter register
sub @5678      ; Subtract value at memory address 5678 from the value in acc

; Size of data may be specified
base word @2345    ; Load the 2-byte value (word) at 2345 into B register

; byte, dword, qword may also be specified in place of word. The size is qword
; by default 
```