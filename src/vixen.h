#ifndef VIXEN_H
#define VIXEN_H

#include <stdint.h>

#define RESET_VECTOR 0xFFFFFFFFFFFF0000ULL

/**
 * SizeMask Constants
 * These define the bit-width of a transaction. 
 * Using #define ensures these can be used for bitwise masking operations
 * directly against the data_bus.
 */
#define MASK_BYTE  0xFFULL
#define MASK_WORD  0xFFFFULL
#define MASK_DWORD 0xFFFFFFFFULL
#define MASK_QWORD 0xFFFFFFFFFFFFFFFFULL

typedef enum {
    SIZE_BYTE,
    SIZE_WORD,
    SIZE_DWORD,
    SIZE_QWORD
} SizeType;

/**
 * Control Signals
 * Defines whether the transaction is moving data from the CPU to the 
 * Fabric (WRITE) or from the Fabric to the CPU (READ).
 */
typedef enum {
    MEM_READ,
    MEM_WRITE
} ControlSignal;

/**
 * MemoryTransaction Struct
 * This acts as the "Physical Bus." Instead of passing values, we pass
 * pointers to the registers/wires. The Fabric will read from or write to
 * these pointers directly.
 */
typedef struct {
    uint64_t *addr_bus;      // Pointer representing the address bus
    uint64_t *data_bus;      // Pointer representing the data bus
    SizeType size_msk;       // Enum indicating the width of the data
    ControlSignal control;    // Read or Write signal
} MemoryTransaction;

/**
 * AddressTranslator (The Fabric Interface)
 * This is the contract for modularity. Any memory controller or system
 * fabric must provide an 'execute' function that satisfies this signature.
 */
typedef struct {
    /**
     * execute
     * Performs the routing, address stripping, and data movement
     * described in the transaction packet.
     */
    void (*execute)(MemoryTransaction *tx);
} AddressTranslator;

#endif // VIXEN_H