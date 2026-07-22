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
    float zoomPerPixel = 2.0f;
    float previewSize = 40.0f;
    bool previewSelected = false;
    bool autoRefresh = true;
    bool showGrid = true;
    int tileSize = 0;

    TileItem hoveredTileItem;
    TileItem hoveredTileItemBottom;
    TileItem selectedTileItem;
    TileItem selectedTileItemBottom;
    bool hoveredHasBottom = false;
    bool selectedHasBottom = false;

    void initializeData(uint8_t emulatorType);
    void decodeTile(const uint8_t* tileData, TileItem& tile);
    void renderTiles();
    void renderTilePreview();
    void drawTileUnit(ImDrawList* draw_list, const TileItem& top, const TileItem& bottom, bool hasBottom, ImVec2 pos, float pixelSize);
    void drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder = true);
    int pickHoveredSlot(ImVec2 start, float tileStepX, float tileStepY, int tilesPerRow, int count);
};

#endif