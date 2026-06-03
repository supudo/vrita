#ifndef VRITA_DMG_CPU_REGISTERS_INCLUDES
#define VRITA_DMG_CPU_REGISTERS_INCLUDES

#include <stdint.h>

struct DMGCpuRegisters {
    uint8_t A, F;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;

    uint16_t SP = 0;
    uint16_t PC = 0;

    union { struct { uint8_t F; uint8_t A; };  uint16_t AF; };
    union { struct { uint8_t C; uint8_t B; }; uint16_t BC; };
    union { struct { uint8_t E; uint8_t D; }; uint16_t DE; };
    union { struct { uint8_t L; uint8_t H; }; uint16_t HL; };
};

enum DMGCpuFlags : uint8_t {
    FLAG_ZERO = 1 << 7,
    FLAG_SUBTRACT = 1 << 6,
    FLAG_HALF_CARRY = 1 << 5,
    FLAG_CARRY = 1 << 4
};

#endif
