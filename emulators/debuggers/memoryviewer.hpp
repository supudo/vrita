/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_MEMORYVIEWER_INCLUDES
#define VRITA_MEMORYVIEWER_INCLUDES

#include <stdint.h>
#include <imgui.h>

#include "utilities/logger.hpp"

class Settings;

class MemoryViewer {
public:
    MemoryViewer(Logger& logger) : logger(logger) {}

    bool init(Settings settings);
    void release(Settings& settings);

    void render(bool* windowOpened);
    void setMemory(const uint8_t* data, uint32_t size);

private:
    Logger& logger;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    const uint8_t* memoryData = nullptr;
    uint32_t memorySize = 0;
    int scrollToAddrress = -1;
};

#endif