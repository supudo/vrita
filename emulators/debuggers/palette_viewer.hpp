#ifndef VRITA_PALETTEVIEWER_INCLUDES
#define VRITA_PALETTEVIEWER_INCLUDES

#include <array>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"

class Settings;

class PaletteViewer {
public:
    PaletteViewer(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void release();
    void setMemory(const char* emulatorType, uint8_t* data, uint32_t size);
    void render(bool* windowOpened);

private:
    Logger& logger;
    Settings& settings;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint8_t* memoryData = nullptr;
    uint32_t memorySize = 0;
    uint8_t emulatorType = 0;
};

#endif