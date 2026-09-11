#include "ppu.hpp"

#include <algorithm>

#include "emulators/dmg/interrupt.hpp"
#include "utilities/logger.hpp"

void DMG_PPU::setFramebuffer(uint32_t* fb) {
    framebuffer = fb;
}

void DMG_PPU::clearResources() {
    dotCycles = 0;
    windowLine = 0;
    lastPPUMode = 0xFF;
    mmu.memory[addressLY] = 0; // LY
    mmu.memory[addressSTAT] = (mmu.memory[addressSTAT] & 0xFC) | 2; // STAT
}

void DMG_PPU::step(uint32_t cycles) {
#ifdef TRACY_ENABLE
    ZoneScopedN("PPU::step");
#endif
    if (!(mmu.memory[addressLCDC] & 0x80)) {
        mmu.memory[addressLY] = 0;
        mmu.memory[addressSTAT] = mmu.memory[addressSTAT] & 0xF8;
        dotCycles = 0;
        windowLine = 0;
        lastPPUMode = 0xFF;
        return;
    }
    dotCycles += cycles;
    while (dotCycles >= 456) {
        dotCycles -= 456;
        uint8_t ly = mmu.memory[addressLY];
        if (ly < 144 && framebuffer)
            renderScanline(ly);
        ly++;
        mmu.memory[addressLY] = ly;
        if (ly == 144)
            interrupts.setInterruptFlag(INTERRUPT_VBLANK);
        if (ly > 153) {
            mmu.memory[addressLY] = 0;
            windowLine = 0;
        }
        if (mmu.memory[addressLY] == mmu.memory[addressLYC] && (mmu.memory[addressSTAT] & 0x40))
            interrupts.setInterruptFlag(INTERRUPT_LCD);
    }
    uint8_t mode;
    if (mmu.memory[addressLY] >= 144)
        mode = 1;
    else if (dotCycles < 80)
        mode = 2;
    else if (dotCycles < 252)
        mode = 3;
    else
        mode = 0;

    if (cgbMode && mode == 0 && lastPPUMode != 0)
        mmu.onHBlank();
    lastPPUMode = mode;

    uint8_t lyc = mmu.memory[addressLYC];
    uint8_t stat = (mmu.memory[addressSTAT] & 0xF8) | mode;
    if (mmu.memory[addressLY] == lyc) stat |= 0x04;
    mmu.memory[addressSTAT] = stat;
}

void DMG_PPU::setPalette(int palleteId) {
    paletteChoicesSelected = palleteId;
}

uint32_t DMG_PPU::applyPalette(uint8_t paletteReg, uint8_t colorId) const {
    uint8_t shade = (paletteReg >> (colorId * 2)) & 0x03;
    uint32_t argb;
    switch (paletteChoicesSelected) {
        case 1:
            argb = DMG_PALETTE_DMG[shade];
            break;
        case 2:
            argb = DMG_PALETTE_CGB[shade];
            break;
        case 3:
            argb = DMG_PALETTE_MGB[shade];
            break;
        case 4:
            argb = DMG_PALETTE_MGL[shade];
            break;
        default:
            argb = DMG_PALETTE_DEFAULT[shade];
            break;
    }
    return DMG_PackForFramebuffer(argb);
}

uint32_t DMG_PPU::getBackground() const {
    return applyPalette(0x00, 0);
}

uint8_t DMG_PPU::tileColorId(uint16_t tilemapBase, bool signedAddr, uint8_t tileCol, uint8_t tileRow, uint8_t pixelRow, uint8_t pixelCol) const {
    uint8_t tileIndex = mmu.memory[tilemapBase + tileRow * 32 + tileCol];

    uint16_t tileAddr;
    if (signedAddr)
        tileAddr = (uint16_t)(0x9000 + (int8_t)tileIndex * 16);
    else
        tileAddr = addressVRAMStart + tileIndex * 16;

    uint8_t low = mmu.memory[tileAddr + pixelRow * 2];
    uint8_t high = mmu.memory[tileAddr + pixelRow * 2 + 1];
    uint8_t bit = 7 - pixelCol;
    return (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
}

void DMG_PPU::renderScanline(uint8_t ly) {
    renderBackground(ly);
    renderWindow(ly);
    renderSprites(ly);
}

void DMG_PPU::renderBackground(uint8_t ly) {
    uint8_t lcdc = mmu.memory[addressLCDC];
    if (!cgbMode && !(lcdc & 0x01)) {
        for (int x = 0; x < 160; x++) {
            framebuffer[ly * 160 + x] = getBackground();
            scanlineBGColorId[x] = 0;
            scanlineBGPriority[x] = false;
        }
        return;
    }
    uint8_t scy = mmu.memory[0xFF42];
    uint8_t scx = mmu.memory[0xFF43];
    uint8_t bgp = mmu.memory[addressPaletteBGP];
    uint16_t tilemapBase = (lcdc & 0x08) ? addressTiles1 : addressTiles0;
    bool signedAddr = !(lcdc & 0x10);

    for (int x = 0; x < 160; x++) {
        uint8_t mapX = (scx + x) & 0xFF;
        uint8_t mapY = (scy + ly) & 0xFF;
        if (cgbMode) {
            TilePixel pixel = tileColorIdCGB(tilemapBase, signedAddr, mapX / 8, mapY / 8, mapY % 8, mapX % 8);
            framebuffer[ly * 160 + x] = applyCGBPalette(mmu.getBGPaletteRAM(), pixel.paletteNum, pixel.colorId);
            scanlineBGColorId[x] = pixel.colorId;
            scanlineBGPriority[x] = pixel.bgPriority;
        }
        else {
            uint8_t colorId = tileColorId(tilemapBase, signedAddr, mapX / 8, mapY / 8, mapY % 8, mapX % 8);
            framebuffer[ly * 160 + x] = applyPalette(bgp, colorId);
            scanlineBGColorId[x] = colorId;
            scanlineBGPriority[x] = false;
        }
    }
}

void DMG_PPU::renderWindow(uint8_t ly) {
    uint8_t lcdc = mmu.memory[addressLCDC];
    if (!(lcdc & 0x20))
        return;
    
    uint8_t wy = mmu.memory[addressWY];
    if (ly < wy)
        return;
    
    int wx = (int)mmu.memory[addressWX] - 7;
    if (wx >= 160)
        return;

    uint8_t bgp = mmu.memory[addressPaletteBGP];
    uint16_t tilemapBase = (lcdc & 0x40) ? addressTiles1 : addressTiles0;
    bool signedAddr = !(lcdc & 0x10);
    bool drewAnyPixel = false;

    for (int x = (wx < 0 ? 0 : wx); x < 160; x++) {
        int winX = x - wx;
        int winY = windowLine;
        if (cgbMode) {
            TilePixel pixel = tileColorIdCGB(tilemapBase, signedAddr, winX / 8, winY / 8, winY % 8, winX % 8);
            framebuffer[ly * 160 + x] = applyCGBPalette(mmu.getBGPaletteRAM(), pixel.paletteNum, pixel.colorId);
            scanlineBGColorId[x] = pixel.colorId;
            scanlineBGPriority[x] = pixel.bgPriority;
        }
        else {
            uint8_t colorId = tileColorId(tilemapBase, signedAddr, winX / 8, winY / 8, winY % 8, winX % 8);
            framebuffer[ly * 160 + x] = applyPalette(bgp, colorId);
            scanlineBGColorId[x] = colorId;
            scanlineBGPriority[x] = false;
        }
        drewAnyPixel = true;
    }
    if (drewAnyPixel)
        windowLine++;
}

void DMG_PPU::renderSprites(uint8_t ly) {
    uint8_t lcdc = mmu.memory[addressLCDC];
    if (!(lcdc & 0x02)) return;
    uint8_t sprH = (lcdc & 0x04) ? 16 : 8;

    struct Sprite { uint8_t y, x, tile, flags; int oamIndex; };
    Sprite visible[10];
    int count = 0;

    for (int i = 0; i < 40 && count < 10; i++) {
        uint8_t sy = mmu.memory[addressTilesOBJ + i * 4];
        uint8_t sx = mmu.memory[addressTilesOBJ + i * 4 + 1];
        if (ly + 16 >= sy && ly + 16 < sy + sprH)
            visible[count++] = { sy, sx, mmu.memory[addressTilesOBJ + i * 4 + 2], mmu.memory[addressTilesOBJ + i * 4 + 3], i };
    }

    if (!cgbMode)
        std::stable_sort(visible, visible + count, [] (const Sprite& a, const Sprite& b) { return a.x < b.x; }); // X ascending for DMG

    bool masterPriority = (lcdc & 0x01) != 0;

    for (int i = count - 1; i >= 0; i--) {
        Sprite& s = visible[i];
        int screenX = (int)s.x - 8;
        int screenY = (int)s.y - 16;
        int pixelRow = (int)ly - screenY;
        
        bool yFlip = s.flags & 0x40;
        bool xFlip = s.flags & 0x20;
        bool oamBgPriority = s.flags & 0x80;

        uint8_t cgbPaletteNum = s.flags & 0x07;
        uint8_t cgbBank = (s.flags & 0x08) ? 1 : 0;
        
        uint8_t dmgPalette = (s.flags & 0x10) ? mmu.memory[addressPaletteOBP1] : mmu.memory[addressPaletteOBP0];

        if (yFlip) pixelRow = sprH - 1 - pixelRow;

        uint8_t tileIndex = s.tile;
        if (sprH == 16) tileIndex &= 0xFE;

        uint16_t tileAddr = addressVRAMStart + tileIndex * 16;
        int row = pixelRow;
        if (row >= 8) { tileAddr += 16; row -= 8; }

        uint8_t low, high;
        if (cgbMode) {
            low = mmu.vramReadBank(tileAddr + row * 2, cgbBank);
            high = mmu.vramReadBank(tileAddr + row * 2 + 1, cgbBank);
        }
        else {
            low = mmu.memory[tileAddr + row * 2];
            high = mmu.memory[tileAddr + row * 2 + 1];
        }

        for (int col = 0; col < 8; col++) {
            int px = screenX + col;
            if (px < 0 || px >= 160)
                continue;
            uint8_t bit = xFlip ? col : (7 - col);
            uint8_t colorId = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
            if (colorId == 0)
                continue;
            bool bgWins = cgbMode ? (masterPriority && scanlineBGColorId[px] != 0 && (oamBgPriority || scanlineBGPriority[px])) : (oamBgPriority && scanlineBGColorId[px] != 0);
            if (bgWins)
                continue;
            if (cgbMode)
                framebuffer[ly * 160 + px] = applyCGBPalette(mmu.getOBJPaletteRAM(), cgbPaletteNum, colorId);
            else
                framebuffer[ly * 160 + px] = applyPalette(dmgPalette, colorId);
        }
    }
}

TilePixel DMG_PPU::tileColorIdCGB(uint16_t tilemapBase, bool signedAddr, uint8_t tileCol, uint8_t tileRow, uint8_t pixelRow, uint8_t pixelCol) const {
    uint16_t mapAddr = tilemapBase + tileRow * 32 + tileCol;
    uint8_t tileIndex = mmu.vramReadBank(mapAddr, 0);
    uint8_t attr = mmu.vramReadBank(mapAddr, 1);

    uint8_t paletteNum = attr & 0x07;
    uint8_t bank = (attr & 0x08) ? 1 : 0;
    if (attr & 0x40)
        pixelRow = 7 - pixelRow; // y-flip
    if (attr & 0x20)
        pixelCol = 7 - pixelCol; // x-flip

    uint16_t tileAddr = signedAddr ? (uint16_t)(0x9000 + (int8_t)tileIndex * 16) : addressVRAMStart + tileIndex * 16;
    uint8_t low = mmu.vramReadBank(tileAddr + pixelRow * 2, bank);
    uint8_t high = mmu.vramReadBank(tileAddr + pixelRow * 2 + 1, bank);
    uint8_t bit = 7 - pixelCol;
    return { (uint8_t)((((high >> bit) & 1) << 1) | ((low >> bit) & 1)), paletteNum, (attr & 0x80) != 0 };
}

uint32_t DMG_PPU::applyCGBPalette(const std::array<uint8_t, 64>& paletteRAM, uint8_t paletteNum, uint8_t colorId) const {
    uint16_t idx = paletteNum * 8 + colorId * 2;
    uint16_t rgb555 = paletteRAM[idx] | (paletteRAM[idx + 1] << 8);
    uint8_t r5 = rgb555 & 0x1F;
    uint8_t g5 = (rgb555 >> 5) & 0x1F;
    uint8_t b5 = (rgb555 >> 10) & 0x1F;
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g5 << 3) | (g5 >> 2);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    return DMG_PackForFramebuffer(0xFF000000 | (r8 << 16) | (g8 << 8) | b8);
}