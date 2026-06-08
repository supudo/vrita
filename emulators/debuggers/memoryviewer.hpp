/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_MEMORYVIEWER_INCLUDES
#define VRITA_MEMORYVIEWER_INCLUDES

#include <array>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"

class Settings;

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

class MemoryViewer {
public:
    MemoryViewer(Logger& logger) : logger(logger) {}

    bool init(Settings settings);
    void release(Settings& settings);
    void render(bool* windowOpened);
    
    void setMemory(const char* emulatorType, uint8_t* data, uint32_t size);
    void setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8);

private:
    Logger& logger;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint8_t* memoryData = nullptr;
    uint32_t memorySize = 0;
    int scrollToAddrress = -1;

    std::array<MemoryRegion, 9> MemoryMap_DMG;
    std::array<MemoryRegion, 11> MemoryMap_AGB;
    const MemoryRegion* memoryRegions = nullptr;
    size_t memoryRegionCount = 0;
    uint8_t emulatorType = 0;

    void renderMemoryRegion(MemoryRegion region);
    const MemoryRegion* getRegion(uint32_t addr) const;

    std::function<uint8_t(uint32_t)> memoryRead;
    std::function<void(uint32_t, uint8_t)> memoryWrite;
};

#endif