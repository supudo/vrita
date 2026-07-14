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
    void setPalette(int palleteId);
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
    uint8_t tileColorId(uint16_t tilemapBase, bool signedAddr, uint8_t tileCol, uint8_t tileRow, uint8_t pixelRow, uint8_t pixelCol) const;

    int paletteChoicesSelected = 0;
    static constexpr uint32_t COLORS_DEFAULT[4] = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F };
    static constexpr uint32_t COLORS_DMG[4] = { 0xFFE0F7D1, 0xFF8CBF6B, 0xFF386B2E, 0xFF0F260F };
    static constexpr uint32_t COLORS_CGB[4] = { 0xFF9CBD0F, 0xFF8CAB0F, 0xFF306130, 0xFF0F380F };
    static constexpr uint32_t COLORS_MGB[4] = { 0xFFC4CFA1, 0xFF8C946E, 0xFF4D543D, 0xFF1F1F1F };
    static constexpr uint32_t COLORS_MGL[4] = { 0xFF1CDECF, 0xFF1AC7B3, 0xFF17A696, 0xFF0A7A6E };

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
};

#endif