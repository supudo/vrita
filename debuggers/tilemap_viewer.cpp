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
    return true;
}

void TilemapViewer::release() {
    settings.Set("Debuggers - Tilemap Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tilemap Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tilemap Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tilemap Viewer", "height", (int)lastWindowSize.y);
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

    if (!ImGui::Begin("Debuggers - Tilemap Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    ImGui::Checkbox("Show grid", &showGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Auto refresh", &autoRefresh);
    ImGui::SameLine();
    if (autoRefresh)
        ImGui::BeginDisabled();
    if (ImGui::Button("Refresh"))
        initializeData(emulatorType);
    if (autoRefresh)
        ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::End();
}