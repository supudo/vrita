#ifndef VRITA_MEMORYEDITOR_INCLUDES
#define VRITA_MEMORYEDITOR_INCLUDES

#include <array>
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <map>
#include <stdint.h>
#include <vector>

#include "utilities/logger.hpp"
#include "memory_editor_cgb.hpp"
#include "memory_editor_dmg.hpp"
#include "memory_editor_agb.hpp"

class Settings;

class MemoryEditor {
public:
    MemoryEditor(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void release();
    void render(bool* windowOpened);
    
    void setMemory(const char* emulatorType, uint8_t* data, uint32_t size);
    void setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8);
    void setRegsiterCallback(std::function<uint16_t(const char*)> getRegsiter);

private:
    Logger& logger;
    Settings& settings;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    int viewPerspective = 0;
    float panelsWidthLeft = 250.0f;
    float panelsSplitterWidth = 6.0f;
    const MemoryRegion* selectedMemoryRegion = nullptr;

    uint8_t* memoryData = nullptr;
    uint32_t memorySize = 0;
    int scrollToAddress = -1;
    int activeAddress = -1;

    std::vector<uint8_t> shadowMemory;
    std::vector<float> changeTimer;

    const MemoryRegion* memoryRegions = nullptr;
    size_t memoryRegionCount = 0;
    uint8_t emulatorType = 0;
    static constexpr const char* previewDataTypes[] = { "Uint8", "Uint16", "Uint32" };
    const char* selectedDataType = previewDataTypes[0];

    int followRegister = 0;
    int followAddress = -1;
    std::function<uint16_t(const char*)> registerReadFunction;

    void renderMemoryRegion(MemoryRegion region);
    const MemoryRegion* getRegion(uint32_t addr) const;

    std::function<uint8_t(uint32_t)> memoryRead;
    std::function<void(uint32_t, uint8_t)> memoryWrite;

    void getPreviewData(int address, char* out_buf, char format);

    void renderViewPerspectiveDefault();
    void renderViewPerspectiveAdvanced(const MemoryTree& tree);
    void renderViewPerspectiveTree(const MemoryTree& tree);
    void renderViewPerspectiveTreeRegion(const MemoryRegion& region);
};

#endif