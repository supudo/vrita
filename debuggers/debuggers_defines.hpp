#ifndef VRITA_DEBUGGERS_DEFINES_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_INCLUDES

#include <cstdio>
#include <map>
#include <vector>
#include <imgui.h>

struct SpriteItem {
    ImGuiID SpriteID;
    uint8_t X;
    uint8_t Y;
    uint8_t Tile;
    uint8_t Flags;
    uint8_t Index;
    uint16_t OAMAddress;
    SpriteItem() : SpriteID(0) {}
    SpriteItem(ImGuiID spriteID, uint8_t x, uint8_t y, uint8_t tile, uint8_t flags, uint8_t index, uint16_t oamAddress) {
        SpriteID = spriteID;
        X = x;
        Y = y;
        Tile = tile;
        Flags = flags;
        Index = index;
        OAMAddress = oamAddress;
    }
};

struct TileItem {
    uint8_t Pixels[8][8];
    ImGuiID TileItemID;
    uint16_t TileAddress;
    TileItem() : TileItemID(0) {
        TileAddress = 0x0000;
        for (auto& row : Pixels)
            for (auto& c : row)
                c = 0;
    }
    TileItem(ImGuiID id, uint16_t tileAddress) {
        TileItemID = id;
        TileAddress = tileAddress;
        for (auto& row : Pixels)
            for (auto& c : row)
                c = 0;
    }
};

struct TilemapItem {
    ImGuiID TilemapItemID;
    uint16_t TileAddress;
    const TileItem* Tile;
    TilemapItem() : TilemapItemID(0), TileAddress(0x0000), Tile(nullptr) {}
    TilemapItem(ImGuiID tilemapItemID, uint16_t tileAddress, const TileItem* tile) {
        TilemapItemID = tilemapItemID;
        TileAddress = tileAddress;
        Tile = tile;
    }
};

struct PaletteColor {
    float r, g, b;
};

struct AddressRange {
    uint32_t start;
    uint32_t end;
};

struct MemoryRegion {
    const char* region;
    const char* notes;
    AddressRange range;
    uint32_t color;
    bool editable;
};

using MemoryCategory = std::vector<MemoryRegion>;
using MemoryUnit = std::map<const char*, MemoryCategory>;
using MemoryTree = std::map<const char*, MemoryUnit>;

enum DebuggerNodeValueSource : uint8_t {
    NVS_None = 0,
    NVS_Memory,
    NVS_RegBC,
    NVS_RegDE,
    NVS_RegHL,
    NVS_RegAF,
    NVS_RegSP,
    NVS_RegPC,
};

enum DebuggerNodeDisplayType : uint8_t {
    NDT_None = 0,
    NDT_Custom = 1,
    NDT_Hex8 = 8,
    NDT_Hex16 = 16,
};

struct DebuggerRegisterTreeNode {
    std::function<void(DebuggerRegisterTreeNode*)> renderCustom;
    const char* Name;
    uint32_t Address;
    int ChildIdx;
    int ChildCount;
    DebuggerNodeDisplayType Type;
    DebuggerNodeValueSource Source;
    uint16_t Value;
    bool isOpenedByDefault;
    bool isRoot;
};

#endif