#ifndef VRITA_PALETTEVIEWER_DMG_INCLUDES
#define VRITA_PALETTEVIEWER_DMG_INCLUDES

#include <array>

struct PaletteColor {
    float r, g, b;
};

inline std::array<PaletteColor, 4> DMG_Palette_Green = { {
    {0.88f, 0.97f, 0.82f},
    {0.55f, 0.75f, 0.42f},
    {0.22f, 0.42f, 0.18f},
    {0.06f, 0.15f, 0.06f}
} };

#endif