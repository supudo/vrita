/*

GameBoy

*/

#ifndef VRITA_DMG_PPU_INCLUDES
#define VRITA_DMG_PPU_INCLUDES

#include "emulators/dmg/mmu.hpp"
#include "emulators/dmg/cpu_registers.hpp"
#include "emulators/dmg/palette_presets.hpp"

#include "emulators/dmg/ppu_includes.hpp"

class Logger;
class DMG_MMU;
class DMG_INTERRUPT;

class DMG_PPU {
public:
    DMG_PPU(Logger& logger, DMG_MMU& mmu, DMG_INTERRUPT& interrupts) : logger(logger), mmu(mmu), interrupts(interrupts){}

    void step(uint32_t cycles);
    void setPalette(int palleteId);
    void clearResources();
    void setFramebuffer(uint32_t* fb);

    void setCGBMode(bool state) { cgbMode = state; }

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& interrupts;

    uint32_t dotCycles = 0;
    uint32_t* framebuffer = nullptr;
    uint8_t windowLine = 0;

    // CGB related
    bool cgbMode = false;
    uint8_t lastPPUMode = 0xFF;
    uint8_t scanlineBGColorId[160] {};
    bool scanlineBGPriority[160] {};

    void renderScanline(uint8_t ly);
    void renderBackground(uint8_t ly);
    void renderWindow(uint8_t ly);
    void renderSprites(uint8_t ly);
    uint32_t applyPalette(uint8_t paletteReg, uint8_t colorId) const;
    uint8_t tileColorId(uint16_t tilemapBase, bool signedAddr, uint8_t tileCol, uint8_t tileRow, uint8_t pixelRow, uint8_t pixelCol) const;

    int paletteChoicesSelected = 0;

    uint32_t getBackground() const;

    uint16_t addressLCDC = 0xFF40;
    uint16_t addressSTAT = 0xFF41;
    uint16_t addressLY = 0xFF44;
    uint16_t addressLYC = 0xFF45;
    uint16_t addressWY = 0xFF4A;
    uint16_t addressWX = 0xFF4B;
    
    uint16_t addressPaletteBGP = 0xFF47;
    uint16_t addressPaletteOBP0 = 0xFF48;
    uint16_t addressPaletteOBP1 = 0xFF49;

    uint16_t addressTilesOBJ = 0xFE00;
    uint16_t addressTiles0 = 0x9800;
    uint16_t addressTiles1 = 0x9C00;

    uint16_t addressVRAMStart = 0x8000;

    TilePixel tileColorIdCGB(uint16_t tilemapBase, bool signedAddr, uint8_t tileCol, uint8_t tileRow, uint8_t pixelRow, uint8_t pixelCol) const;
    uint32_t applyCGBPalette(const std::array<uint8_t, 64>& paletteRAM, uint8_t paletteNum, uint8_t colorId) const;
};

#endif