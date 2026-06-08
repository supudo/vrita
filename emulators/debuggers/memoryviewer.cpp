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

void MemoryViewer::setMemory(const uint8_t* data, uint32_t size) {
    memoryData = data;
    memorySize = size;
}

void MemoryViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Memory Viewer", windowOpened)) {
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

    static uint32_t gotoAddr = 0;
    ImGui::SetNextItemWidth(120);
    ImGui::InputScalar("##memoryviewergoto", ImGuiDataType_U32, &gotoAddr, nullptr, nullptr, "%04X");
    ImGui::SameLine();
    if (ImGui::Button("Jump"))
        scrollToAddrress = (int)(gotoAddr & ~0xF);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##memoryviewer", 18, tableFlags, ImVec2(0, 0))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        for (int i = 0; i < 16; i++) {
            char label[3];
            snprintf(label, sizeof(label), "%X", i);
            ImGui::TableSetupColumn(label);
        }
        ImGui::TableSetupColumn("Dump");
        ImGui::TableHeadersRow();

        if (scrollToAddrress >= 0) {
            int targetRow = scrollToAddrress / 16;
            ImGui::SetScrollY((float)targetRow * ImGui::GetTextLineHeightWithSpacing());
            scrollToAddrress = -1;
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)(memorySize / 16));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                uint32_t addr = (uint32_t)(row * 16);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%04X", addr);

                char ascii[17] = {};
                for (int col = 0; col < 16; col++) {
                    ImGui::TableSetColumnIndex(col + 1);
                    uint8_t b = memoryData[addr + col];
                    ImGui::Text("%02X", b);
                    ascii[col] = (b >= 32 && b < 127) ? (char)b : '.';
                }

                ImGui::TableSetColumnIndex(17);
                ImGui::TextUnformatted(ascii);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}