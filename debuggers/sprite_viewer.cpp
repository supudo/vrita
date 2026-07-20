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

void SpriteViewer::setMemory(const char* emulatorType, uint8_t* data) {
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

void SpriteViewer::initializeData(uint8_t emulatorType) {
}

void SpriteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!ImGui::Begin("Debuggers - Sprite Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    if (!memoryData) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    ImGui::End();
}