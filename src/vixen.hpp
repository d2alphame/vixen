#pragma once

// Enum defining the type of transaction.
enum class TransactionType : bool {
    READ = false,
    WRITE = true
};

// Enum defining the busy state of a response.
enum class ResponseBusyState : bool {
    READY = false,
    BUSY = true
};

// Enum defining the Data Width Mask
enum class DataWidthMask : uint64_t {
    BYTE  =               0xFF,  // 8 bits
    WORD  =             0xFFFF,  // 16 bits
    DWORD =         0xFFFFFFFF,  // 32 bits
    QWORD = 0xFFFFFFFFFFFFFFFF   // 64 bits
};

// Enum defining the status of a memory transaction.
enum class MemoryTransactionStatus : uint8_t {
    SUCCESS                = 0, // Transaction completed successfully
    WRITE_PERMISSION_ERROR = 1, // Attempted to write to a read-only address range
    ADDRESS_UNMAPPED_ERROR = 2, // Attempted to access an address that is not mapped
    DEVICE_ABSENT_ERROR    = 3, // The device is not present
    TIMEOUT_ERROR          = 4  // The transaction timed out 
};

// Struct definining a memory transaction.
struct MemoryTransaction {
    uint64_t        address;
    uint64_t        data;
    DataWidthMask   data_width_mask; // Mask indicating the width of the data to read/write
    TransactionType control;         // READ for read, WRITE for write.
};

// Struct defining a memory transaction response.
struct MemoryTransactionResponse {
    uint64_t        data;
    DataWidthMask   data_width_mask;  // Mask indicating the width of the data read
    MemoryTransactionStatus status;   // Status of the transaction (success or failure)
    ResponseBusyState busy;           // Ready or busy
};

// Fabric writers implement this namespace.
namespace Fabric {
    void     init(uint64_t rom_base, uint64_t rom_size);
    MemoryTransactionResponse execute(MemoryTransaction transaction);
};

// Cpu writers implement this namespace.
namespace Cpu {
    void init(uint64_t reset_vector);
    void power_on();
};