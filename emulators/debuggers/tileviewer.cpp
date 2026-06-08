#include "tileviewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool TileViewer::init(Settings settings) {
    windowPositionX = settings.GetInt("Debuggers - Tile Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tile Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tile Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tile Viewer", "height", 300);
    return true;
}

void TileViewer::release(Settings& settings) {
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

    ImGui::End();
}