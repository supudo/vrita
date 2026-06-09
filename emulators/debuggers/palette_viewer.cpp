#include "palette_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool PaletteViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Palette Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Palette Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Palette Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Palette Viewer", "height", 300);
    return true;
}

void PaletteViewer::release() {
    settings.Set("Debuggers - Palette Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Palette Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Palette Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Palette Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void PaletteViewer::setMemory(const char* emulatorType, uint8_t* data, uint32_t size) {
    memoryData = data;
    memorySize = size;
    if (emulatorType = "dmg")
        this->emulatorType = 1;
    else if (emulatorType = "agb")
        this->emulatorType = 2;
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

    ImGui::BeginChild("##palettes");

    float previewHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetTextLineHeightWithSpacing() * 2.0f + ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
    float tableHeight = ImGui::GetContentRegionAvail().y - previewHeight;
    tableHeight = std::max(tableHeight, ImGui::GetFrameHeightWithSpacing() * 3.0f);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##paletteviewer", 18, tableFlags, ImVec2(0, tableHeight))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::EndTable();
    }

    // BGP
    ImGui::BeginChild("##bgp", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);
    ImGui::Text("BGP");
    ImGui::EndChild();

    // OBP0
    ImGui::Text("OBP0");

    // OBP1
    ImGui::Text("OBP1");

    ImGui::EndChild();

    ImGui::End();
}