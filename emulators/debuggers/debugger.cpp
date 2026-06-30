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
#include "utilities/iconfonts/IconsFontAwesome7.h"

#include "debuggers_defines_dmg.hpp"
#include "debuggers_defines_dmg.inl"
#include "debuggers_defines_cgb.hpp"
#include "debuggers_defines_cgb.inl"
#include "debuggers_defines_agb.hpp"
#include "debuggers_defines_agb.inl"

bool Debugger::init() {
    windowPositionX = settings.GetInt("Debuggers - Debugger", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Debugger", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Debugger", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Debugger", "height", 300);

    initRegisters();

    return true;
}

void Debugger::setCallbacks(std::function<uint8_t(uint16_t)> read8,
                            std::function<void(uint16_t, uint8_t)> write8,
                            std::function<bool(uint8_t)> getFlag,
                            std::function<bool(uint8_t)> interruptsEnabled,
                            std::function<bool()> isGameRunning,
                            std::function<void()> stopGame,
                            std::function<void()> startGame) {
    funcMemoryRead = read8;
    funcMemoryWrite = write8;
    funcCpuGetFlag = getFlag;
    funcInterruptsEnabled = interruptsEnabled;
    funcIsGameRunning = isGameRunning;
    funcStopGame = stopGame;
    funcStartGame = startGame;
}

void Debugger::release() {
    settings.Set("Debuggers - Debugger", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Debugger", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Debugger", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Debugger", "height", (int)lastWindowSize.y);
    settings.Save();
}

void Debugger::setMemory(const char* emulatorType, uint32_t size) {
    memorySize = size;
    if (strcmp(emulatorType, "dmg") == 0)
        this->emulatorType = 1;
    else if (strcmp(emulatorType, "agb") == 0)
        this->emulatorType = 2;
    else
        this->emulatorType = 0;
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

    if ((int)memorySize == 0) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    if (!gameIsRunning && funcIsGameRunning)
        funcStopGame();
    else if (gameIsRunning && !funcIsGameRunning)
        funcStartGame();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::SetItemTooltip(gameIsRunning ? "Pause" : "Run");
    if (ImGui::Button(gameIsRunning ? ICON_FA_PAUSE : ICON_FA_PLAY, ImVec2(40, 32))) {
        gameIsRunning = !gameIsRunning;
        if (gameIsRunning) funcStartGame();
        else funcStopGame();
    }
    ImGui::SameLine();
    ImGui::SetItemTooltip("Step Over");
    if (ImGui::Button(ICON_FA_ARROW_TURN_DOWN, ImVec2(40, 32))) {
    }
    ImGui::SameLine();
    ImGui::SetItemTooltip("Step Into");
    if (ImGui::Button(ICON_FA_ARROW_DOWN, ImVec2(40, 32))) { 
    }
    ImGui::SameLine();
    ImGui::SetItemTooltip("Step Out");
    if (ImGui::Button(ICON_FA_ARROW_UP, ImVec2(40, 32))) {
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();
    
    renderPerspective(registers);
    
    ImGui::End();
}

void Debugger::renderPerspective(DMGCpuRegisters& registers) {
    const float widthVerticalSeparator = 2.0f;
    const float heightSplitter = 4.0f;
    const float widthRightPanel = 240.0f;
    const float heightCPULoad = 130.0f;

    float totalH = ImGui::GetContentRegionAvail().y;
    float totalW = ImGui::GetContentRegionAvail().x;
    float leftW = totalW - widthRightPanel - widthVerticalSeparator;

    // left panel
    ImGui::BeginChild("childLeft", ImVec2(leftW, totalH), ImGuiChildFlags_ResizeX);
    {
        float availH = ImGui::GetContentRegionAvail().y;
        float memH = std::clamp(memoryPanelHeight, 50.0f, availH - heightSplitter - 50.0f);
        float assemblyH = availH - memH - heightSplitter;

        ImGui::BeginChild("childAssembly", ImVec2(0, assemblyH), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        renderAssembly();
        ImGui::EndChild();

        ImGui::InvisibleButton("splitterLeftH", ImVec2(-1, heightSplitter));
        if (ImGui::IsItemActive())
            memoryPanelHeight = std::clamp(memoryPanelHeight - ImGui::GetIO().MouseDelta.y, 50.0f, availH - heightSplitter - 50.0f);
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        {
            ImVec2 sMin = ImGui::GetItemRectMin();
            ImVec2 sMax = ImGui::GetItemRectMax();
            float midY = (sMin.y + sMax.y) * 0.5f;
            ImU32 col = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_SeparatorActive : ImGuiCol_Separator);
            ImGui::GetWindowDrawList()->AddLine(ImVec2(sMin.x, midY), ImVec2(sMax.x, midY), col);
        }

        ImGui::BeginChild("childRest", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        renderRest();
        ImGui::EndChild();
    }
    ImGui::EndChild();

    // separator
    {
        ImVec2 rMin = ImGui::GetItemRectMin();
        ImVec2 rMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(rMax.x, rMin.y), ImVec2(rMax.x, rMax.y), ImGui::GetColorU32(ImGuiCol_Separator), widthVerticalSeparator);
        ImGui::SameLine(0, widthVerticalSeparator);
    }

    // right panel
    ImGui::BeginChild("childRight", ImVec2(0, totalH), ImGuiChildFlags_None);
    {
        float rightAvailH = ImGui::GetContentRegionAvail().y;
        float sepH = ImGui::GetStyle().ItemSpacing.y * 2.0f + 1.0f;
        float registersH = rightAvailH - heightCPULoad - sepH;

        ImGui::BeginChild("childRegisters", ImVec2(0, registersH), ImGuiChildFlags_None);
        renderRegisters(registers);
        ImGui::EndChild();

        ImGui::Separator();

        ImGui::BeginChild("childCPU", ImVec2(0, heightCPULoad), ImGuiChildFlags_None);
        renderCPULoad();
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

// region: Assembly quadrant

void Debugger::renderAssembly() {
    ImGui::Text("Assembly");
}

// region: Rest quadrant

void Debugger::renderRest() {
    if (ImGui::BeginTabBar("Rest", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Memory")) {
            ImGui::SetItemTooltip("View memory data");
            renderRestMemory();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Expressions")) {
            ImGui::SetItemTooltip("Type custom expression");
            renderRestCustomExpression();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Breakpoints")) {
            ImGui::SetItemTooltip("View and manage all breakpoints");
            renderRestBreakpoints();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Overlays")) {
            ImGui::SetItemTooltip("Add custom overlay");
            renderRestOverlays();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void Debugger::renderRestMemory() {
    ImVec2 paneSize(140, 0);
    ImVec2 buttonSize(140, 0);
    if (emulatorType == 1) {
        ImGui::BeginChild("Memory Regions", paneSize);
        for (size_t r = 0; r < MemoryMap_DMG_Default.size(); r++) {
            if (ImGui::Selectable(MemoryMap_DMG_Default.data()[r].region, selectedMemoryRegion == r, 0, buttonSize)) {
                ImGui::SetItemTooltip(MemoryMap_DMG_Default.data()[r].notes);
                selectedMemoryRegion = (int)r;
            }
        }
        ImGui::EndChild();

        // vertical separator
        {
            ImVec2 rMin = ImGui::GetItemRectMin();
            ImVec2 rMax = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddLine(ImVec2(rMax.x + 1, rMin.y), ImVec2(rMax.x + 1, rMax.y), ImGui::GetColorU32(ImGuiCol_Separator));
            ImGui::SameLine(0, 3.0f);
        }

        renderMemoryRegion();
    }
}

void Debugger::renderRestCustomExpression() {
}

void Debugger::renderRestBreakpoints() {}

void Debugger::renderRestOverlays() {}

void Debugger::renderMemoryRegion() {
    if (!funcMemoryRead)
        ImGui::TextDisabled("N/A");
    MemoryRegion region = MemoryMap_DMG_Default.data()[selectedMemoryRegion];
    uint32_t regionStart = region.range.start;
    uint32_t regionEnd = region.range.end;
    if (regionStart >= memorySize)
        return;
    regionEnd = std::min(regionEnd, memorySize - 1);
    uint32_t regionSize = regionEnd - regionStart + 1;

    float previewHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetTextLineHeightWithSpacing() * 2.0f + ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
    float tableHeight = std::max(ImGui::GetContentRegionAvail().y, ImGui::GetFrameHeightWithSpacing());

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##memoryeditor", 18, tableFlags, ImVec2(0, tableHeight))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        for (int i = 0; i < 16; i++) {
            char label[3];
            snprintf(label, sizeof(label), "%X", i);
            ImGui::TableSetupColumn(label);
        }
        ImGui::TableSetupColumn("Dump");
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin((int)((regionSize + 15) / 16));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                uint32_t addr = regionStart + row * 16;
                if (addr >= memorySize) break;

                ImGui::TableNextRow();

                // Address
                ImGui::TableSetColumnIndex(0);
                if (this->emulatorType == 1)
                    ImGui::Text("%04X", addr);
                else
                    ImGui::Text("%08X", addr);

                // Bytes
                char ascii[17] = {};
                for (int col = 0; col < 16; col++) {
                    ImGui::TableSetColumnIndex(col + 1);
                    if (addr + col < memorySize) {
                        uint8_t b = funcMemoryRead(addr + col);
                        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%02X", b);
                        uint8_t final_b = funcMemoryRead(addr + col);
                        ascii[col] = (final_b >= 32 && final_b < 127) ? (char)final_b : '.';
                    }
                }

                // Dump
                ImGui::TableSetColumnIndex(17);
                if (!region.editable)
                    ImGui::TextUnformatted(ascii);
                else
                    for (int col = 0; col < 16; col++) {
                        uint32_t current_addr = addr + col;
                        if (current_addr >= memorySize)
                            break;
                        uint8_t value = funcMemoryRead(current_addr);
                        char c[2];
                        c[0] = (value >= 32 && value <= 126) ? static_cast<char>(value) : '.';
                        c[1] = '\0';
                        ImGui::PushID(current_addr);
                        ImGui::SetNextItemWidth(12);
                        ImGui::Text(c, sizeof(c));
                        ImGui::PopID();
                        if (col != 15)
                            ImGui::SameLine(0.0f, 0.0f);
                    }
            }
        }
        ImGui::EndTable();
    }
}

// region: CPU Load quadrant

void Debugger::renderCPULoad() {
    ImGui::SeparatorText("CPU Usage (% per Frame)");

    static float values[90] = {};
    static int values_offset = 0;
    static double refresh_time = 0.0;
    if (gameIsRunning || refresh_time == 0.0)
        refresh_time = ImGui::GetTime();
    while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
    {
        static float phase = 0.0f;
        values[values_offset] = cosf(phase);
        values_offset = (values_offset + 1) % IM_COUNTOF(values);
        phase += 0.10f * values_offset;
        refresh_time += 1.0f / 60.0f;
    }

    float average = 0.0f;
    for (int n = 0; n < IM_COUNTOF(values); n++)
        average += values[n];
    average /= (float)IM_COUNTOF(values);
    ImGui::PlotLines("##cpuload", values, IM_COUNTOF(values), values_offset, nullptr, -1.0f, 1.0f, ImGui::GetContentRegionAvail());
}