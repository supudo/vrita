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

#include "debuggers_defines_dmg.inl"

#endif