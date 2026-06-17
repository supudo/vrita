#ifndef VRITA_DEBUGGERS_DEFINES_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_INCLUDES

#include <map>
#include <vector>
#include <imgui.h>

struct TileColor {
    float r, g, b, a;
};

struct TileItem {
    ImGuiID TileItemID;
    TileColor pixels[8][8];

    TileItem(ImGuiID id) { TileItemID = id; }
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

#endif