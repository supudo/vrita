#ifndef VRITA_TILEMAPVIEWER_INCLUDES
#define VRITA_TILEMAPVIEWER_INCLUDES

#include <array>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "palette_viewer.hpp"

class Settings;

class TilemapViewer {
public:
    TilemapViewer(Logger& logger, Settings& settings, PaletteViewer& paletteViewer) : logger(logger), settings(settings), paletteViewer(paletteViewer) {}

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

    float zoomPerPixel = 2.0f;
    bool autoRefresh = true;
    bool showGrid = true;

    int tileMapAddress = 0;
    int tileDataAddress = 0;

    float lastInfoHeight = 0.0f;

    ImVector<TileItem> tiles;
    ImVector<TilemapItem> mapTiles1;
    ImVector<TilemapItem> mapTiles2;
    TilemapItem hoveredTilemapItem;
    TilemapItem selectedTilemapItem;

    void initializeData(uint8_t emulatorType);
    void decodeTile(const uint8_t* tileData, TileItem& tile);
    void renderTileMap(float height, ImVector<TilemapItem> mapTiles);
    void renderTileMapInfo();
    void textRightAligned(const char* text);
    void drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder = true);
};

#endif