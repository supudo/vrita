/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_MEMORYVIEWER_INCLUDES
#define VRITA_MEMORYVIEWER_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <imgui.h>

#include "emulators/emulators.hpp"
#include "utilities/logger.hpp"

class Settings;

class MemoryViewer {
public:
    MemoryViewer(Logger& logger) : logger(logger) {}

    bool init(Settings settings);
    void release(Settings& settings);

    void render(bool* windowOpened);

private:
    Logger& logger;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);
};

#endif