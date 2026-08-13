#include "debugger.hpp"
#include "assembly_dmg.inl"
#include "utilities/iconfonts/IconsFontAwesome7.h"
#include "utilities/fonts.hpp"

void Debugger::initEditor() {
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);
}

void Debugger::disassemblySource() {
    if (!editorSourceSet) {
        if (!funcMemoryRead)
            return; // try next frame?

        editorSourceSet = true;

        // editorAssembly.SetText(assemblySampleDMG);

        std::string assemblySource;
        uint32_t address = 0x0000;
        int line = 0;
        const int maxInstructions = 0x8000;
        assemblySource.reserve(static_cast<size_t>(maxInstructions) * 24);
        
        for (int i = 0; i < maxInstructions && address <= 0xFFFF; i++) {
            const uint16_t instructionAddress = static_cast<uint16_t>(address);
            const uint8_t opcode = funcMemoryRead(instructionAddress);
            DisassembledInstruction instruction = disassembleInstruction(instructionAddress, opcode, funcMemoryRead);

            char prefix[8];
            snprintf(prefix, sizeof(prefix), "$%04X     ", instructionAddress);
            assemblySource += prefix;

            assemblySource += formatBytes(instruction);

            for (uint8_t j = instruction.length; j < 3; ++j)
                assemblySource += "   ";

            assemblySource += "  ";

            assemblySource += instructionToString(instruction.mnemonic);

            bool first = true;

            for (const auto& operand : instruction.operands) {
                if (operand.type == OperandType::None)
                    continue;
                assemblySource += first ? " " : ", ";
                assemblySource += instructionFormatOperand(operand);

                first = false;
            }

            //assemblySource += instructionFormat(instruction);
            assemblySource += "\n";

            line++;
            address += instruction.length;
        }

        editorAssembly.SetText(assemblySource);
    }
}

void Debugger::renderAssembly(DMGCpuRegisters& registers, float height) {
    disassemblySource();

    if (!gameIsRunning && funcIsGameRunning())
        funcStopGame();
    else if (gameIsRunning && !funcIsGameRunning())
        funcStartGame();

    ImGui::BeginChild("childAssembly", ImVec2(0, height), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushFont(VritaFontSmall);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    bool wasGameRunning = gameIsRunning;
    if (wasGameRunning) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_PLAY)) {
        gameIsRunning = true;
        funcStartGame();
    }
    if (wasGameRunning) ImGui::EndDisabled();
    ImGui::SetItemTooltip("Run");
    ImGui::SameLine();
    if (!wasGameRunning) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_PAUSE)) {
        gameIsRunning = false;
        funcStopGame();
    }
    if (!wasGameRunning) ImGui::EndDisabled();
    ImGui::SetItemTooltip("Pause");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_DOWN)) {
    }
    ImGui::SetItemTooltip("Step In");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_DOWN)) {
    }
    ImGui::SetItemTooltip("Step Over");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_UP)) {
    }
    ImGui::SetItemTooltip("Step Back");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_UP)) {
    }
    ImGui::SetItemTooltip("Step Return");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_UP_RIGHT_FROM_SQUARE)) {
    }
    ImGui::SetItemTooltip("Advance Frame");

    // separator
    ImGui::SameLine();
    {
        float spacing = 8.0f;
        float lineHeight = ImGui::GetFrameHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = p.x + spacing * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(lineX, p.y), ImVec2(lineX, p.y + lineHeight), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
        ImGui::Dummy(ImVec2(spacing, lineHeight));
    }

    ImGui::SameLine();
    if (ImGui::Button(breakpointsDisabled ? ICON_FA_TOGGLE_ON : ICON_FA_TOGGLE_OFF)) {
        breakpointsDisabled = !breakpointsDisabled;
    }
    ImGui::SetItemTooltip(breakpointsDisabled ? "Disable Breakpoints" : "Enable Breakpoints");

    // separator
    ImGui::SameLine();
    {
        float spacing = 8.0f;
        float lineHeight = ImGui::GetFrameHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = p.x + spacing * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(lineX, p.y), ImVec2(lineX, p.y + lineHeight), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
        ImGui::Dummy(ImVec2(spacing, lineHeight));
    }

    ImGui::SameLine();
    if (ImGui::Button(logCPUCalls ? "Log CPU Calls OFF" : "Log CPU Calls ON")) {
        logCPUCalls = !logCPUCalls;
        funcLogCPUCalls(logCPUCalls);
    }
    ImGui::SetItemTooltip("Log CPU calls");
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::Separator();

    editorAssembly.ClearMarkers();
    editorAssembly.AddMarker(editorAssembly.GetCurrentCursorPosition().line, IM_COL32(55, 55, 60, 255), IM_COL32(55, 55, 60, 255), "", "");

    editorAssembly.Render("Assembly");

    ImGui::EndChild();
}
