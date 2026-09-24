#ifndef VRITA_DMG_PPU_INCLUDES_INCLUDES
#define VRITA_DMG_PPU_INCLUDES_INCLUDES

#include <cstdint>

struct TilePixel {
    uint8_t colorId;
    uint8_t paletteNum;
    bool bgPriority;
};

struct OAMSprite {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t flags;
    int oamIndex;
};

#endif