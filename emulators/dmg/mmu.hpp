#ifndef VRITA_DMG_MMU_INCLUDES
#define VRITA_DMG_MMU_INCLUDES

#include <stdint.h>

class DMG_MMU {
public:
    void clearMemory();
    void clearResources();

    static const uint32_t MEMORY_SIZE = 0x10000; // 64 KB
    uint8_t memory[MEMORY_SIZE] = {};

    inline uint8_t read8(uint16_t address) const { return memory[address]; }
    inline void write8(uint16_t address, uint8_t value) { memory[address] = value; }
    inline uint16_t read16(uint16_t address) const { return memory[address] | (memory[address + 1] << 8); }
    inline void write16(uint16_t address, uint16_t value) { memory[address] = value & 0xFF; memory[address + 1] = value >> 8; }
    inline void write_stack(uint16_t* sp, uint16_t value) { (*sp)--; write8(*sp, (uint8_t)((value & 0xff00) >> 8)); (*sp)--; write8(*sp, (uint8_t)(value & 0x00ff)); }
    inline uint16_t read_stack(uint16_t* sp) { uint16_t value = read16(*sp); *sp += 2; return value; }
};

#endif