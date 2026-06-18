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

    registerNodes = {
        // Registers
        { nullptr, "Registers", 0, 1, 8, NDT_Hex8, NVS_None, 0, true, true },
        { nullptr, "BC", 0, -1, 0, NDT_Hex16, NVS_RegBC, 0, false },
        { nullptr, "DE", 0, -1, 0, NDT_Hex16, NVS_RegDE, 0, false },
        { nullptr, "HL", 0, -1, 0, NDT_Hex16, NVS_RegHL, 0, false },
        { nullptr, "AF", 0, -1, 0, NDT_Hex16, NVS_RegAF, 0, false },
        { nullptr, "SP", 0, -1, 0, NDT_Hex16, NVS_RegSP, 0, false },
        { nullptr, "PC", 0, -1, 0, NDT_Hex16, NVS_RegPC, 0, false },
        { [this](DebuggerRegisterTreeNode* n){ renderFlags(n); }, "Flags", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { nullptr, "Interrupts", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // PPU
        { nullptr, "PPU", 0, 10, 11, NDT_Hex8, NVS_None, 0, true, true },

        { nullptr, "LCDC ($FF40)", 0xFF40, 21, 8, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "STAT ($FF41)", 0xFF41, 29, 6, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SCY ($FF42)", 0xFF42, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SCX ($FF43)", 0xFF43, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "LY ($FF44)", 0xFF44, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "LYC ($FF45)", 0xFF45, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "BGP ($FF47)", 0xFF47, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "OBP0 ($FF48)", 0xFF48, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "OBP1 ($FF49)", 0xFF49, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "WY ($FF4A)", 0xFF4A, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "WX ($FF4B)", 0xFF4B, -1, 0, NDT_Hex8, NVS_Memory, 0, false },

        // LCDC children
        { nullptr, "Bit 7 - LCD display enable", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 6 - Window tile map display select", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 5 - Window display enable", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 4 - BG & Window tile data select", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 3 - BG tile map display select", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 2 - OBJ (Sprite) size", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 1 - OBJ (Sprite) display enable", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 0 - BG/Window display/priority", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // STAT children
        { nullptr, "Bit 6 - LYC=LY coincidence interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 5 - Mode 2 OAM interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 4 - Mode 1 V-Blank interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 3 - Mode 0 H-Blank interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 2 - Coincidence flag", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Bit 1 - Mode flag", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // APU
        { nullptr, "APU", 0, 36, 23, NDT_Hex8, NVS_None, 0, false, true },

        { nullptr, "NR50 ($FF24)", 0xFF24, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR51 ($FF25)", 0xFF25, 58, 2, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR52 ($FF26)", 0xFF26, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR10 ($FF10)", 0xFF10, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR11 ($FF11)", 0xFF11, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR12 ($FF12)", 0xFF12, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR13 ($FF13)", 0xFF13, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR14 ($FF14)", 0xFF14, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR21 ($FF16)", 0xFF16, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR22 ($FF17)", 0xFF17, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR23 ($FF18)", 0xFF18, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR24 ($FF19)", 0xFF19, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR30 ($FF1A)", 0xFF1A, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR31 ($FF1B)", 0xFF1B, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR32 ($FF1C)", 0xFF1C, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR33 ($FF1D)", 0xFF1D, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR34 ($FF1E)", 0xFF1E, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR41 ($FF20)", 0xFF20, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR42 ($FF21)", 0xFF21, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR43 ($FF22)", 0xFF22, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR44 ($FF23)", 0xFF23, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "Wave pattern", 0, 60, 16, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 1 (SQ1)", 0, 76, 10, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 2 (SQ2)", 0, 86, 7, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 3 (WAV)", 0, 93, 5, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 4 (NOI)", 0, 98, 8, NDT_Hex8, NVS_None, 0, false },

        // APU STAT children
        { nullptr, "Channels left", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channels right", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // Wave pattern children
        { nullptr, "[$FF30]", 0xFF30, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF31]", 0xFF31, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF32]", 0xFF32, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF33]", 0xFF33, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF34]", 0xFF34, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF35]", 0xFF35, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF36]", 0xFF36, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF37]", 0xFF37, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF38]", 0xFF38, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF39]", 0xFF39, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3A]", 0xFF3A, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3B]", 0xFF3B, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3C]", 0xFF3C, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3D]", 0xFF3D, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3E]", 0xFF3E, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "[$FF3F]", 0xFF3F, -1, 0, NDT_Hex8, NVS_Memory, 0, false },

        // Channel 1 children
        { nullptr, "Cycles to next sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Index", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles until length expires", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Volume", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Envelope Direction", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles to next envelope", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sweep Frequency", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sweep Addend", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles to next sweep", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // Channel 2 children
        { nullptr, "Cycles to next sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Index", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles until length expires", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Volume", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Envelope Direction", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles to next envelope", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // Channel 3 children
        { nullptr, "Cycles to next sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Index", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles until length expires", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Volume", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // Channel 4 children
        { nullptr, "Cycles to next sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Sample", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles until length expires", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Volume", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Envelope direction", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Cycles to next envelope", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "LSFR", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Noise counter", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // Cartridge
        { nullptr, "Cartridge", 0, 107, 3, NDT_Hex8, NVS_None, 0, true, true },
        { nullptr, "Title", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Type", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "ROM Bank", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // GameBoy
        { nullptr, "GameBoy", 0, 111, 9, NDT_Hex8, NVS_None, 0, false, true },

        { nullptr, "Input", 0, 120, 8, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "IE ($FFFF)", 0xFFFF, 128, 5, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "IF ($FF0F)", 0xFF0F, 133, 5, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "DIV ($FF04)", 0xFF04, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TIMA ($FF05)", 0xFF05, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TMA ($FF06)", 0xFF06, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TAC ($FF07)", 0xFF07, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "JOYP ($FF00)", 0xFF00, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SB ($FF01)", 0xFF01, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SC ($FF02)", 0xFF02, -1, 0, NDT_Hex8, NVS_Memory, 0, false },

        // Input children
        { nullptr, "A", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "B", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "SELECT", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "START", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "RIGHT", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "LEFT", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "UP", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "DOWN", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // IE children
        { nullptr, "V-Blank Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "LCD STAT Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Timer Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Serial Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Joypad Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // IF children
        { nullptr, "V-Blank Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "LCD STAT Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Timer Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Serial Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Joypad Interrupt", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
    };

    return true;
}

void Debugger::setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8) {
    memoryRead = read8;
    memoryWrite = write8;
}

void Debugger::release() {
    settings.Set("Debuggers - Debugger", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Debugger", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Debugger", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Debugger", "height", (int)lastWindowSize.y);
    settings.Save();
}

void Debugger::setMemory(const char* emulatorType) {
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

    if (this->emulatorType) {
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
                        uint8_t b = memoryRead(addr + col);
                        ImGui::TextColored(ImVec4(1, 1, 1, 1), "%02X", b);
                        uint8_t final_b = memoryRead(addr + col);
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
                        uint8_t value = memoryRead(current_addr);
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

uint8_t Debugger::getAddressValue8(uint32_t address) const {
    return memoryRead(address);
}

void Debugger::renderFlags(DebuggerRegisterTreeNode* node) {
    ImGui::Text("F - Z -");
}

void Debugger::renderRegisterValue(DebuggerRegisterTreeNode* node) {
    if (node->Type == NDT_Custom && node->renderCustom)
        node->renderCustom(node);
    else {
        switch (node->Type) {
            case NDT_Hex16: ImGui::Text("$%04X", node->Value); break;
            default: ImGui::Text("$%02X", node->Value); break;
        }
    }
}

void Debugger::renderRegisterNode(DebuggerRegisterTreeNode* node, bool isRoot) {
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
            renderRegisterValue(node);
        }
        if (open) {
            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                renderRegisterNode(&registerNodes[node->ChildIdx + child_n]);
            ImGui::TreePop();
        }
    }
    else {
        ImGui::TreeNodeEx(node->Name, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TableNextColumn();
        renderRegisterValue(node);
    }
}

void Debugger::renderRegisters(DMGCpuRegisters& registers) {
    float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("dRegisters", 2, table_flags)) {
        ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("$XX", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableHeadersRow();

        for (auto& node : registerNodes) {
            switch (node.Source) {
                case NVS_None: break;
                case NVS_Memory: node.Value = getAddressValue8(node.Address); break;
                case NVS_RegBC: node.Value = registers.BC; break;
                case NVS_RegDE: node.Value = registers.DE; break;
                case NVS_RegHL: node.Value = registers.HL; break;
                case NVS_RegAF: node.Value = registers.AF; break;
                case NVS_RegSP: node.Value = registers.SP; break;
                case NVS_RegPC: node.Value = registers.PC; break;
            }
        }

        for (auto& node : registerNodes)
            if (node.isRoot)
                renderRegisterNode(&node, true);

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