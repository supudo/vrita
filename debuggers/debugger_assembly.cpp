#include "debugger.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include "assembly_dmg.inl"
#include "utilities/iconfonts/IconsFontAwesome7.h"
#include "utilities/fonts.hpp"
#include "emulators/dmg/cpu_registers.hpp"

void Debugger::initEditor() {
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);

    editorAssembly.SetLineNumberContextMenuCallback([this] (TextEditor::PopupData& data) {
        const int32_t line = static_cast<int32_t>(data.pos.line);
        const uint32_t addr = (line >= 0 && static_cast<size_t>(line) < lineToAddress.size()) ? lineToAddress[line] : 0;
        if (ImGui::MenuItem("Set Breakpoint")) {
            breakpoints[addr] = DebuggerBreakpoint{ addr, line, true, false, editorAssembly.GetLineText(static_cast<size_t>(line)) };
        }
        if (ImGui::MenuItem("Remove Breakpoint"))
            breakpoints.erase(addr);
    });
}

void Debugger::disassemblySource(DMGCpuRegisters& registers) {
    if (disassemblyDone.load()) {
        if (disassemblyThread.joinable())
            disassemblyThread.join();

        addressToLine = pendingAddressToLine;
        lineToAddress = std::move(pendingLineToAddress);
        editorAssembly.SetText(pendingAssemblySource);
        pendingAssemblySource.clear();
        pendingAssemblySource.shrink_to_fit();

        disassemblyDone.store(false);
        editorSourceSet = true;
        hideThinking();

        scrollToAddress(registers.PC);
        return;
    }

    if (editorSourceSet || disassemblyStarted)
        return;

    if (!funcMemoryRead)
        return; // try next frame?

    disassemblyStarted = true;
    showThinking();
    disassemblyThread = std::thread(&Debugger::disassembleWork, this);
}

void Debugger::disassembleWork() {
    const auto startTime = std::chrono::steady_clock::now();

    auto localAddressToLine = std::make_unique<std::array<int32_t, 0x10000>>();
    localAddressToLine->fill(-1);

    std::string assemblySource;
    std::vector<uint16_t> localLineToAddress;
    uint32_t address = 0x0000;
    int line = 0;
    const int maxInstructions = 0x8000;
    assemblySource.reserve(static_cast<size_t>(maxInstructions) * 40);
    localLineToAddress.reserve(maxInstructions);

    breakpoints.clear();

    for (int i = 0; i < maxInstructions && address <= 0xFFFF; i++) {
        const uint16_t instructionAddress = static_cast<uint16_t>(address);
        const uint8_t opcode = funcMemoryRead(instructionAddress);
        DisassembledInstruction instruction = disassembleInstruction(instructionAddress, opcode, funcMemoryRead);

        (*localAddressToLine)[instructionAddress] = line;

        char prefix[16];
        snprintf(prefix, sizeof(prefix), "$%04X     ", instructionAddress);
        assemblySource += prefix;

        assemblySource += formatBytes(instruction);

        for (uint8_t j = instruction.length; j < 3; ++j)
            assemblySource += "    ";
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

        assemblySource += "\n";

        localLineToAddress.push_back(instructionAddress);
        line++;
        address += instruction.length;
        thinkingPercentage.store(100.0f * static_cast<float>(address) / 0x10000);
    }

    pendingAddressToLine = *localAddressToLine;
    pendingLineToAddress = std::move(localLineToAddress);
    pendingAssemblySource = std::move(assemblySource);

    const auto elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    logger.log("[Debugger] Disassembly took %.2f ms", elapsedMs);

    disassemblyDone.store(true);
}

void Debugger::scrollToAddress(uint16_t address) {
    if (!editorSourceSet)
        return;

    const int32_t line = addressToLine[address];
    if (line < 0)
        return;

    followedLine = static_cast<size_t>(line);
    editorAssembly.SetCursor(TextEditor::DocPos(followedLine, 0));
    editorAssembly.SelectLine(followedLine);
    editorAssembly.ScrollToLine(followedLine, TextEditor::Scroll::alignMiddle);
}

void Debugger::followPC(DMGCpuRegisters& registers) {
    if (!breakpointsDisabled) {
        for (auto& [addr, bp] : breakpoints)
            bp.isHit = (bp.enabled && addr == registers.PC);
    }

    if (!gameIsRunning)
        return;

    if (!breakpointsDisabled) {
        auto it = breakpoints.find(registers.PC);
        if (it != breakpoints.end() && it->second.enabled) {
            gameIsRunning = false;
            funcStopGame();
        }
    }

    const int32_t line = addressToLine[registers.PC];
    if (line < 0 || static_cast<size_t>(line) == followedLine)
        return;
    scrollToAddress(registers.PC);
}

void Debugger::stepIn() {
}

void Debugger::stepOver(DMGCpuRegisters& registers) {
    if (gameIsRunning || !funcStepInstruction)
        return;
    funcStepInstruction();
    scrollToAddress(registers.PC);
}

void Debugger::stepBack() {
}

void Debugger::stepReturn() {
}

void Debugger::advanceFrame() {
}

void Debugger::renderAssembly(DMGCpuRegisters& registers, float height) {
    disassemblySource(registers);
    followPC(registers);

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
    if (ImGui::Button(ICON_FA_ARROW_TURN_DOWN))
        stepIn();
    ImGui::SetItemTooltip("Step In");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_DOWN))
        stepOver(registers);
    ImGui::SetItemTooltip("Step Over");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_UP))
        stepBack();
    ImGui::SetItemTooltip("Step Back");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_UP))
        stepReturn();
    ImGui::SetItemTooltip("Step Return");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_UP_RIGHT_FROM_SQUARE))
        advanceFrame();
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
    const size_t markerLine = (gameIsRunning && followedLine != SIZE_MAX) ? followedLine : editorAssembly.GetCurrentCursorPosition().line;
    editorAssembly.AddMarker(markerLine, IM_COL32(55, 55, 60, 255), IM_COL32(55, 55, 60, 255), "", "");

    for (const auto& [addr, bp] : breakpoints) {
        const int32_t bpLine = (addr < addressToLine.size()) ? addressToLine[addr] : -1;
        if (bpLine >= 0)
            editorAssembly.AddMarker(static_cast<size_t>(bpLine), breakpointsDisabled ? IM_COL32(255, 0, 0, 100) : IM_COL32(255, 0, 0, 255), 0, "", "Breakpoint");
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
    ImGui::SameLine(54);
    ImGui::Text("Address");
    ImGui::SameLine(128);
    ImGui::Text("Bytes");
    ImGui::SameLine(216);
    ImGui::Text("Code");
    ImGui::PopStyleColor();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
    editorAssembly.Render("Assembly");
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void Debugger::renderRestBreakpoints() {
    float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_HighlightHoveredColumn;
    std::optional<int32_t> breakpointToRemove;

    if (ImGui::BeginTable("tableBreakpoints", 5, table_flags)) {
        ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Disable", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Hit", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_NoResize);
        ImGui::TableHeadersRow();

        for (auto& [addr, bp] : breakpoints) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            char lineLabel[16];
            snprintf(lineLabel, sizeof(lineLabel), "%d", bp.line);
            if (ImGui::Selectable(lineLabel, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, ImGui::GetFrameHeight())))
                scrollToAddress(static_cast<uint16_t>(bp.address));

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::Selectable("Remove breakpoint"))
                    breakpointToRemove = addr;
                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            {
                const float checkboxWidth = ImGui::GetFrameHeight();
                const float cellWidth = ImGui::GetContentRegionAvail().x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - checkboxWidth) * 0.5f);
                ImGui::Checkbox("##bpEnabled", &bp.enabled);
            }

            ImGui::TableNextColumn();
            {
                const float checkboxWidth = ImGui::GetFrameHeight();
                const float cellWidth = ImGui::GetContentRegionAvail().x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - checkboxWidth) * 0.5f);
                ImGui::BeginDisabled();
                ImGui::Checkbox("##bpHit", &bp.isHit);
                ImGui::EndDisabled();
            }

            ImGui::TableNextColumn();
            ImGui::Text("$%04X", bp.address);

            ImGui::TableNextColumn();
            ImGui::Text("%s", bp.description.c_str());
        }

        ImGui::EndTable();
    }

    if (breakpointToRemove.has_value())
        breakpoints.erase(*breakpointToRemove);
}
