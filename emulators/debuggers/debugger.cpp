#include "debugger.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool Debugger::init() {
    windowPositionX = settings.GetInt("Debuggers - Debugger", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Debugger", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Debugger", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Debugger", "height", 300);
    return true;
}

void Debugger::setMemory(const char* emulatorType, uint8_t* data) {
    memoryData = data;
    if (strcmp(emulatorType, "dmg") == 0) {
        this->emulatorType = 1;
    }
    else if (strcmp(emulatorType, "agb") == 0) {
        this->emulatorType = 2;
    }
    else {
        this->emulatorType = 0;
    }
}

void Debugger::release() {
    settings.Set("Debuggers - Debugger", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Debugger", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Debugger", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Debugger", "height", (int)lastWindowSize.y);
    settings.Save();
}

void Debugger::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Debugger", windowOpened)) {
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

    ImGui::End();
}