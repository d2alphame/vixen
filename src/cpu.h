// Header file for the cpu

#include <stdint.h>

// The reset vector. CPU jumps immediately to this address when powered on or reset.
#define RESET_VECTOR 0xFFFFFFFFFFFF0000;

/* The registers are 64 bits wide. They are
1. A : The A regiter is the accumulator register
2. B : The B register is the base register
3. C : The C register is the counter register
4. D : The D register is the data register
5. E : The E register is the extra register
6. F : The F register is the flags register
7. I : The I register is the instruction register (not accessible to the programmer)
7. S : The S register is the source register
8. T : The T register is the target register
*/

uint64_t A, B, C, D, E, F, I, S, T;  // Define a unsigned 64 bit integers