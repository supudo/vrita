#include "paletteviewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool PaletteViewer::init(Settings settings) {
    windowPositionX = settings.GetInt("Debuggers - Palette Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Palette Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Palette Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Palette Viewer", "height", 300);
    return true;
}

void PaletteViewer::release(Settings& settings) {
    settings.Set("Debuggers - Palette Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Palette Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Palette Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Palette Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void PaletteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Palette Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    ImGui::End();
}