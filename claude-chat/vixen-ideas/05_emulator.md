# Vixen Virtual Machine — Emulator Implementation Notes

---

## Architecture Overview

```
+------------------+
|   Assembler      |  source.vxm → binary ROM image
|   (encoding)     |
+------------------+
         ↓ binary file
+------------------+
|   CPU Emulator   |  fetch → decode → execute loop
|   (decoding)     |
+------------------+
         ↓ memory access
+------------------+
|   Address        |  routes addresses to correct destination
|   Translator     |
+------------------+
         ↓
    +----+----+-------+
    |    |    |       |
   ROM  RAM  Devices (MMIO)
```

---

## The Physical Bus Model

The emulator models the hardware buses as shared global state,
faithful to how real hardware works:

```c
uint64_t addressBus;    // driven by CPU
uint64_t dataBus;       // driven by CPU (write) or device (read)
uint8_t  rwLine;        // 1 = READ, 0 = WRITE
uint8_t  clockLine;     // drives everything
```

The `dataBus` is intentionally a shared global — this mirrors the physical
reality of a shared bus that every device reads and writes.

---

## The Clock

Everything is driven by the clock. Nothing happens outside a clock tick:

```c
void clock_tick() {
    CPU_drive_buses();      // CPU sets address and R/W
    AddressTranslator();    // devices respond
    CPU_latch_data();       // CPU reads result
}

void run() {
    while(1) {
        clock_tick();
    }
}
```

---

## The Address Translator

Routes memory accesses to the correct device based on address range.
This is the software equivalent of address decoding hardware.

```c
#define ROM_START   0xFF00
#define ROM_END     0xFFFF
#define RAM_START   0x0000
#define RAM_END     0xDFFF
#define MMIO_START  0xE000
#define MMIO_END    0xEFFF

void AddressTranslator() {
    if (addressBus >= ROM_START && addressBus <= ROM_END) {
        if (rwLine) {
            // READ — ROM drives data bus
            dataBus = ROM[addressBus - ROM_START];
        } else {
            // WRITE to ROM — error
            flags.memory_error = 1;
        }
    }
    else if (addressBus >= RAM_START && addressBus <= RAM_END) {
        if (rwLine) {
            dataBus = RAM[addressBus - RAM_START];
        } else {
            RAM[addressBus - RAM_START] = dataBus;
        }
    }
    else if (addressBus >= MMIO_START && addressBus <= MMIO_END) {
        if (rwLine) {
            dataBus = device_read(addressBus);
        } else {
            device_write(addressBus, dataBus);
        }
    }
    else {
        // Open bus — nothing responds
        if (rwLine) {
            dataBus = 0xFF;     // floating / undefined
        }
    }
}
```

---

## ROM Loading

The emulator accepts a binary file as the ROM image:

```
$ vixen myprogram.bin
```

The binary is loaded directly into the ROM region. Execution begins
at the reset vector address. The first bytes of the file are the
first instructions executed — no indirection required.

```c
void load_rom(const char* filename) {
    FILE* f = fopen(filename, "rb");
    fread(ROM, 1, ROM_SIZE, f);
    fclose(f);
    registers.PC = RESET_VECTOR;
}
```

---

## Registers

```c
typedef struct {
    uint64_t A;     // accumulator
    uint64_t B;     // base
    uint64_t C;     // counter
    uint64_t D;     // data (auxiliary accumulator)
    uint64_t E;     // extra
    uint64_t F;     // flags
    uint64_t PC;    // program counter
} Registers;
```

---

## Flags

```c
typedef struct {
    uint8_t sign;           // bit 7
    uint8_t frozen;         // bit 6
    uint8_t mark_error;     // bit 5
    uint8_t stack_error;    // bit 4
    uint8_t x;              // bit 3
    uint8_t y;              // bit 2
    // bit 1 reserved
    uint8_t zero;           // bit 0
} Flags;

void update_flags(uint64_t result) {
    flags.zero = (result == 0);
    flags.sign = (result >> 63) & 1;    // MSB of 64-bit result
}
```

---

## The Computation Stack

Fixed 8-slot array with a stack pointer. Not memory mapped.

```c
#define STACK_SIZE 8

uint64_t stack[STACK_SIZE];
int      stack_pointer = -1;    // -1 = empty

void stack_push(uint64_t value) {
    if (stack_pointer >= STACK_SIZE - 1) {
        flags.stack_error = 1;  // overflow
        return;
    }
    stack[++stack_pointer] = value;
}

uint64_t stack_pop() {
    if (stack_pointer < 0) {
        flags.stack_error = 1;  // underflow
        return 0;
    }
    return stack[stack_pointer--];
}

uint64_t stack_peek() {
    if (stack_pointer < 0) {
        flags.stack_error = 1;
        return 0;
    }
    return stack[stack_pointer];
}
```

---

## The Call Stack

Separate 16-slot stack for return addresses. Not accessible to programs.

```c
#define CALL_STACK_SIZE 16

uint64_t call_stack[CALL_STACK_SIZE];
int      call_stack_pointer = -1;

void call_push(uint64_t return_address) {
    if (call_stack_pointer >= CALL_STACK_SIZE - 1) {
        flags.stack_error = 1;  // call stack overflow
        return;
    }
    call_stack[++call_stack_pointer] = return_address;
}

uint64_t call_pop() {
    if (call_stack_pointer < 0) {
        flags.stack_error = 1;
        return 0;
    }
    return call_stack[call_stack_pointer--];
}
```

---

## Instruction Decoding

```c
typedef struct {
    uint8_t  operand_type;  // bits 15-12
    uint8_t  reg;           // bits 11-9
    uint8_t  opcode;        // bits 8-2
    uint64_t immediate;     // from following words if needed
} Instruction;

Instruction fetch_and_decode() {
    Instruction inst = {0};

    // Fetch first 16-bit word
    uint16_t word = fetch_word();

    inst.operand_type = (word >> 12) & 0xF;
    inst.reg          = (word >>  9) & 0x7;
    inst.opcode       = (word >>  2) & 0x7F;

    // Fetch immediate if needed
    switch(inst.operand_type) {
        case 0x1: inst.immediate = fetch_word() & 0xFF;         break; // 8-bit
        case 0x2: inst.immediate = fetch_word();                break; // 16-bit
        case 0x3: inst.immediate = fetch_u32();                 break; // 32-bit
        case 0x4: inst.immediate = fetch_u64();                 break; // 64-bit
        case 0x6: inst.immediate = fetch_word() & 0xFF;         break; // @8-bit
        case 0x7: inst.immediate = fetch_word();                break; // @16-bit
        case 0x8: inst.immediate = fetch_u32();                 break; // @32-bit
        case 0x9: inst.immediate = fetch_u64();                 break; // @64-bit
    }

    return inst;
}
```

---

## Operand Resolution

Resolves an instruction's operand to its actual value:

```c
uint64_t resolve_operand(Instruction* inst) {
    switch(inst->operand_type) {
        case 0x0: return 0;                          // no operand
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4: return inst->immediate;            // immediate value

        case 0x5:                                    // register
            switch(inst->reg) {
                case 1: return registers.A;
                case 2: return registers.B;
                case 3: return registers.C;
                case 4: return registers.D;
                case 5: return registers.E;
            }

        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9: return mem_read(registers.B + inst->immediate); // @immediate

        case 0xA:                                    // @register
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE: {
            uint64_t reg_val = resolve_register(inst->reg);
            return mem_read(registers.B + reg_val);
        }
    }
    return 0;
}
```

---

## mark / reset Behaviour

```c
void execute_mark() {
    if (flags.frozen) {
        flags.mark_error = 1;   // nested mark — fail
        return;
    }
    registers.E = registers.B;
    flags.frozen = 1;
}

void execute_reset() {
    registers.B = registers.E;
    flags.frozen = 0;           // always succeeds
}
```

---

## Suggested Build Order

1. Registers, memory arrays, address translator
2. `acc`, `base`, `counter`, `extra` — can load values
3. Arithmetic — `add`, `sub`, `mul`, `div`, `neg`
4. Flags and conditional jumps
5. Stack instructions
6. `load`, `store`, `mark`, `reset`
7. `call`, `ret`, call stack
8. Test with factorial and Fibonacci programs
9. MMIO device layer
10. Assembler

---

## Running the Emulator

```
$ vixen <rom_image.bin>
```

The binary file is loaded into the ROM region.
The PC is set to the reset vector.
Execution begins immediately.
