/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_PPU_INCLUDES
#define VRITA_DMG_PPU_INCLUDES

#include "mmu.hpp"
#include "cpu_registers.hpp"

class Logger;
class DMG_MMU;
class DMG_INTERRUPT;

class DMG_PPU {
public:
    DMG_PPU(Logger& logger, DMG_MMU& mmu, DMG_INTERRUPT& interrupts) : logger(logger), mmu(mmu), interrupts(interrupts){}

    void step(bool ROMFileLoaded, uint32_t cycles);
    void clearResources();

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& interrupts;

    uint32_t dotCycles = 0;
};

#endif