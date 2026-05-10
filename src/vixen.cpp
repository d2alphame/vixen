// The main program for the vixen emulator

#include <iostream>
#include <cstdint>
#include <filesystem>

#include "vixen.hpp"

#define MAX_ROM_SIZE 0x200000 // 2 MB


int main(int argc, char* argv[]) {
    
    enum Result : int {
        Success                  = 0,
        InvalidSubcommand        = 1,
        InvalidFilename          = 2,
        FileNotFoundOrNotRegular = 3,
        FileTooLarge             = 4,
        ArgumentNotProvided      = 5
    };
    
    int current_index = 0; // Start from the first argument after the program name
    std::string subcommand;

    if(current_index + 1 < argc) {
        current_index += 1;
        subcommand = argv[current_index];

        std::filesystem::path rom_file;
        if(subcommand == "load-rom" ) {
            if(current_index + 1 < argc) {
                current_index += 1;
                rom_file = argv[current_index]; // This should be the ROM file name.
            }
            else {
                std::cerr << "Error: Missing ROM file name after 'load-rom' subcommand. Enter a valid ROM file name." << std::endl;
                return Result::ArgumentNotProvided;
            }
            // Check that the ROM file string is not empty
           if(rom_file.empty()) {
                std::cerr << "Error: ROM file name is empty. Enter a valid ROM file name." << std::endl;
                return Result::InvalidFilename;
            }
            // Check if the file exists and is a regular file
            if(!std::filesystem::exists(rom_file) || !std::filesystem::is_regular_file(rom_file)) {
                std::cerr << "Error: ROM file does not exist or is not a regular file. Enter a valid ROM file name." << std::endl;
                return Result::FileNotFoundOrNotRegular;
            }
            // Check if the file is too large
            if(std::filesystem::file_size(rom_file) > MAX_ROM_SIZE) {
                std::cerr << "Error: ROM file is too large. Maximum allowed size is " << MAX_ROM_SIZE << " bytes." << std::endl;
                return Result::FileTooLarge;
            }   
        }
    }

    // The user passes in the name of the binary file to use as a ROM when starting the emulator.
    if(current_index < argc) {
        std::string subcommand = argv[current_index];
    }

    return Result::Success;
}

__attribute__((weak)) void Fabric::init(uint64_t rom_base, uint64_t rom_size) {
    // Default implementation of the Fabric::init function.
    // This can be overridden by the user to provide custom initialization logic for the fabric.
}

__attribute__((weak)) uint64_t Fabric::execute(MemoryTransaction transaction) {
    // Default implementation of the Fabric::execute function.
    // This can be overridden by the user to provide custom execution logic for the fabric.
    return 0;
}

__attribute__((weak)) void Cpu::init(uint64_t reset_vector) {
    // Default implementation of the Cpu::init function.
    // This can be overridden by the user to provide custom initialization logic for the CPU.
}

__attribute__((weak)) void Cpu::power_on() {
    // Default implementation of the Cpu::power_on function.
    // This can be overridden by the user to provide custom power-on logic for the CPU.
}