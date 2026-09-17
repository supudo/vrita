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
    void setCallbacks(std::function<uint8_t(uint16_t, uint8_t)> const& vramReadBank, std::function<const uint8_t* (bool isOBJ)> const& getPaletteRAM);
    void setMemory(const char* emuType, uint8_t* data, bool isCGB);
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

    void initializeData(uint8_t emuType);
    void decodeTile(const uint8_t* tileData, TileItem& tile) const;
    void renderTiles();
    void renderTiles1Tab(const char* tabLabel, const char* childId, int bankOffset, float tileStepX, float tileStepY, int tilesPerRow, bool stacked);
    void renderTiles2Tab(const char* tabLabel, const char* childId, int bankOffset, float tileStepX, float tileStepY, int tilesPerRow, bool stacked);
    void renderOBJTab(const char* tabLabel, const char* childId, int bankFilter, float tileStepX, float tileStepY, int tilesPerRow, bool stacked);
    void renderTilePreview();
    void drawTileUnit(ImDrawList* draw_list, const TileItem& top, const TileItem& bottom, bool hasBottom, ImVec2 pos, float pixelSize) const;
    void drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder = true) const;
    int pickHoveredSlot(ImVec2 start, float tileStepX, float tileStepY, int tilesPerRow, int count) const;

    bool isCGBLoaded = false;
    std::function<uint8_t(uint16_t, uint8_t)> funcVramReadBank;
    std::function<const uint8_t* (bool isOBJ)> funcGetPaletteRAM;
    int cgbPaletteIndex = 0;
    bool cgbUseObjPalette = false;
    PaletteColor resolveCGBColor(uint8_t colorId) const;
};

#endif