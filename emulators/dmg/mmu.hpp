/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_MMU_INCLUDES
#define VRITA_DMG_MMU_INCLUDES

#include <stdint.h>

class DMG_MMU {
public:
    void initialize();
    void clearMemory();

    static const uint32_t MEMORY_SIZE = 0x10000; // 64 KB
    uint8_t memory[MEMORY_SIZE] = {};

private:
    inline uint8_t read8(uint16_t address) const { return memory[address]; }
    inline void write8(uint16_t address, uint8_t value) { memory[address] = value; }
    inline uint16_t read16(uint16_t address) const { return memory[address] | (memory[address + 1] << 8); }
    inline void write16(uint16_t address, uint16_t value) { memory[address] = value & 0xFF; memory[address + 1] = value >> 8; }
};

#endif