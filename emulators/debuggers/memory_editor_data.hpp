#ifndef VRITA_MEMORYEDITOR_DATA_INCLUDES
#define VRITA_MEMORYEDITOR_DATA_INCLUDES

#include <cstdint>
#include <map>
#include <vector>

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