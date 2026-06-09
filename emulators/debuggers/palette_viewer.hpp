/*

GameBoy Advance (AGB)

*/

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
    PaletteViewer(Logger& logger) : logger(logger) {}

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