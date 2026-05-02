#pragma once


// Enum defining the type of transaction.
enum class TransactionType : bool {
    READ = false,
    WRITE = true
};


// Enum defining the Data Width Mask
enum class DataWidthMask : uint64_t {
    BYTE  =               0xFF,  // 8 bits
    WORD  =             0xFFFF,  // 16 bits
    DWORD =         0xFFFFFFFF,  // 32 bits
    QWORD = 0xFFFFFFFFFFFFFFFF   // 64 bits
};


// Struct definining a memory transaction.
struct MemoryTransaction {
    uint64_t        address;
    uint64_t        data;
    DataWidthMask   data_width_mask; // Mask indicating the width of the data to read/write
    TransactionType control;  // READ for read, WRITE for write.
};


// Fabric writers implement this namespace.
namespace Fabric {
    void     init(uint64_t rom_base, uint64_t rom_size);
    uint64_t execute(MemoryTransaction transaction);
};


// Cpu writers implement this namespace.
namespace Cpu {
    void init(uint64_t reset_vector);
    void power_on();
};