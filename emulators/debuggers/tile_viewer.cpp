#include "tile_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

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
    settings.Save();
}

void TileViewer::setMemory(const char* emulatorType, uint8_t* data) {
    memoryData = data;
    uint8_t et = -1;
    if (strcmp(emulatorType, "dmg") == 0)
        et = 1;
    else if (strcmp(emulatorType, "agb") == 0)
        et = 2;
    else
        et = 0;
    bool changed = et != this->emulatorType;
    this->emulatorType = et;
    if (changed)
        initializeData(et);
}

void TileViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1) {
        tiles.clear();
        tiles.reserve(DMG_TilesCount);
        const uint8_t* vram = memoryData + DMG_TileAddressStart;
        uint8_t tileCounterHex = 0x00;
        for (uint32_t i = 0; i < DMG_TilesCount; i++, nextTileID++) {
            const uint8_t* vramAddress = vram + i * 16;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);

            char tileAddress[8];
            snprintf(tileAddress, sizeof(tileAddress), "00:%04X", address);

            TileItem tile(i, tileCounterHex, i, tileAddress);
            decodeTile(vramAddress, tile);
            tiles.push_back(tile);
            ++tileCounterHex;
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
                tile.pixels[x][y] = (hi << 1) | lo; // from 0 to 3
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

    ImGui::Text("Tiles size");
    ImGui::SameLine();
    ImGui::SliderFloat("##tileSize", &zoomPerPixel, 1.0f, 64.0f);

    ImGui::SetNextItemWidth(100);
    static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
    if (ImGui::Combo("##paletteChoicesCombo", &paletteViewer.paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
        settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteViewer.paletteChoicesSelected);
        settings.Save();
    }
    ImGui::SameLine();
    ImGui::Text("Choose palette transformer");
    ImGui::SameLine();
    ImGui::Checkbox("Auto refresh", &autoRefresh);
    ImGui::SameLine();
    ImGui::Checkbox("Show grid", &showGrid);

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

int TileViewer::pickHoveredSlot(ImVec2 start, float tileStep, int tilesPerRow, int count) {
    if (!ImGui::IsItemHovered())
        return -1;
    ImVec2 mouse = ImGui::GetIO().MousePos;
    int tileX = (int)((mouse.x - start.x) / tileStep);
    int tileY = (int)((mouse.y - start.y) / tileStep);
    if (tileX < 0 || tileX >= tilesPerRow || tileY < 0)
        return -1;
    int slot = tileY * tilesPerRow + tileX;
    if (slot < 0 || slot >= count)
        return -1;
    return slot;
}

void TileViewer::renderTiles() {
    float tileSize = 8.0f * zoomPerPixel;
    float tileStep = tileSize + (showGrid ? 1.0f : 0.0f);
    int tilesPerRow = std::max(1, (int)(ImGui::GetContentRegionAvail().x / tileStep));

    if (ImGui::BeginTabBar("Tiles", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Tiles 1 (0x8000)", nullptr, ImGuiTabItemFlags_None)) {
            ImGui::BeginChild("TilesScroll1", ImVec2(0, 0), ImGuiChildFlags_None);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            for (int i = 0; i < tiles.Size; i++) {
                int tx = i % tilesPerRow;
                int ty = i / tilesPerRow;
                drawTile(draw_list, tiles[i], ImVec2(start.x + tx * tileStep, start.y + ty * tileStep), zoomPerPixel);
            }
            int numRows1 = (tiles.Size + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStep, numRows1 * tileStep));
            int slot = pickHoveredSlot(start, tileStep, tilesPerRow, tiles.Size);
            if (slot >= 0) {
                hoveredTileItem = tiles[slot];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected)
                        selectedTileItem = tiles[slot];
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tiles 2 (0x8800 signed)", nullptr, ImGuiTabItemFlags_None)) {
            ImGui::BeginChild("TilesScroll2", ImVec2(0, 0), ImGuiChildFlags_None);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            for (int i = 0; i < 256; i++) {
                int tileIndex = 256 + (int8_t)i;
                int tx = i % tilesPerRow;
                int ty = i / tilesPerRow;
                drawTile(draw_list, tiles[tileIndex], ImVec2(start.x + tx * tileStep, start.y + ty * tileStep), zoomPerPixel);
            }
            int numRows2 = (256 + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStep, numRows2 * tileStep));
            int slot = pickHoveredSlot(start, tileStep, tilesPerRow, 256);
            if (slot >= 0) {
                int idx = 256 + (int8_t)slot;
                hoveredTileItem = tiles[idx];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected)
                        selectedTileItem = tiles[idx];
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
            for (int oam = 0; oam < 160; oam += 4) {
                uint8_t tileIndex = memoryData[DMG_TileAddressOBJ + oam + 2];
                objTileIndices[count] = tileIndex;
                int tx = count % tilesPerRow;
                int ty = count / tilesPerRow;
                drawTile(draw_list, tiles[tileIndex], ImVec2(start.x + tx * tileStep, start.y + ty * tileStep), zoomPerPixel);
                count++;
            }
            int numRows3 = (40 + tilesPerRow - 1) / tilesPerRow;
            ImGui::Dummy(ImVec2(tilesPerRow * tileStep, numRows3 * tileStep));
            int slot = pickHoveredSlot(start, tileStep, tilesPerRow, 40);
            if (slot >= 0) {
                int idx = objTileIndices[slot];
                hoveredTileItem = tiles[idx];
                if (ImGui::IsItemClicked()) {
                    previewSelected = !previewSelected;
                    if (previewSelected)
                        selectedTileItem = tiles[idx];
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void TileViewer::drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            PaletteColor color = paletteViewer.getColorPalette(tile.pixels[x][y]);
            ImU32 col = IM_COL32((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), 255);
            ImVec2 p0(pos.x + x * pixelSize, pos.y + y * pixelSize);
            ImVec2 p1(p0.x + pixelSize, p0.y + pixelSize);
            draw_list->AddRectFilled(p0, p1, col);
        }
    }
    if (showGrid) {
        float s = pixelSize * 8.0f;
        draw_list->AddRect(pos, ImVec2(pos.x + s, pos.y + s), IM_COL32(60, 60, 60, 255));
    }
}

void TileViewer::renderTilePreview() {
    if (tiles.Size == 0)
        return;

    const TileItem& item = previewSelected ? selectedTileItem : hoveredTileItem;

    ImGui::Text("Number: %02X : %i", item.TileNumberHex, item.TileNumberInt);
    ImGui::Text("Address: %s", item.TileAddress);

    ImGui::Separator();

    ImGui::Text("Size:");
    ImGui::SameLine();
    ImGui::SliderFloat("##previewSize", &previewSize, 2.0f, 64.0f);

    ImGui::Separator();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    drawTile(draw_list, item, ImVec2(start.x + 2.0f, start.y + 2.0f), previewSize);
}