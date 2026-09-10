#ifndef VRITA_DMG_PPU_INCLUDES_INCLUDES
#define VRITA_DMG_PPU_INCLUDES_INCLUDES

struct TilePixel {
    uint8_t colorId;
    uint8_t paletteNum;
    bool bgPriority;
};

#endif