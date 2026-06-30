#include "debugger.hpp"

#include "emulators/dmg/cpu_registers.hpp"

void Debugger::initRegisters() {
    registerNodes = {
        // Registers
        { nullptr, "Registers", 0, 1, 8, NDT_Hex8, NVS_None, 0, true, true },
        { nullptr, "BC", 0, -1, 0, NDT_Hex16, NVS_RegBC, 0, false },
        { nullptr, "DE", 0, -1, 0, NDT_Hex16, NVS_RegDE, 0, false },
        { nullptr, "HL", 0, -1, 0, NDT_Hex16, NVS_RegHL, 0, false },
        { nullptr, "AF", 0, -1, 0, NDT_Hex16, NVS_RegAF, 0, false },
        { nullptr, "SP", 0, -1, 0, NDT_Hex16, NVS_RegSP, 0, false },
        { nullptr, "PC", 0, -1, 0, NDT_Hex16, NVS_RegPC, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderFlags(n); }, "Flags", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInterrupts(n); }, "Interrupts", 0xFFFF, -1, 0, NDT_Custom, NVS_None, 0, false },

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
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 7); }, "Bit 7 - LCD display enable", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 6); }, "Bit 6 - Window tile map display select", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 5); }, "Bit 5 - Window display enable", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 4); }, "Bit 4 - BG & Window tile data select", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 3); }, "Bit 3 - BG tile map display select", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 2); }, "Bit 2 - OBJ (Sprite) size", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 1); }, "Bit 1 - OBJ (Sprite) display enable", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDCBit(n, 0); }, "Bit 0 - BG/Window display/priority", 0xFF40, -1, 0, NDT_Custom, NVS_None, 0, false },

        // STAT children
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 6); }, "Bit 6 - LYC=LY coincidence interrupt", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 5); }, "Bit 5 - Mode 2 OAM interrupt", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 4); }, "Bit 4 - Mode 1 V-Blank interrupt", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 3); }, "Bit 3 - Mode 0 H-Blank interrupt", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 2); }, "Bit 2 - Coincidence flag", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderLCDSBit(n, 1); }, "Bit 1 - Mode flag", 0xFF41, -1, 0, NDT_Custom, NVS_None, 0, false },

        // APU
        { nullptr, "APU", 0, 36, 26, NDT_Hex8, NVS_None, 0, false, true },

        { nullptr, "NR50 ($FF24)", 0xFF24, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "NR51 ($FF25)", 0xFF25, 62, 2, NDT_Hex8, NVS_Memory, 0, false },
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
        { nullptr, "Wave pattern", 0, 64, 16, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 1 (SQ1)", 0, 80, 10, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 2 (SQ2)", 0, 90, 7, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 3 (WAV)", 0, 97, 5, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Channel 4 (NOI)", 0, 102, 8, NDT_Hex8, NVS_None, 0, false },

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
        { nullptr, "Cartridge", 0, 111, 3, NDT_Hex8, NVS_None, 0, true, true },
        { nullptr, "Title", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "Type", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },
        { nullptr, "ROM Bank", 0, -1, 0, NDT_Hex8, NVS_None, 0, false },

        // GameBoy
        { nullptr, "GameBoy", 0, 115, 10, NDT_Hex8, NVS_None, 0, false, true },

        { nullptr, "Input", 0, 125, 8, NDT_None, NVS_None, 0, false },
        { nullptr, "IE ($FFFF)", 0xFFFF, 133, 5, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "IF ($FF0F)", 0xFF0F, 138, 5, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "DIV ($FF04)", 0xFF04, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TIMA ($FF05)", 0xFF05, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TMA ($FF06)", 0xFF06, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "TAC ($FF07)", 0xFF07, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "JOYP ($FF00)", 0xFF00, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SB ($FF01)", 0xFF01, -1, 0, NDT_Hex8, NVS_Memory, 0, false },
        { nullptr, "SC ($FF02)", 0xFF02, -1, 0, NDT_Hex8, NVS_Memory, 0, false },

        // Input children
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, true, 0); }, "A", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, true, 1); }, "B", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, true, 2); }, "SELECT", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, true, 3); }, "START", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, false, 0); }, "RIGHT", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, false, 1); }, "LEFT", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, false, 2); }, "UP", 0, -1, 0, NDT_Custom, NVS_None, 0, false },
        { [this](DebuggerRegisterTreeNode* n) { renderInput(n, false, 3); }, "DOWN", 0, -1, 0, NDT_Custom, NVS_None, 0, false },

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
}

void Debugger::renderRegisterValue(DebuggerRegisterTreeNode* node) {
    if (node->Type == NDT_Custom && node->renderCustom)
        node->renderCustom(node);
    else {
        switch (node->Type) {
            case NDT_Hex8: ImGui::Text("$%02X", node->Value); break;
            case NDT_Hex16: ImGui::Text("$%04X", node->Value); break;
            default: break;
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

uint8_t Debugger::getAddressValue8(uint32_t address) const {
    if (address > memorySize)
        return 0x0000;
    return funcMemoryRead(address);
}

void Debugger::renderFlags(DebuggerRegisterTreeNode* node) {
    if (!funcCpuGetFlag) { 
        ImGui::TextDisabled("N/A"); 
        return; 
    }
    bool is_zero = funcCpuGetFlag(FLAG_ZERO);
    bool is_substract = funcCpuGetFlag(FLAG_SUBTRACT);
    bool is_half_carry = funcCpuGetFlag(FLAG_HALF_CARRY);
    bool is_carry = funcCpuGetFlag(FLAG_CARRY);
    ImGui::Text("%s %s %s %s", (is_zero ? "Z" : "-"), (is_substract ? "S" : "-"), (is_half_carry ? "H" : "-"), (is_carry ? "C" : "-"));
}

void Debugger::renderInterrupts(DebuggerRegisterTreeNode* node) {
    if (!funcInterruptsEnabled) { 
        ImGui::TextDisabled("N/A");
        return; 
    }
    bool interrupt_enabled = funcInterruptsEnabled(node->Address);
    ImGui::Text("%s", interrupt_enabled ? "Enabled" : "Disabled");
}

void Debugger::renderLCDCBit(DebuggerRegisterTreeNode* node, uint8_t bit) {
    uint8_t addressValue = funcMemoryRead(node->Address);
    uint8_t bitValue = (addressValue & (1 << bit));
    switch (bit) {
        case 7:
            ImGui::Text("%s", bitValue == 0 ? "On" : "Off");
            break;
        case 6:
            ImGui::Text("%s", bitValue == 0 ? "$9800 - $9BFF" : "$9C00 - $9FFF");
            break;
        case 5:
            ImGui::Text("%s", bitValue == 0 ? "Off" : "On");
            break;
        case 4:
            ImGui::Text("%s", bitValue == 0 ? "$8800 - $97FF" : "$8000 - $8FFF");
            break;
        case 3:
            ImGui::Text("%s", bitValue == 0 ? "$9800 - $9BFF" : "$9C00 - $9FFF");
            break;
        case 2:
            ImGui::Text("%s", bitValue == 0 ? "8 x 8" : "8 x 16");
            break;
        case 1:
            ImGui::Text("%s", bitValue == 0 ? "Off" : "On");
            break;
        case 0:
            ImGui::Text("%s", bitValue == 0 ? "Off" : "On");
            break;
    }
}

void Debugger::renderLCDSBit(DebuggerRegisterTreeNode* node, uint8_t bit) {
    uint8_t addressValue = funcMemoryRead(node->Address);
    uint8_t bitValue = (addressValue & (1 << bit));
    switch (bit) {
        case 6:
            ImGui::Text("%s", bitValue == 0 ? "Disabled" : "Enabled");
            break;
        case 5:
            ImGui::Text("%s", bitValue == 0 ? "Disabled" : "Enabled");
            break;
        case 4:
            ImGui::Text("%s", bitValue == 0 ? "Disabled" : "Enabled");
            break;
        case 3:
            ImGui::Text("%s", bitValue == 0 ? "Disabled" : "Enabled");
            break;
        case 2:
            ImGui::Text("%s", bitValue == 1 ? "LY == LYC" : "LY <> LYC");
            break;
        case 1:
            switch (bitValue) {
                case 3:
                    ImGui::Text("Mode 3 (Drawing pixels)");
                    break;
                case 2:
                    ImGui::Text("Mode 2 (OAM scan)");
                    break;
                case 0:
                    ImGui::Text("Mode 0 (H-Blank)");
                    break;
            }
            break;
    }
}

void Debugger::renderInput(DebuggerRegisterTreeNode* node, bool isButton, uint8_t bit) {
    uint8_t addressValue = funcMemoryRead(node->Address);
    char checkboxId[12];
    snprintf(checkboxId, sizeof(checkboxId), "##input%d%d", (isButton ? 0 : 1), bit);
    bool checked = ((addressValue & (1 << bit)) == 0);
    if (ImGui::Checkbox(checkboxId, &checked)) {
        if (checked)
            addressValue &= ~(1 << bit); // pressed = 0
        else
            addressValue |= (1 << bit); // released = 1
        funcMemoryWrite(node->Address, addressValue);
    }
}