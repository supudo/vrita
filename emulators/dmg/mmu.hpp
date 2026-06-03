#ifndef VRITA_DMG_MMU_INCLUDES
#define VRITA_DMG_MMU_INCLUDES

#include <stdint.h>

class DMG_APU;
class DMG_CARTRIDGE;
class DMG_CPU;
class DMG_INTERRUPT;
class DMG_PPU;
class DMG_TIMER;

class DMG_MMU {
public:
    void setUnits(DMG_CARTRIDGE* cartridge, DMG_CPU* cpu, DMG_TIMER* timer, DMG_INTERRUPT* interrupts, DMG_PPU* ppu, DMG_APU* apu);
    void clearMemory();
    void clearResources();
    void tick(uint32_t cycles);

    static const uint32_t MEMORY_SIZE = 0x10000; // 64 KB
    uint8_t memory[MEMORY_SIZE] = {};

    bool is_halted = false;
    bool trigger_halt_bug = false;

    inline uint8_t read8(uint16_t address) const { return memory[address]; }
    inline void write8(uint16_t address, uint8_t value) { memory[address] = value; }
    inline uint16_t read16(uint16_t address) const { return memory[address] | (memory[address + 1] << 8); }
    inline void write16(uint16_t address, uint16_t value) { memory[address] = value & 0xFF; memory[address + 1] = value >> 8; }
    inline void write_stack(uint16_t* sp, uint16_t value) { (*sp)--; write8(*sp, (uint8_t)((value & 0xff00) >> 8)); (*sp)--; write8(*sp, (uint8_t)(value & 0x00ff)); }
    inline uint16_t read_stack(uint16_t* sp) { uint16_t value = read16(*sp); *sp += 2; return value; }

private:
    DMG_CARTRIDGE* managerCartridge = nullptr;
    DMG_CPU* managerCPU = nullptr;
    DMG_TIMER* managerTimer = nullptr;
    DMG_INTERRUPT* managerInterrupts = nullptr;
    DMG_PPU* managerPPU = nullptr;
    DMG_APU* managerAPU = nullptr;
};

#endif