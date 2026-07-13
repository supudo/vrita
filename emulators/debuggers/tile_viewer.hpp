#ifndef VRITA_TILEVIEWER_INCLUDES
#define VRITA_TILEVIEWER_INCLUDES

#include <array>
#include <stdint.h>
#include <cstdint>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "debuggers_defines.hpp"
#include "palette_viewer.hpp"

class Settings;

class TileViewer {
public:
    TileViewer(Logger& logger, Settings& settings, PaletteViewer& paletteViewer) : logger(logger), settings(settings), paletteViewer(paletteViewer) {}

    bool init();
    void setMemory(const char* emulatorType, uint8_t* data);
    void release();
    void render(bool* windowOpened);

private:
    Logger& logger;
    Settings& settings;
    PaletteViewer& paletteViewer;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint8_t* memoryData = nullptr;
    uint8_t emulatorType = -1;

    ImVector<TileItem> tiles;
    ImGuiID nextTileID;
    float zoomPerPixel = 2.0f;
    float previewSize = 40.0f;
    int tileHovered = 0;
    int tileSelected = 0;
    bool previewSelected = false;

    void initializeData(uint8_t emulatorType);
    void decodeTile(const uint8_t* tileData, TileItem& tile);
    void renderTiles();
    void renderTilePreview();
    void drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize);
};

#endif