#include "ppu.hpp"
#include "interrupt.hpp"
#include "utilities/logger.hpp"

void DMG_PPU::setFramebuffer(uint32_t* fb) {
    framebuffer = fb;
}

void DMG_PPU::clearResources() {
    dotCycles = 0;
    windowLine = 0;
    mmu.memory[addressLY] = 0; // LY
    mmu.memory[addressSTAT] = (mmu.memory[addressSTAT] & 0xFC) | 2; // STAT
}

void DMG_PPU::step(bool ROMFileLoaded, uint32_t cycles) {
    if (!ROMFileLoaded) return;
    if (!(mmu.memory[addressLCDC] & 0x80)) {
        mmu.memory[addressLY] = 0;
        mmu.memory[addressSTAT] = mmu.memory[addressSTAT] & 0xF8;
        dotCycles = 0;
        windowLine = 0;
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
    uint8_t lyc = mmu.memory[addressLYC];
    uint8_t stat = (mmu.memory[addressSTAT] & 0xF8) | mode;
    if (mmu.memory[addressLY] == lyc) stat |= 0x04;
    mmu.memory[addressSTAT] = stat;
}

uint32_t DMG_PPU::applyPalette(uint8_t paletteReg, uint8_t colorId) const {
    return DMG_COLORS[(paletteReg >> (colorId * 2)) & 0x03];
}

void DMG_PPU::renderScanline(uint8_t ly) {
    renderBackground(ly);
    renderWindow(ly);
    renderSprites(ly);
}

void DMG_PPU::renderBackground(uint8_t ly) {
    uint8_t lcdc = mmu.memory[addressLCDC];
    if (!(lcdc & 0x01)) {
        for (int x = 0; x < 160; x++)
            framebuffer[ly * 160 + x] = DMG_COLORS[0];
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
        uint8_t tileCol = mapX / 8;
        uint8_t tileRow = mapY / 8;
        uint8_t tileIndex = mmu.memory[tilemapBase + tileRow * 32 + tileCol];

        uint16_t tileAddr;
        if (signedAddr)
            tileAddr = (uint16_t)(0x9000 + (int8_t)tileIndex * 16);
        else
            tileAddr = addressVRAMStart + tileIndex * 16;

        uint8_t pixelRow = mapY % 8;
        uint8_t low = mmu.memory[tileAddr + pixelRow * 2];
        uint8_t high = mmu.memory[tileAddr + pixelRow * 2 + 1];
        uint8_t bit = 7 - (mapX % 8);
        uint8_t colorId = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);

        framebuffer[ly * 160 + x] = applyPalette(bgp, colorId);
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
        uint8_t tileCol = winX / 8;
        uint8_t tileRow = winY / 8;
        uint8_t tileIndex = mmu.memory[tilemapBase + tileRow * 32 + tileCol];

        uint16_t tileAddr;
        if (signedAddr)
            tileAddr = (uint16_t)(0x9000 + (int8_t)tileIndex * 16);
        else
            tileAddr = addressVRAMStart + tileIndex * 16;

        uint8_t pixelRow = winY % 8;
        uint8_t low = mmu.memory[tileAddr + pixelRow * 2];
        uint8_t high = mmu.memory[tileAddr + pixelRow * 2 + 1];
        uint8_t bit = 7 - (winX % 8);
        uint8_t colorId = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);

        framebuffer[ly * 160 + x] = applyPalette(bgp, colorId);
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

    for (int i = count - 1; i >= 0; i--) {
        Sprite& s = visible[i];
        int screenX = (int)s.x - 8;
        int screenY = (int)s.y - 16;
        int pixelRow = (int)ly - screenY;
        
        // 6-th bit
        // sprite is normal or upside-down for Y
        bool yFlip = s.flags & 0x40;
        
        // 5-th bit
        // sprite is normal or upside-down for X
        bool xFlip = s.flags & 0x20;
        
        // 7-th but
        // if == 0, sprite is in front of background/winow, sprite has priority
        // if == 1, sprite is behind the background/window, background has priority
        bool bgPriority = s.flags & 0x80;

        // 4-th but
        // which palette to use
        uint8_t palette = (s.flags & 0x10) ? mmu.memory[addressPaletteOBP1] : mmu.memory[addressPaletteOBP0];

        if (yFlip) pixelRow = sprH - 1 - pixelRow;

        uint8_t tileIndex = s.tile;
        if (sprH == 16) tileIndex &= 0xFE;

        uint16_t tileAddr = addressVRAMStart + tileIndex * 16;
        int row = pixelRow;
        if (row >= 8) { tileAddr += 16; row -= 8; }

        uint8_t low = mmu.memory[tileAddr + row * 2];
        uint8_t high = mmu.memory[tileAddr + row * 2 + 1];

        for (int col = 0; col < 8; col++) {
            int px = screenX + col;
            if (px < 0 || px >= 160)
                continue;
            uint8_t bit = xFlip ? col : (7 - col);
            uint8_t colorId = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
            if (colorId == 0)
                continue;
            if (bgPriority && framebuffer[ly * 160 + px] != DMG_COLORS[0])
                continue;
            framebuffer[ly * 160 + px] = applyPalette(palette, colorId);
        }
    }
}
