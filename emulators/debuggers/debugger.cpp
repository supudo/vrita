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

uint8_t Debugger::getAddressValue(uint32_t address) const {
    if (address >= memorySize)
        return 0;
    return memoryData[address];
}

void Debugger::renderRegisters() {
    float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
    
    if (ImGui::BeginTable("dRegisters", 2, table_flags)) {
        ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("$XX", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableHeadersRow();

        struct RegisterTreeNode {
            const char* Name;
            bool isOpenedByDefault;
            uint8_t Value;
            int ChildIdx;
            int ChildCount;
            static void DisplayNode(const RegisterTreeNode* node, const RegisterTreeNode* all_nodes, bool isRoot = false) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool is_folder = (node->ChildCount > 0);

                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_LabelSpanAllColumns;
                if (!isRoot)
                    node_flags &= ~ImGuiTreeNodeFlags_LabelSpanAllColumns;

                if (is_folder) {
                    if (node->isOpenedByDefault)
                        node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
                    bool open = ImGui::TreeNodeEx(node->Name, node_flags);
                    if ((node_flags & ImGuiTreeNodeFlags_LabelSpanAllColumns) == 0) {
                        ImGui::TableNextColumn();
                        ImGui::Text("$%02X", node->Value);
                    }
                    if (open) {
                        for (int child_n = 0; child_n < node->ChildCount; child_n++)
                            DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                        ImGui::TreePop();
                    }
                }
                else {
                    ImGui::TreeNodeEx(node->Name, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                    ImGui::TableNextColumn();
                    ImGui::Text("$%02X", node->Value);
                }
            }
        };
        
        const RegisterTreeNode nodes[] = {
            // Registers
            { "Registers", true, 0, 1, 8 },
            { "BC", false, 0, -1, 0 },
            { "DE", false, 0, -1, 0 },
            { "HL", false, 0, -1, 0 },
            { "AF", false, 0, -1, 0 },
            { "SP", false, 0, -1, 0 },
            { "PC", false, 0, -1, 0 },
            { "Flags", false, 0, -1, 0 },
            { "Interrupts", false, 0, -1, 0 },

            // PPU
            { "PPU", true, 0, 10, 11 },

            { "LCDC ($FF40)", false, getAddressValue(0xFF40), 21, 8 },
            { "STAT ($FF41)", false, getAddressValue(0xFF41), 29, 6 },
            { "SCY ($FF42)", false, getAddressValue(0xFF42), -1, 0 },
            { "SCX ($FF43)", false, getAddressValue(0xFF43), -1, 0 },
            { "LY ($FF44)", false, getAddressValue(0xFF44), -1, 0 },
            { "LYC ($FF45)", false, getAddressValue(0xFF45), -1, 0 },
            { "BGP ($FF47)", false, getAddressValue(0xFF47), -1, 0 },
            { "OBP0 ($FF48)", false, getAddressValue(0xFF48), -1, 0 },
            { "OBP1 ($FF49)", false, getAddressValue(0xFF49), -1, 0 },
            { "WY ($FF4A)", false, getAddressValue(0xFF4A), -1, 0 },
            { "WX ($FF4B)", false, getAddressValue(0xFF4B), -1, 0 },

            // LCDC children
            { "Bit 7 - LCD display enable", false, 0, -1, 0 },
            { "Bit 6 - Window tile map display select", false, 0, -1, 0 },
            { "Bit 5 - Window display enable", false, 0, -1, 0 },
            { "Bit 4 - BG & Window tile data select", false, 0, -1, 0 },
            { "Bit 3 - BG tile map display select", false, 0, -1, 0 },
            { "Bit 2 - OBJ (Sprite) size", false, 0, -1, 0 },
            { "Bit 1 - OBJ (Sprite) display enable", false, 0, -1, 0 },
            { "Bit 0 - BG/Window display/priority", false, 0, -1, 0 },

            // STAT children
            { "Bit 6 - LYC=LY coincidence interrupt", false, 0, -1, 0 },
            { "Bit 5 - Mode 2 OAM interrupt", false, 0, -1, 0 },
            { "Bit 4 - Mode 1 V-Blank interrupt", false, 0, -1, 0 },
            { "Bit 3 - Mode 0 H-Blank interrupt", false, 0, -1, 0 },
            { "Bit 2 - Coincidence flag", false, 0, -1, 0 },
            { "Bit 1 - Mode flag", false, 0, -1, 0 },

            // APU
            { "APU", false, 0, 36, 23 },

            { "NR50 ($FF24)", false, getAddressValue(0xFF24), -1, 0 },
            { "NR51 ($FF25)", false, getAddressValue(0xFF25), 58, 2 },
            { "NR52 ($FF26)", false, getAddressValue(0xFF26), -1, 0 },
            { "NR10 ($FF10)", false, getAddressValue(0xFF10), -1, 0 },
            { "NR11 ($FF11)", false, getAddressValue(0xFF11), -1, 0 },
            { "NR12 ($FF12)", false, getAddressValue(0xFF12), -1, 0 },
            { "NR13 ($FF13)", false, getAddressValue(0xFF13), -1, 0 },
            { "NR14 ($FF14)", false, getAddressValue(0xFF14), -1, 0 },
            { "NR21 ($FF16)", false, getAddressValue(0xFF16), -1, 0 },
            { "NR22 ($FF17)", false, getAddressValue(0xFF17), -1, 0 },
            { "NR23 ($FF18)", false, getAddressValue(0xFF18), -1, 0 },
            { "NR24 ($FF19)", false, getAddressValue(0xFF19), -1, 0 },
            { "NR30 ($FF1A)", false, getAddressValue(0xFF1A), -1, 0 },
            { "NR31 ($FF1B)", false, getAddressValue(0xFF1B), -1, 0 },
            { "NR32 ($FF1C)", false, getAddressValue(0xFF1C), -1, 0 },
            { "NR33 ($FF1D)", false, getAddressValue(0xFF1D), -1, 0 },
            { "NR34 ($FF1E)", false, getAddressValue(0xFF1E), -1, 0 },
            { "NR41 ($FF20)", false, getAddressValue(0xFF20), -1, 0 },
            { "NR42 ($FF21)", false, getAddressValue(0xFF21), -1, 0 },
            { "NR43 ($FF22)", false, getAddressValue(0xFF22), -1, 0 },
            { "NR44 ($FF23)", false, getAddressValue(0xFF23), -1, 0 },
            { "Wave pattern", false, 0, 60, 16 },
            { "Channel 1 (SQ1)", false, 0, 76, 10 },
            { "Channel 2 (SQ2)", false, 0, 86, 7 },
            { "Channel 3 (WAV)", false, 0, 93, 5 },
            { "Channel 4 (NOI)", false, 0, 98, 8 },

            // APU STAT children
            { "Channels left", false, 0, -1, 0 },
            { "Channels right", false, 0, -1, 0 },

            // Wave pattern children
            { "[$FF30]", false, getAddressValue(0xFF30), -1, 0 },
            { "[$FF31]", false, getAddressValue(0xFF31), -1, 0 },
            { "[$FF32]", false, getAddressValue(0xFF32), -1, 0 },
            { "[$FF33]", false, getAddressValue(0xFF33), -1, 0 },
            { "[$FF34]", false, getAddressValue(0xFF34), -1, 0 },
            { "[$FF35]", false, getAddressValue(0xFF35), -1, 0 },
            { "[$FF36]", false, getAddressValue(0xFF36), -1, 0 },
            { "[$FF37]", false, getAddressValue(0xFF37), -1, 0 },
            { "[$FF38]", false, getAddressValue(0xFF38), -1, 0 },
            { "[$FF39]", false, getAddressValue(0xFF39), -1, 0 },
            { "[$FF3A]", false, getAddressValue(0xFF3A), -1, 0 },
            { "[$FF3B]", false, getAddressValue(0xFF3B), -1, 0 },
            { "[$FF3C]", false, getAddressValue(0xFF3C), -1, 0 },
            { "[$FF3D]", false, getAddressValue(0xFF3D), -1, 0 },
            { "[$FF3E]", false, getAddressValue(0xFF3E), -1, 0 },
            { "[$FF3F]", false, getAddressValue(0xFF3F), -1, 0 },

            // Channel 1 children
            { "Cycles to next sample", false, 0, -1, 0 },
            { "Index", false, 0, -1, 0 },
            { "Sample", false, 0, -1, 0 },
            { "Cycles until length expires", false, 0, -1, 0 },
            { "Volume", false, 0, -1, 0 },
            { "Envelope Direction", false, 0, -1, 0 },
            { "Cycles to next envelope", false, 0, -1, 0 },
            { "Sweep Frequency", false, 0, -1, 0 },
            { "Sweep Addend", false, 0, -1, 0 },
            { "Cycles to next sweep", false, 0, -1, 0 },

            // Channel 2 children
            { "Cycles to next sample", false, 0, -1, 0 },
            { "Index", false, 0, -1, 0 },
            { "Sample", false, 0, -1, 0 },
            { "Cycles until length expires", false, 0, -1, 0 },
            { "Volume", false, 0, -1, 0 },
            { "Envelope Direction", false, 0, -1, 0 },
            { "Cycles to next envelope", false, 0, -1, 0 },

            // Channel 3 children
            { "Cycles to next sample", false, 0, -1, 0 },
            { "Index", false, 0, -1, 0 },
            { "Sample", false, 0, -1, 0 },
            { "Cycles until length expires", false, 0, -1, 0 },
            { "Volume", false, 0, -1, 0 },

            // Channel 4 children
            { "Cycles to next sample", false, 0, -1, 0 },
            { "Sample", false, 0, -1, 0 },
            { "Cycles until length expires", false, 0, -1, 0 },
            { "Volume", false, 0, -1, 0 },
            { "Envelope direction", false, 0, -1, 0 },
            { "Cycles to next envelope", false, 0, -1, 0 },
            { "LSFR", false, 0, -1, 0 },
            { "Noise counter", false, 0, -1, 0 },

            // Cartridge
            { "Cartridge", true, 0, 107, 3 },
            { "Title", false, 0, -1, 0 },
            { "Type", false, 0, -1, 0 },
            { "ROM Bank", false, 0, -1, 0 },

            // GameBoy
            { "GameBoy", false, 0, 111, 9 },

            { "Input", false, 0, 120, 8 },
            { "IE ($FFFF)", getAddressValue(0xFFFF), 0, 128, 5 },
            { "IF ($FF0F)", getAddressValue(0xFF0F), 0, 133, 5 },
            { "DIV ($FF04)", getAddressValue(0xFF04), 0, -1, 0 },
            { "TIMA ($FF05)", getAddressValue(0xFF05), 0, -1, 0 },
            { "TMA ($FF06)", getAddressValue(0xFF06), 0, -1, 0 },
            { "TAC ($FF07)", getAddressValue(0xFF07), 0, -1, 0 },
            { "JOYP ($FF00)", getAddressValue(0xFF00), 0, -1, 0 },
            { "SB ($FF01)", getAddressValue(0xFF01), 0, -1, 0 },
            { "SC ($FF02)", getAddressValue(0xFF02), 0, -1, 0 },

            // Input children
            { "A", false, 0, -1, 0 },
            { "B", false, 0, -1, 0 },
            { "SELECT", false, 0, -1, 0 },
            { "START", false, 0, -1, 0 },
            { "RIGHT", false, 0, -1, 0 },
            { "LEFT", false, 0, -1, 0 },
            { "UP", false, 0, -1, 0 },
            { "DOWN", false, 0, -1, 0 },

            // IE children
            { "V-Blank Interrupt", false, 0, -1, 0 },
            { "LCD STAT Interrupt", false, 0, -1, 0 },
            { "Timer Interrupt", false, 0, -1, 0 },
            { "Serial Interrupt", false, 0, -1, 0 },
            { "Joypad Interrupt", false, 0, -1, 0 },

            // IF children
            { "V-Blank Interrupt", false, 0, -1, 0 },
            { "LCD STAT Interrupt", false, 0, -1, 0 },
            { "Timer Interrupt", false, 0, -1, 0 },
            { "Serial Interrupt", false, 0, -1, 0 },
            { "Joypad Interrupt", false, 0, -1, 0 },
        };

        RegisterTreeNode::DisplayNode(&nodes[0],   nodes, true);
        RegisterTreeNode::DisplayNode(&nodes[9],   nodes, true);
        RegisterTreeNode::DisplayNode(&nodes[35],  nodes, true);
        RegisterTreeNode::DisplayNode(&nodes[110], nodes, true);
        RegisterTreeNode::DisplayNode(&nodes[114], nodes, true);

        ImGui::EndTable();
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