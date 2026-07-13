#ifndef VRITA_DEBUGGERS_DEFINES_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_INCLUDES

#include <cstdio>
#include <functional>
#include <map>
#include <vector>
#include <imgui.h>

struct TileColor {
    float r, g, b, a;
};

struct TileItem {
    ImGuiID TileItemID;
    TileColor pixels[8][8];
    uint8_t TileNumberHex;
    uint8_t TileNumberInt;
    char TileAddress[8];
    TileItem() : TileItemID(0), TileNumberHex(0), TileNumberInt(0) {
        TileAddress[0] = '\0';
        for (auto& row : pixels)
            for (auto& c : row)
                c = { 0.0f, 0.0f, 0.0f, 1.0f };
    }
    TileItem(ImGuiID id, uint8_t tileNumberHex, uint8_t tileNumberInt, const char* tileAddress) {
        TileItemID = id;
        TileNumberHex = tileNumberHex;
        TileNumberInt = tileNumberInt;
        snprintf(TileAddress, sizeof(TileAddress), "%s", tileAddress);
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