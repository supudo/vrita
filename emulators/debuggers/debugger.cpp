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
    return true;
}

void Debugger::release() {
    settings.Set("Debuggers - Debugger", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Debugger", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Debugger", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Debugger", "height", (int)lastWindowSize.y);
    settings.Save();
}

void Debugger::setMemory(const char* emulatorType, uint8_t* data, uint32_t size) {
    memoryData = data;
    memorySize = size;
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
    if (ImGui::Button("Run", ImVec2(80, 26))) { 
        running = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause", ImVec2(80, 26))) { 
        running = false;
    }
    ImGui::PopStyleColor(3);

    ImGui::Separator();
    
    renderPerspective();
    
    ImGui::End();
}

void Debugger::renderPerspective() {
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
        renderRegisters();
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
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(rMax.x + 1, rMin.y),
                ImVec2(rMax.x + 1, rMax.y),
                ImGui::GetColorU32(ImGuiCol_Separator)
            );
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
                        uint8_t b = memoryData[addr + col];
                        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%02X", b);
                        uint8_t final_b = memoryData[addr + col];
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
                        uint8_t value = memoryData[current_addr];
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

// region: Registers quadrant

void Debugger::renderRegisters() {
    if (ImGui::TreeNodeEx("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TreeNodeEx("BC", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("DE", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("HL", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("AF", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("SP", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("PC", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Flags", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Interrupts", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("PPU", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::TreeNodeEx("LCDC ($FF40)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Bit 7 - LCD display enable:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 6 - Window tile map display select:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 5 - Window display enable:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 4 - BG & Window tile data select:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 3 - BG tile map display select:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 2 - OBJ (Sprite) size:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 1 - OBJ (Spirte) display enable:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 0 - BG/Window display/priority:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("STAT ($FF41)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Bit 6 - LYC=LY coincidence interrupt:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 5 - Mode 2 OAM interrupt:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 4 - Mode 1 V-Blank interrupt:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 3 - Mode 0 H-Blank interrupt:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 2 - Coincidence flag:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Bit 1 - Mode flag:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }

        ImGui::TreeNodeEx("SCY ($FF42)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("SCX ($FF43)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("LY ($FF414", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("LYC ($FF45)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("BGP ($FF47)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("OBP0 ($FF48)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("OBP1 ($FF49)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("WY ($FF4A)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("WX ($FF4B)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("APU")) {
        ImGui::TreeNodeEx("NR50 ($FF24)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        if (ImGui::TreeNodeEx("STAT ($FF41)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Channels left:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Channels right:", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        ImGui::TreeNodeEx("NR52 ($FF26)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR10 ($FF10)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR11 ($FF11)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR12 ($FF12)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR13 ($FF13)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR14 ($FF14)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR21 ($FF16)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR22 ($FF17)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR23 ($FF18)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR24 ($FF19)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR30 ($FF1A)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR31 ($FF1B)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR32 ($FF1C)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR33 ($FF1D)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR34 ($FF1E)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR41 ($FF20)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR42 ($FF21)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR43 ($FF22)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("NR44 ($FF23)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        if (ImGui::TreeNodeEx("Wave pattern", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("[$FF30]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF31]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF32]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF33]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF34]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF35]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF36]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF37]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF38]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF39]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3A]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3B]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3C]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3D]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3E]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("[$FF3F]", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Channel 1 (SQ1)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Cycles to next sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Index", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles until length expires", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Volume", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Envelope Direction", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles to next envelope", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sweep Frequency", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sweep Addend", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles to next sweep", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Channel 2 (SQ2)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Cycles to next sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Index", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles until length expires", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Volume", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Envelope Direction", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles to next envelope", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Channel 3 (WAV)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Cycles to next sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Index", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles until length expires", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Volume", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Channel 4 (NOI)", ImGuiTreeNodeFlags_None)) {
            ImGui::TreeNodeEx("Cycles to next sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Sample", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles until length expires", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Volume", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Envelope direction", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Cycles to next envelope", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("LSFR", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Noise counter", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Cartridge", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TreeNodeEx("Title", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("Type", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("ROM Bank", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("GameBoy")) {
        if (ImGui::TreeNodeEx("Input")) {
            ImGui::TreeNodeEx("A", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("B", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("SELECT", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("START", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("RIGHT", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("LEFT", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("UP", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("DOWN", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("IE ($FFFF)")) {
            ImGui::TreeNodeEx("V - Blank Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("LCD STAT Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Timer Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Serial Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Joypad Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("IF ($FF0F)")) {
            ImGui::TreeNodeEx("V - Blank Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("LCD STAT Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Timer Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Serial Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreeNodeEx("Joypad Interrupt", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
            ImGui::TreePop();
        }
        ImGui::TreeNodeEx("DIV ($FF04)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("TIMA ($FF05)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("TMA ($FF06)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("TAC ($FF07)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("JOYP ($FF00)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("SB ($FF01)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreeNodeEx("SC ($FF02)", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TreePop();
    }
}

// region: CPU Load quadrant

void Debugger::renderCPULoad() {
    ImGui::SeparatorText("CPU Usage (% per Frame)");

    static float values[90] = {};
    static int values_offset = 0;
    static double refresh_time = 0.0;
    if (!running || refresh_time == 0.0)
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