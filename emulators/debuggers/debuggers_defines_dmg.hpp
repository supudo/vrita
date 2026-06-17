#ifndef VRITA_DEBUGGERS_DEFINES_DMG_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_DMG_INCLUDES

#include <array>
#include <map>
#include <vector>
#include <imgui.h>

#include "debuggers_defines.hpp"

const uint32_t DMG_Width = 160;
const uint32_t DMG_Height = 144;
const uint16_t DMG_TilesCount = 384;
const uint16_t DMG_TileAddressStart = 0x8000;
const uint16_t DMG_TileAddressEnd = 0x8FFF;
const uint16_t DMG_TileAddressOBJ = 0xFE00;
const uint16_t DMG_TileAddressEnded = 0x97FF;

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

#include "debuggers_defines_dmg.inl"

#endif