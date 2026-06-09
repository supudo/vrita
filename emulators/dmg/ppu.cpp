#include "ppu.hpp"
#include "interrupt.hpp"
#include "utilities/logger.hpp"

void DMG_PPU::clearResources() {
    dotCycles = 0;
    mmu.memory[0xFF44] = 0;
    mmu.memory[0xFF41] = (mmu.memory[0xFF41] & 0xFC) | 2; // start in OAM mode
}

void DMG_PPU::step(bool ROMFileLoaded, uint32_t cycles) {
    if (!ROMFileLoaded) return;
    dotCycles += cycles;
    while (dotCycles >= 456) {
        dotCycles -= 456;
        mmu.memory[0xFF44]++;
        if (mmu.memory[0xFF44] == 144)
            interrupts.setInterruptFlag(INTERRUPT_VBLANK);
        if (mmu.memory[0xFF44] > 153)
            mmu.memory[0xFF44] = 0;
    }
    uint8_t mode;
    if (mmu.memory[0xFF44] >= 144) mode = 1; // VBlank
    else if (dotCycles < 80) mode = 2; // OAM scan
    else if (dotCycles < 252) mode = 3; // Transfer
    else mode = 0; // HBlank
    mmu.memory[0xFF41] = (mmu.memory[0xFF41] & 0xFC) | mode;
}