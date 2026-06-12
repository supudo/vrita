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
    void setFramebuffer(uint32_t* fb);

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& interrupts;

    uint32_t dotCycles = 0;
    uint32_t* framebuffer = nullptr;
    uint8_t windowLine = 0;

    void renderScanline(uint8_t ly);
    void renderBackground(uint8_t ly);
    void renderWindow(uint8_t ly);
    void renderSprites(uint8_t ly);
    uint32_t applyPalette(uint8_t paletteReg, uint8_t colorId) const;

    static constexpr uint32_t DMG_COLORS[4] = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F };
};

#endif