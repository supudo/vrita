#include "tile_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include "third_party/imgui/imgui_impl_sdl2.h"

#include "utilities/settings.hpp"
#include "debuggers_defines_dmg.hpp"
#include "debuggers_defines_cgb.hpp"
#include "debuggers_defines_agb.hpp"

bool TileViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Tile Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tile Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tile Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tile Viewer", "height", 300);
    zoomPerPixel = settings.GetFloat("Debuggers - Tile Viewer", "zoom_per_pixel", 2.0f);
    previewSize = settings.GetFloat("Debuggers - Tile Viewer", "preview_size", 40.0f);
    autoRefresh = settings.GetFloat("Debuggers - Tile Viewer", "auto_refresh", true);
    showGrid = settings.GetFloat("Debuggers - Tile Viewer", "show_grid", true);
    tileSize = settings.GetInt("Debuggers - Tile Viewer", "tile_size", 0);
    return true;
}

void TileViewer::release() {
    tiles.clear();
    settings.Set("Debuggers - Tile Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tile Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tile Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tile Viewer", "height", (int)lastWindowSize.y);
    settings.Set("Debuggers - Tile Viewer", "zoom_per_pixel", zoomPerPixel);
    settings.Set("Debuggers - Tile Viewer", "preview_size", previewSize);
    settings.Set("Debuggers - Tile Viewer", "auto_refresh", autoRefresh);
    settings.Set("Debuggers - Tile Viewer", "show_grid", showGrid);
    settings.Set("Debuggers - Tile Viewer", "tile_size", tileSize);
    settings.Save();
}

void TileViewer::setMemory(const char* emulatorType, uint8_t* data, bool isCGB) {
    memoryData = data;
    uint8_t et = -1;
    if (strcmp(emulatorType, "dmg") == 0)
        et = 1;
    else if (strcmp(emulatorType, "agb") == 0)
        et = 2;
    else
        et = 0;
    bool changed = et != this->emulatorType || isCGB != isCGBLoaded;
    this->emulatorType = et;
    isCGBLoaded = isCGB;
    if (changed)
        initializeData(et);
}

void TileViewer::setVRAMBankCallback(std::function<uint8_t(uint16_t, uint8_t)> vramReadBank) {
    funcVramReadBank = vramReadBank;
}

void TileViewer::setPaletteRamCallback(std::function<const uint8_t* (bool isOBJ)> getPaletteRAM) {
    funcGetPaletteRAM = getPaletteRAM;
}

void TileViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1 && !isCGBLoaded) {
        tiles.clear();
        tiles.reserve(DMG_TilesCount);
        const uint8_t* vram = memoryData + DMG_Address_TileStart;
        for (uint32_t i = 0; i < DMG_TilesCount; i++) {
            const uint8_t* vramAddress = vram + i * 16;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);
            TileItem tile(i, address);
            decodeTile(vramAddress, tile);
            tiles.push_back(tile);
        }
    }
    else if (emulatorType == 1 && isCGBLoaded) {
        tiles.clear();
        tiles.reserve(CGB_TilesCount);
        for (int i = 0; i < CGB_TilesCount; i++) {
            int bank = i / DMG_TilesCount;
            int tileIndexInBank = i % DMG_TilesCount;
            uint16_t address = DMG_Address_TileStart + tileIndexInBank * 16;
            uint8_t tileBytes[16];
            for (int b = 0; b < 16; b++)
                tileBytes[b] = funcVramReadBank(address + b, (uint8_t)bank);
            TileItem tile(i, address);
            decodeTile(tileBytes, tile);
            tiles.push_back(tile);
        }
    }
}

void TileViewer::decodeTile(const uint8_t* tileData, TileItem& tile) {
    if (emulatorType == 1) {
        for (uint8_t y = 0; y < 8; y++) {
            uint8_t low = tileData[y * 2];
            uint8_t high = tileData[y * 2 + 1];
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t bit = 7 - x;
                uint8_t lo = (low >> bit) & 1;
                uint8_t hi = (high >> bit) & 1;
                tile.Pixels[x][y] = (hi << 1) | lo; // from 0 to 3
            }
        }
    }
}

void TileViewer::render(bool* windowOpened) {
    const float widthVerticalSeparator = 2.0f;
    const float widthRightPanel = 240.0f;

    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Tile Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!memoryData) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    if (autoRefresh)
        initializeData(emulatorType);

    if (isCGBLoaded) {
        ImGui::Checkbox("OBJ Palette", &cgbUseObjPalette);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        ImGui::SliderInt("##cgbPaletteIndex", &cgbPaletteIndex, 0, 7);
    }
    else {
        ImGui::SetNextItemWidth(120);
        static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
        if (ImGui::Combo("##paletteChoicesCombo", &paletteViewer.paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
            settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteViewer.paletteChoicesSelected);
            settings.Save();
        }
        ImGui::SameLine();
        ImGui::Text("Palette transformer");
    }

    ImGui::SetNextItemWidth(120);
    static const char* tileSizeChoices[] = { "8 x 8", "8 x 16" };
    if (ImGui::Combo("##tileSizeCombo", &tileSize, tileSizeChoices, IM_ARRAYSIZE(tileSizeChoices))) {
        settings.Set("Debuggers - Tile Viewer", "tile_size", tileSize);
        settings.Save();
    }
    ImGui::SameLine();
    ImGui::Text("Tile size");

    ImGui::Checkbox("Show grid", &showGrid);

    ImGui::Checkbox("Auto refresh", &autoRefresh);
    ImGui::SameLine();
    if (autoRefresh)
        ImGui::BeginDisabled();
    if (ImGui::Button("Refresh"))
        initializeData(emulatorType);
    if (autoRefresh)
        ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::Text("Tiles size");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat("##tileSize", &zoomPerPixel, 1.0f, 16.0f);

    ImGui::Separator();

    float totalH = ImGui::GetContentRegionAvail().y;
    float totalW = ImGui::GetContentRegionAvail().x;
    float leftW = totalW - widthRightPanel - widthVerticalSeparator;

    ImGui::BeginChild("childLeft", ImVec2(leftW, totalH), ImGuiChildFlags_ResizeX);
    renderTiles();
    ImGui::EndChild();

    // separator
    {
        ImVec2 rMin = ImGui::GetItemRectMin();
        ImVec2 rMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x, rMax.y), ImGui::GetColorU32(ImGuiCol_Separator), widthVerticalSeparator);
        ImGui::SameLine(0, widthVerticalSeparator);
    }

    ImGui::BeginChild("childRight", ImVec2(0, totalH), ImGuiChildFlags_None);
    renderTilePreview();
    ImGui::EndChild();

    ImGui::End();
}

int TileViewer::pickHoveredSlot(ImVec2 start, float tileStepX, float tileStepY, int tilesPerRow, int count) {
    if (!ImGui::IsItemHovered())
        return -1;
    ImVec2 mouse = ImGui::GetIO().MousePos;
    int tileX = (int)((mouse.x - start.x) / tileStepX);
    int tileY = (int)((mouse.y - start.y) / tileStepY);
    if (tileX < 0 || tileX >= tilesPerRow || tileY < 0)
        return -1;
    int slot = tileY * tilesPerRow + tileX;
    if (slot < 0 || slot >= count)
        return -1;
    return slot;
}

void TileViewer::renderTiles() {
    float tileSizeZoom = 8.0f * zoomPerPixel;
    float gridGap = showGrid ? 1.0f : 0.0f;
    bool stacked = tileSize == 1;
    int unitHeightTiles = stacked ? 2 : 1;
    float tileStepX = tileSizeZoom + gridGap;
    float tileStepY = tileSizeZoom * unitHeightTiles + gridGap;
    int tilesPerRow = std::max(1, (int)(ImGui::GetContentRegionAvail().x / tileStepX));

    int cgbBankOffset = 0;
    if (isCGBLoaded) {
        ImGui::RadioButton("Bank 0", &cgbBank, 0); ImGui::SameLine();
        ImGui::RadioButton("Bank 1", &cgbBank, 1);
        cgbBankOffset = cgbBank * DMG_TilesCount;
    }

    if (ImGui::BeginTabBar("Tiles", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Tiles 1 (0x8000)", nullptr, ImGuiTabItemFlags_None)) {
            ImGui::BeginChild("TilesScroll1", ImVec2(0, 0), ImGuiChildFlags_None);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            int totalTiles = isCGBLoaded ? DMG_TilesCount : tiles.Size;
            int unitCount1 = stacked ? (totalTiles + 1) / 2 : totalTiles; // 192 stacked
            for (int u = 0; u < unitCount1; u++) {
                int topIndex = stacked ? u * 2 : u;
                int bottomIndex = topIndex + 1;
                bool hasBottom = stacked && (bottomIndex < totalTiles);
                int tx = u % tilesPerRow;
                int ty = u / tilesPerRow;
                ImVec2 pos(start.x + tx * tileStepX, start.y + ty * tileStepY);
                drawTileUnit(draw_list, tiles[topIndex + cgbBankOffset], hasBottom ? tiles[bottomIndex + cgbBankOffset] : tiles[topIndex + cgbBankOffset], hasBottom, pos, zoomPerPixel);
            }
            int numRows1 = (unitCount1 + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStepX, numRows1 * tileStepY));
            int slot = pickHoveredSlot(start, tileStepX, tileStepY, tilesPerRow, unitCount1);
            if (slot >= 0) {
                int topIndex = stacked ? slot * 2 : slot;
                int bottomIndex = topIndex + 1;
                bool hasBottom = stacked && (bottomIndex < totalTiles);
                hoveredTileItem = tiles[topIndex + cgbBankOffset];
                hoveredHasBottom = hasBottom;
                if (hasBottom)
                    hoveredTileItemBottom = tiles[bottomIndex + cgbBankOffset];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected) {
                        selectedTileItem = tiles[topIndex + cgbBankOffset];
                        selectedHasBottom = hasBottom;
                        if (hasBottom)
                            selectedTileItemBottom = tiles[bottomIndex + cgbBankOffset];
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tiles 2 (0x8800 signed)", nullptr, ImGuiTabItemFlags_None)) {
            ImGui::BeginChild("TilesScroll2", ImVec2(0, 0), ImGuiChildFlags_None);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            int unitCount2 = stacked ? 128 : 256;
            for (int u = 0; u < unitCount2; u++) {
                int i0 = stacked ? u * 2 : u;
                int topIndex = 256 + (int8_t)i0;
                int bottomIndex = 256 + (int8_t)(i0 + 1);
                bool hasBottom = stacked;
                int tx = u % tilesPerRow;
                int ty = u / tilesPerRow;
                ImVec2 pos(start.x + tx * tileStepX, start.y + ty * tileStepY);
                drawTileUnit(draw_list, tiles[topIndex + cgbBankOffset], hasBottom ? tiles[bottomIndex + cgbBankOffset] : tiles[topIndex + cgbBankOffset], hasBottom, pos, zoomPerPixel);
            }
            int numRows2 = (unitCount2 + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStepX, numRows2 * tileStepY));
            int slot = pickHoveredSlot(start, tileStepX, tileStepY, tilesPerRow, unitCount2);
            if (slot >= 0) {
                int i0 = stacked ? slot * 2 : slot;
                int topIndex = 256 + (int8_t)i0;
                int bottomIndex = 256 + (int8_t)(i0 + 1);
                hoveredTileItem = tiles[topIndex + cgbBankOffset];
                hoveredHasBottom = stacked;
                if (stacked)
                    hoveredTileItemBottom = tiles[bottomIndex + cgbBankOffset];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected) {
                        selectedTileItem = tiles[topIndex + cgbBankOffset];
                        selectedHasBottom = stacked;
                        if (stacked)
                            selectedTileItemBottom = tiles[bottomIndex + cgbBankOffset];
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("OBJ Tiles", nullptr, ImGuiTabItemFlags_None)) {
            ImGui::BeginChild("TilesScroll3", ImVec2(0, 0), ImGuiChildFlags_None);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            int count = 0;
            uint8_t objTileIndices[40];
            int objBankOffsets[40];
            for (int oam = 0; oam < 160; oam += 4) {
                uint8_t tileIndex = memoryData[DMG_Address_TileOBJ + oam + 2];
                uint8_t oamFlags = memoryData[DMG_Address_TileOBJ + oam + 3];
                int objBankOffset = (isCGBLoaded && (oamFlags & 0x08)) ? DMG_TilesCount : 0;
                objTileIndices[count] = tileIndex;
                objBankOffsets[count] = objBankOffset;
                int topIndex = (stacked ? (tileIndex & 0xFE) : tileIndex) + objBankOffset;
                int bottomIndex = (tileIndex | 0x01) + objBankOffset;
                int tx = count % tilesPerRow;
                int ty = count / tilesPerRow;
                ImVec2 pos(start.x + tx * tileStepX, start.y + ty * tileStepY);
                drawTileUnit(draw_list, tiles[topIndex], stacked ? tiles[bottomIndex] : tiles[topIndex], stacked, pos, zoomPerPixel);
                count++;
            }
            int numRows3 = (40 + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStepX, numRows3 * tileStepY));
            int slot = pickHoveredSlot(start, tileStepX, tileStepY, tilesPerRow, 40);
            if (slot >= 0) {
                uint8_t raw = objTileIndices[slot];
                int objBankOffset = objBankOffsets[slot];
                int topIndex = (stacked ? (raw & 0xFE) : raw) + objBankOffset;
                int bottomIndex = (raw | 0x01) + objBankOffset;
                hoveredTileItem = tiles[topIndex];
                hoveredHasBottom = stacked;
                if (stacked)
                    hoveredTileItemBottom = tiles[bottomIndex];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected) {
                        selectedTileItem = tiles[topIndex];
                        selectedHasBottom = stacked;
                        if (stacked)
                            selectedTileItemBottom = tiles[bottomIndex];
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void TileViewer::drawTileUnit(ImDrawList* draw_list, const TileItem& top, const TileItem& bottom, bool hasBottom, ImVec2 pos, float pixelSize) {
    drawTile(draw_list, top, pos, pixelSize, false);
    if (hasBottom)
        drawTile(draw_list, bottom, ImVec2(pos.x, pos.y + pixelSize * 8.0f), pixelSize, false);
    if (showGrid) {
        float w = pixelSize * 8.0f;
        float h = pixelSize * 8.0f * (hasBottom ? 2.0f : 1.0f);
        draw_list->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(60, 60, 60, 255));
    }
}

void TileViewer::drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            PaletteColor color = isCGBLoaded ? resolveCGBColor(tile.Pixels[x][y]) : paletteViewer.getColorPalette(tile.Pixels[x][y]);
            ImU32 col = IM_COL32((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), 255);
            ImVec2 p0(pos.x + x * pixelSize, pos.y + y * pixelSize);
            ImVec2 p1(p0.x + pixelSize, p0.y + pixelSize);
            draw_list->AddRectFilled(p0, p1, col);
        }
    }
    if (showGrid && drawBorder) {
        float s = pixelSize * 8.0f;
        draw_list->AddRect(pos, ImVec2(pos.x + s, pos.y + s), IM_COL32(60, 60, 60, 255));
    }
}

void TileViewer::renderTilePreview() {
    if (tiles.Size == 0)
        return;

    const TileItem& item = previewSelected ? selectedTileItem : hoveredTileItem;
    bool hasBottom = previewSelected ? selectedHasBottom : hoveredHasBottom;
    const TileItem& bottomItem = previewSelected ? selectedTileItemBottom : hoveredTileItemBottom;

    if (hasBottom) {
        ImGui::Text("   Top tile: %02X : %i (Address %02X:%04X)", item.TileItemID, item.TileItemID, item.TileItemID / DMG_TilesCount, item.TileAddress);
        ImGui::Text("Bottom tile: %02X : %i (Address %02X:%04X)", bottomItem.TileItemID, bottomItem.TileItemID, bottomItem.TileItemID / DMG_TilesCount, bottomItem.TileAddress);
    }
    else
        ImGui::Text("Tile: %02X : %i (Address %02X:%04X)", item.TileItemID, item.TileItemID, item.TileItemID / DMG_TilesCount, item.TileAddress);

    ImGui::Separator();

    ImGui::Text("Size:");
    ImGui::SameLine();
    ImGui::SliderFloat("##previewSize", &previewSize, 2.0f, 64.0f);

    ImGui::Separator();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();
    drawTileUnit(draw_list, item, bottomItem, hasBottom, ImVec2(start.x + 2.0f, start.y + 2.0f), previewSize);
    ImGui::Dummy(ImVec2(previewSize * 8.0f + 4.0f, previewSize * 8.0f * (hasBottom ? 2.0f : 1.0f) + 4.0f));
}

PaletteColor TileViewer::resolveCGBColor(uint8_t colorId) const {
    if (!funcGetPaletteRAM)
        return { 1.0f, 0.0f, 1.0f }; // some color
    const uint8_t* pal = funcGetPaletteRAM(cgbUseObjPalette);
    uint16_t idx = cgbPaletteIndex * 8 + colorId * 2;
    uint16_t rgb555 = pal[idx] | (pal[idx + 1] << 8);
    uint8_t r5 = rgb555 & 0x1F;
    uint8_t g5 = (rgb555 >> 5) & 0x1F;
    uint8_t b5 = (rgb555 >> 10) & 0x1F;
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g5 << 3) | (g5 >> 2);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    return { r8 / 255.0f, g8 / 255.0f, b8 / 255.0f };
}