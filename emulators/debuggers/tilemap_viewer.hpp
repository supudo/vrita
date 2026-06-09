#ifndef VRITA_TILEMAPVIEWER_INCLUDES
#define VRITA_TILEMAPVIEWER_INCLUDES

#include <array>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"

class Settings;

class TilemapViewer {
public:
    TilemapViewer(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void release();
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
};

#endif