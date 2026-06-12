#include "tile_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool TileViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Tile Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tile Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tile Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tile Viewer", "height", 300);
    return true;
}

void TileViewer::setMemory(const char* emulatorType, uint8_t* data) {
    memoryData = data;
    uint8_t et = -1;
    if (emulatorType == "dmg")
        et = 1;
    else if (emulatorType == "agb")
        et = 2;
    else
        et = 0;
    if (et != this->emulatorType)
        initializeData(et);
    this->emulatorType = et;
}

void TileViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1) {
        tiles.clear();
        tiles.reserve(DMG_TilesCount);
        const uint8_t* vram = memoryData + DMG_TileAddressStart;
        for (uint32_t i = 0; i < DMG_TilesCount; i++, nextTileID++) {
            TileItem tile(i);
            decodeTile(vram + i * 16, tile);
            tiles.push_back(tile);
        }
    }
}

void TileViewer::decodeTile(const uint8_t* tileData, TileItem& tile) {
    for (uint8_t y = 0; y < 8; y++) {
        uint8_t low = tileData[y * 2];
        uint8_t high = tileData[y * 2 + 1];
        for (uint8_t x = 0; x < 8; x++) {
            uint8_t bit = 7 - x;
            uint8_t lo = (low >> bit) & 1;
            uint8_t hi = (high >> bit) & 1;
            uint8_t colorId = (hi << 1) | lo; // from 0 to 3
            PaletteColor color = paletteViewer.getColorPalette(colorId);
            tile.pixels[x][y] = { color.r, color.g, color.b, 1.0f };
        }
    }
}

void TileViewer::release() {
    tiles.clear();
    settings.Set("Debuggers - Tile Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tile Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tile Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tile Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void TileViewer::render(bool* windowOpened) {
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

    ImGui::Text("Tiles per row:");
    ImGui::SameLine();
    ImGui::SliderInt("##tilesPerRow", &tilesPerRow, 8, 64);

    ImGui::Text("Tiles size:");
    ImGui::SameLine();
    ImGui::SliderFloat("##tileSize", &zoomPerPixel, 1.0f, 64.0f);

    ImGui::Separator();

    float tileSize = 8 + zoomPerPixel;

    //logger.log("START ======================================================================");
    if (ImGui::BeginTabBar("Tiles", ImGuiTabBarFlags_None)) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 start = ImGui::GetCursorScreenPos();
        if (ImGui::BeginTabItem("Tiles 1 (0x8000)", nullptr, ImGuiTabItemFlags_None)) {
            //logger.log("Tiles 1");
            for (int i = 0; i < tiles.Size; i++) {
                int tx = i % tilesPerRow;
                int ty = i / tilesPerRow;
                ImVec2 tilePos(start.x + tx * tileSize, start.y  + ty * tileSize);
                drawTile(draw_list, tiles[i], tilePos, tileSize);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tiles 2 (0x8800 signed)", nullptr, ImGuiTabItemFlags_None)) {
            for (int i = 0; i < 256; i++) {
                const uint8_t* tileData = memoryData + DMG_TileAddressStart + 0x1000 + (int8_t)i * 16;
                TileItem temp(0);
                decodeTile(tileData, temp);
                int x = i % tilesPerRow;
                int y = i / tilesPerRow;
                drawTile(draw_list, temp, ImVec2(start.x + x * tileSize, start.y + y * tileSize), tileSize);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("OBJ Tiles", nullptr, ImGuiTabItemFlags_None)) {
            int count = 0;
            for (int oam = 0; oam < 160; oam += 4) {
                uint8_t tileIndex = memoryData[DMG_TileAddressOBJ + oam + 2];
                const uint8_t* tileData = memoryData + DMG_TileAddressStart + tileIndex * 16;
                TileItem temp(0);
                decodeTile(tileData, temp);
                int x = count % tilesPerRow;
                int y = count / tilesPerRow;
                drawTile(draw_list, temp, ImVec2(start.x + x * tileSize, start.y + y * tileSize), tileSize);
                count++;
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    //logger.log("END ======================================================================");

    ImGui::End();
}

void TileViewer::drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            const TileColor& c = tile.pixels[x][y];
            ImU32 col = IM_COL32((int)(c.r * 255.0f), (int)(c.g * 255.0f), (int)(c.b * 255.0f), (int)(c.a * 255.0f));
            //logger.log("%.2f %.2f %.2f %.2f = %i, %i, %i, %i", c.r, c.g, c.b, c.a, (int)(c.r * 255.0f), (int)(c.g * 255.0f), (int)(c.b * 255.0f), (int)(c.a * 255.0f));
            ImVec2 p0(pos.x + x * pixelSize, pos.y + y + pixelSize);
            ImVec2 p1(p0.x + pixelSize, p0.y + pixelSize);
            draw_list->AddRectFilled(p0, p1, col);
        }
    }
}