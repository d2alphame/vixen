#ifndef VIXEN_H
#define VIXEN_H

#include <stdint.h>

#define RESET_VECTOR 0xFFFFFFFFFFFF0000ULL

/**
 * SizeMask Constants
 * These define the bit-width of data to to be transferred. 
 * Using #define ensures these can be used for bitwise masking operations
 * directly against the data_bus.
 */
// #define MASK_BYTE  0xFFULL
// #define MASK_WORD  0xFFFFULL
// #define MASK_DWORD 0xFFFFFFFFULL
// #define MASK_QWORD 0xFFFFFFFFFFFFFFFFULL

// typedef enum {
//     SIZE_BYTE,
//     SIZE_WORD,
//     SIZE_DWORD,
//     SIZE_QWORD
// } SizeType;

/**
 * Control Signals
 * Defines whether the transaction is moving data from the CPU to the 
 * Fabric (WRITE) or from the Fabric to the CPU (READ).
 */
typedef enum : uint8_t {
    MEM_READ,
    MEM_WRITE
} ControlSignal;

#define SIZE_BYTE  0x00000000000000FFULL
#define SIZE_WORD  0x000000000000FFFFULL
#define SIZE_DWORD 0x00000000FFFFFFFFULL
#define SIZE_QWORD 0xFFFFFFFFFFFFFFFFULL

typedef uint64_t SizeMask;

/**
 * MemoryTransaction Struct
 * This acts as the "Physical Bus." Instead of passing values, we pass
 * pointers to the registers/wires. The Fabric will read from or write to
 * these pointers directly.
 */
typedef struct {
    uint64_t *addr_bus;      // Pointer representing the address bus
    uint64_t *data_bus;      // Pointer representing the data bus
    SizeMask size_msk;       // Bitwise mask indicating the width of the data
    ControlSignal ctrl_sig;  // Indicates read or write operation
} MemoryTransaction;


/**
 * AddressTranslator (The Fabric Interface)
 * This is the contract for modularity. Any memory controller or system
 * fabric must provide an 'execute' function that satisfies this signature.
 */
typedef struct {
    /**
     * The `execute` function
     * performs the routing, address stripping, and data movement
     * described in the transaction packet.
     */
    void (*execute)(MemoryTransaction *tx);

    // Initialize the fabric with the ROM base address and size.
    void (*init)(uint64_t rom_base_addr, uint64_t rom_size);
} AddressTranslator;


/**
 * CPU (The main CPU interface)
 *  
 */
typedef struct {
    void(*init)(AddressTranslator *translator);
    void(*power_on)(uint64_t reset_vector);
} CPU;


#endif // VIXEN_H