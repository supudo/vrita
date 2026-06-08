#include "tilemapviewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool TilemapViewer::init(Settings settings) {
    windowPositionX = settings.GetInt("Debuggers - Tilemap Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tilemap Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tilemap Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tilemap Viewer", "height", 300);
    return true;
}

void TilemapViewer::release(Settings& settings) {
    settings.Set("Debuggers - Tilemap Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tilemap Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tilemap Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tilemap Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
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

    ImGui::End();
}