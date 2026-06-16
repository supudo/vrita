#include "debugger.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <imgui_internal.h>

#include "utilities/settings.hpp"
#include "emulators/dmg/cpu_registers.hpp"

bool Debugger::init() {
    windowPositionX = settings.GetInt("Debuggers - Debugger", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Debugger", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Debugger", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Debugger", "height", 300);
    return true;
}

void Debugger::release() {
    settings.Set("Debuggers - Debugger", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Debugger", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Debugger", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Debugger", "height", (int)lastWindowSize.y);
    settings.Save();
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

void Debugger::render(bool* windowOpened, DMGCpuRegisters& registers) {
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

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    if (ImGui::Button("Step", ImVec2(80, 26))) { }
    ImGui::SameLine();
    if (ImGui::Button("Run", ImVec2(80, 26))) { }
    ImGui::SameLine();
    if (ImGui::Button("Pause", ImVec2(80, 26))) { }
    ImGui::PopStyleColor(3);
    
    // ===============

    //renderPerspective1();
    renderPerspective2();
    
    // ===============

    ImGui::End();
}

void Debugger::renderPerspective2() {
    const float vSepWidth = 2.0f;
    const float splitterH = 4.0f;
    const float rightW = 210.0f;
    const float cpuLoadH = 110.0f;

    float totalH = ImGui::GetContentRegionAvail().y;
    float totalW = ImGui::GetContentRegionAvail().x;
    float leftW = totalW - rightW - vSepWidth;

    // left panel
    ImGui::BeginChild("childLeft", ImVec2(leftW, totalH), ImGuiChildFlags_None);
    {
        float availH = ImGui::GetContentRegionAvail().y;
        float memH = std::clamp(memoryPanelHeight, 50.0f, availH - splitterH - 50.0f);
        float assemblyH = availH - memH - splitterH;

        ImGui::BeginChild("childAssembly", ImVec2(0, assemblyH), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Assembly");
        ImGui::EndChild();

        ImGui::InvisibleButton("splitterLeftH", ImVec2(-1, splitterH));
        if (ImGui::IsItemActive())
            memoryPanelHeight = std::clamp(memoryPanelHeight - ImGui::GetIO().MouseDelta.y, 50.0f, availH - splitterH - 50.0f);
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        {
            ImVec2 sMin = ImGui::GetItemRectMin();
            ImVec2 sMax = ImGui::GetItemRectMax();
            float midY = (sMin.y + sMax.y) * 0.5f;
            ImU32 col = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_SeparatorActive : ImGuiCol_Separator);
            ImGui::GetWindowDrawList()->AddLine(ImVec2(sMin.x, midY), ImVec2(sMax.x, midY), col);
        }

        ImGui::BeginChild("childMemory", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Memory");
        ImGui::EndChild();
    }
    ImGui::EndChild();

    // separator
    {
        ImVec2 rMin = ImGui::GetItemRectMin();
        ImVec2 rMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x, rMax.y), ImGui::GetColorU32(ImGuiCol_Separator), vSepWidth);
        ImGui::SameLine(0, vSepWidth);
    }

    // right panel
    ImGui::BeginChild("childRight", ImVec2(0, totalH), ImGuiChildFlags_None);
    {
        float rightAvailH = ImGui::GetContentRegionAvail().y;
        float sepH = ImGui::GetStyle().ItemSpacing.y * 2.0f + 1.0f;
        float registersH = rightAvailH - cpuLoadH - sepH;

        ImGui::BeginChild("childRegisters", ImVec2(0, registersH), ImGuiChildFlags_None);
        ImGui::Text("Registers");
        ImGui::EndChild();

        ImGui::Separator();

        ImGui::BeginChild("childCPU", ImVec2(0, cpuLoadH), ImGuiChildFlags_None);
        ImGui::Text("CPU Load");
        ImGui::EndChild();
    }
    ImGui::EndChild();

    //float tableHeight = ImGui::GetContentRegionAvail().y;
    //ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable;
    //if (ImGui::BeginTable("tableDebugger", 2, tableFlags, ImVec2(0, tableHeight))) {
    //    ImGui::TableNextRow();

    //    ImGui::TableSetColumnIndex(0);
    //    ImGui::Text("Assembly");

    //    ImGui::TableSetColumnIndex(1);
    //    ImGui::Text("Registers");
    //    
    //    ImGui::TableNextRow();

    //    ImGui::TableSetColumnIndex(0);
    //    ImGui::Text("Registers");

    //    ImGui::TableSetColumnIndex(1);
    //    ImGui::Text("CPU Load");
    //    
    //    ImGui::EndTable();
    //}

    //if (ImGui::BeginTable("table1", 3)) {
    //    for (int row = 0; row < 4; row++) {
    //        ImGui::TableNextRow();
    //        for (int column = 0; column < 3; column++) {
    //            ImGui::TableSetColumnIndex(column);
    //            ImGui::Text("Row %d Column %d", row, column);
    //        }
    //    }
    //    ImGui::EndTable();
    //}
}

void Debugger::renderPerspective1() {
    float tableHeight = ImGui::GetContentRegionAvail().y;
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame;
    if (ImGui::BeginTable("grid", 2, tableFlags, ImVec2(0, tableHeight))) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        {
            ImGuiTable* table = GImGui->CurrentTable;
            if (table && table->MinColumnWidth > 0.0f && table->Columns.size() > 0 && table->Columns[0].WidthGiven < 150.0f)
                ImGui::TableSetColumnWidth(0, 150.0f);
        }
        ImGui::TableNextRow();

        // row 1

        // col 1
        ImGui::TableSetColumnIndex(0);
        ImGui::SeparatorText("Assembly");

        // col 2
        ImGui::TableSetColumnIndex(1);
        ImGui::SeparatorText("Memory");

        // row 2
        ImGui::TableNextRow();

        // col 1
        ImGui::TableSetColumnIndex(0);
        ImGui::SeparatorText("Registers");

        // col 2
        ImGui::TableSetColumnIndex(1);
        ImGui::SeparatorText("CPU Load");

        ImGui::EndTable();
    }
}