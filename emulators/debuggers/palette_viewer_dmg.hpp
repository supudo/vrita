#ifndef VRITA_PALETTEVIEWER_DMG_INCLUDES
#define VRITA_PALETTEVIEWER_DMG_INCLUDES

#include <array>

struct PaletteColor {
    float r, g, b;
};

inline std::array<PaletteColor, 4> DMG_Palette_DMG = { {
    { 0.88f, 0.97f, 0.82f },
    { 0.55f, 0.75f, 0.42f },
    { 0.22f, 0.42f, 0.18f },
    { 0.06f, 0.15f, 0.06f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_CGB = { {
    { 0.61f, 0.74f, 0.06f },
    { 0.55f, 0.67f, 0.06f },
    { 0.19f, 0.38f, 0.19f },
    { 0.06f, 0.22f, 0.06f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_MGB = { {
    { 0.77f, 0.81f, 0.63f },
    { 0.55f, 0.58f, 0.43f },
    { 0.30f, 0.33f, 0.24f },
    { 0.12f, 0.12f, 0.12f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_MGL = { {
    { 0.11f, 0.87f, 0.81f },
    { 0.10f, 0.78f, 0.70f },
    { 0.09f, 0.65f, 0.59f },
    { 0.04f, 0.48f, 0.43f }
} };

#endif