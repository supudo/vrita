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
const uint16_t DMG_TilemapCount = 1024;
const uint16_t DMG_TileAddressStart = 0x8000;
const uint16_t DMG_TileAddressEnd = 0x8FFF;
const uint16_t DMG_TileAddressOBJ = 0xFE00;
const uint16_t DMG_TileAddressEnded = 0x97FF;

const uint16_t DMG_TileMap1Start = 0x9800;
const uint16_t DMG_TileMap1End = 0x98FF;
const uint16_t DMG_TileMap2Start = 0x9C00;
const uint16_t DMG_TileMap2End = 0x9FFF;

const uint16_t DMG_TilemapX = 32;
const uint16_t DMG_TilemapY = 32;
const uint16_t DMG_ViewportX = 20;
const uint16_t DMG_ViewportY = 18;

#include "debuggers_defines_dmg.inl"

#endif