#include "tilemap_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool TilemapViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Tilemap Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tilemap Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tilemap Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tilemap Viewer", "height", 300);
    tileMapAddress = settings.GetInt("Debuggers - Tilemap Viewer", "tile_map_address", 0);
    return true;
}

void TilemapViewer::release() {
    settings.Set("Debuggers - Tilemap Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tilemap Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tilemap Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tilemap Viewer", "height", (int)lastWindowSize.y);
    settings.Set("Debuggers - Tile Viewer", "tile_map_address", tileMapAddress);
    settings.Save();
}

void TilemapViewer::setMemory(const char* emulatorType, uint8_t* data) {
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

void TilemapViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1) {
    }
}

void TilemapViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!ImGui::Begin("Debuggers - Tilemap Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    if (autoRefresh)
        initializeData(emulatorType);

    ImGui::SetNextItemWidth(120);
    static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
    if (ImGui::Combo("##paletteChoicesCombo", &paletteViewer.paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
        settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteViewer.paletteChoicesSelected);
        settings.Save();
    }
    ImGui::SameLine();
    ImGui::Text("Palette transformer");

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

    renderTileMap();

    ImGui::End();
}

void TilemapViewer::renderTileMap() {
    ImGui::BeginChild("TileMap", ImVec2(0, 0), ImGuiChildFlags_None);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    ImGui::Text("Tile map comes here ...");

    ImGui::EndChild();
}

void TilemapViewer::renderTileMapInfo() {
    ImGui::Separator();
    ImGui::Text("Tile info comes here ...");
}