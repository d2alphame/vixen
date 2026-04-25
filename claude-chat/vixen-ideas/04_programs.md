# Vixen Virtual Machine — Sample Programs

---

## Absolute Value

```asm
; On entry: A = n
; On exit:  A = |n|

cmp 0           ; compare A with 0
jns done        ; sign clear = positive, skip negation
neg             ; A = -A

done:
; A = |n|
```

---

## Factorial

```asm
; On entry: A = n
; On exit:  A = n!
; Note: 0! = 1 (correct if loop checks before first iteration)

counter A       ; C = n
acc 1           ; A = 1  — result accumulator

factorial:
    mul C       ; A = A * C
    loop factorial  ; dec C, jump if C != 0

; A = n!
```

---

## Square Root (Newton-Raphson)

```asm
; On entry: A = n
; On exit:  A = sqrt(n)
; Uses: stack (holds n), D (holds current guess x)

push            ; stack: [n]  — preserve n
div 2           ; A = n / 2   — initial guess
pres            ; D = x

counter 20      ; C = 20 iterations

nr_loop:
    peek        ; A = n       — recall n non-destructively
    div D       ; A = n / x
    add D       ; A = (n/x) + x
    div 2       ; A = x_next
    pres        ; D = x_next  — update guess
    loop nr_loop

drop            ; clean up stack
rest            ; A = final result
```

**The core update in 3 instructions:**
```asm
div D       ; A = n / x
add D       ; A = (n/x) + x
div 2       ; A = (n/x + x) / 2  =  x_next
```

---

## Fibonacci

```asm
; On entry: A = n
; On exit:  A = F(n)
; Uses: D (previous value), A (current value)
; Core idea: A and D leapfrog up the sequence

counter A       ; C = n
dec             ; C = n - 1  (we seed F(0) and F(1) already)
acc 0           ; A = F(0) = 0
pres            ; D = 0
add 1           ; A = F(1) = 1

fibonacci:
    add D       ; A = prev + curr = next
    xchg D      ; A = old curr, D = next
    loop fibonacci

; A = F(n)
```

**The elegant core — just 2 instructions:**
```asm
add D       ; A = prev + curr
xchg D      ; swap: A gets old curr, D gets next
```

A and D leapfrog up the Fibonacci sequence each iteration.

---

## Array Sum

```asm
; Sum all elements of an array
; On entry: B = base address of array, C = element count
; On exit:  A = sum

acc 0           ; A = 0  — initialise sum

sum:
    add @C      ; A = A + memory[B + C]
    loop sum    ; dec C, jump if C != 0

; A = sum of array elements
```

Note: accesses indices C down to 1, missing index 0.
To include index 0, initialise with `add @0` before the loop
or adjust the addressing scheme.

---

## Pointer Walk (linked list traversal)

```asm
; Walk a linked list, B points to first node
; Assumes each node's first word is a pointer to the next node
; Terminates when next pointer is 0

walk:
    load        ; E = memory[B]  — load next pointer
    acc E       ; A = next pointer
    jaz done    ; if next == 0, end of list
    base E      ; B = next node
    jmp walk

done:
    ; B points to last node
```

---

## Countdown Loop Pattern

```asm
; General pattern for a counted loop using loop instruction

counter 10      ; C = 10

my_loop:
    ; ... body ...
    loop my_loop    ; dec C, repeat if C != 0

; executes body 10 times
```

---

## Save and Restore Base During Subroutine

```asm
; Using mark/reset to safely traverse memory
; without losing the caller's base address

mark            ; E = B, freeze E  (saves caller's B)
base 0x1234     ; B = new address
; ... do memory work freely ...
reset           ; B = E, unfreeze  (restores caller's B)
```

---

## Register Usage Conventions

| Register | Typical Role in Programs           |
|----------|------------------------------------|
| A        | Working value, computation result  |
| B        | Current memory base address        |
| C        | Loop counter, iteration count      |
| D        | Save slot for A during computation |
| E        | Scratch / memory bookmark via mark |
| Stack    | Temporary values, multiple saves   |
