#include "sprite_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool SpriteViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Sprite Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Sprite Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Sprite Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Sprite Viewer", "height", 300);
    return true;
}

void SpriteViewer::release() {
    settings.Set("Debuggers - Sprite Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Sprite Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Sprite Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Sprite Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void SpriteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Sprite Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    ImGui::End();
}