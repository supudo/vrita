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

    uint16_t regsiterAddressLCDC = 0xFF40;
    uint16_t regsiterAddressSTAT = 0xFF41;
    uint16_t regsiterAddressLY = 0xFF44;
    uint16_t regsiterAddressLYC = 0xFF45;
    uint16_t regsiterAddressWY = 0xFF4A;
    uint16_t regsiterAddressWX = 0xFF4B;
    
    uint16_t registerAddressPaletteBGP = 0xFF47;
    uint16_t registerAddressPaletteOBP0 = 0xFF48;
    uint16_t registerAddressPaletteOBP1 = 0xFF49;

    uint16_t registerAddressTilesOBJ = 0xFE00;
    uint16_t registerAddressTiles0 = 0x9800;
    uint16_t registerAddressTiles1 = 0x9C00;

    uint16_t registerAddressVRAMStart = 0x8000;
};

#endif