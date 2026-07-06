#ifndef VRITA_DMG_MMU_INCLUDES
#define VRITA_DMG_MMU_INCLUDES

#include <stdint.h>
#include <vector>

class Logger;
class DMG_APU;
class DMG_CARTRIDGE;
class DMG_CPU;
class DMG_INTERRUPT;
class DMG_PPU;
class DMG_TIMER;

class DMG_MMU {
public:
    void setUnits(Logger& logger, DMG_CARTRIDGE& cartridge, DMG_CPU& cpu, DMG_TIMER& timer, DMG_INTERRUPT& interrupts, DMG_PPU& ppu, DMG_APU& apu);
    void clearMemory();
    void clearResources();
    void resetRegisters();
    void tick(uint32_t cycles);

    uint32_t memorySize;
    std::vector<uint8_t> memory;

    bool isHalted = false;
    bool triggerHaltBug = false;
    uint64_t totalCycles = 0;

    uint8_t read8(uint16_t address) const;
    void write8(uint16_t address, uint8_t value);
    inline uint16_t read16(uint16_t address) const { return read8(address) | (read8(address + 1) << 8); }
    inline void write16(uint16_t address, uint16_t value) { return write8(address, value & 0xFF); write8(address + 1, value >> 8); }
    inline void writeStack(uint16_t* sp, uint16_t value) { (*sp)--; write8(*sp, (uint8_t)((value & 0xFF00) >> 8)); (*sp)--; write8(*sp, (uint8_t)(value & 0x00FF)); }
    inline uint16_t readStack(uint16_t* sp) { uint16_t value = read16(*sp); *sp += 2; return value; }

private:
    Logger* logger = nullptr;
    DMG_CARTRIDGE* managerCartridge = nullptr;
    DMG_CPU* managerCPU = nullptr;
    DMG_TIMER* managerTimer = nullptr;
    DMG_INTERRUPT* managerInterrupts = nullptr;
    DMG_PPU* managerPPU = nullptr;
    DMG_APU* managerAPU = nullptr;
};

#endif