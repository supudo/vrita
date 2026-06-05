#include "memoryviewer.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool MemoryViewer::init(Settings settings) {
    windowPositionX = settings.GetInt("Debuggers - Memory Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Memory Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Memory Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Memory Viewer", "height", 300);
    return true;
}

void MemoryViewer::release(Settings& settings) {
    settings.Set("Debuggers - Memory Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Memory Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Memory Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Memory Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void MemoryViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    ImGui::Begin("Debuggers - Memory Viewer", windowOpened);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    ImGui::End();
}