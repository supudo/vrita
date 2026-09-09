#include "debugger.hpp"

#include <memory>
#include <optional>

#include "utilities/iconfonts/IconsFontAwesome7.h"
#include "utilities/fonts.hpp"
#include "emulators/dmg/cpu_registers.hpp"

constexpr size_t CONST_TriangleMarkerGlyphs = 2;
constexpr size_t CONST_AddressColumnsGlyphs = 10;
constexpr size_t CONST_ByteCodeColumnsGlyphs = 10;

void Debugger::initEditor() {
    editorLanguage = CreateDMGLanguage();
    editorAssembly.SetLanguage(editorLanguage);
    editorAssembly.SetReadOnlyEnabled(true);

    editorOptionShowLineNumbers = settings.GetInt("Debuggers - Editor", "editor_option_show_line_numbers", true);
    editorAssembly.SetShowLineNumbersEnabled(editorOptionShowLineNumbers);
    editorOptionShowAddress = settings.GetInt("Debuggers - Editor", "editor_option_show_address", true);
    editorOptionShowByteCode = settings.GetInt("Debuggers - Editor", "editor_option_show_byte_code", true);
    editorOptionSyntaxHighlight = settings.GetInt("Debuggers - Editor", "editor_option_syntax_highlight", true);
    editorOptionShowMiniMap = settings.GetInt("Debuggers - Editor", "editor_option_show_minimap", false);

    editorAssembly.SetTextContextMenuCallback([this] (TextEditor::PopupData& data) {
        const int32_t line = static_cast<int32_t>(data.pos.line);
        const uint32_t addr = (line >= 0 && static_cast<size_t>(line) < lineToAddress.size()) ? lineToAddress[line] : 0;
        showContextMenu(line, addr);
    });

    editorAssembly.SetLineNumberContextMenuCallback([this] (TextEditor::PopupData& data) {
        const int32_t line = static_cast<int32_t>(data.pos.line);
        const uint32_t addr = (line >= 0 && static_cast<size_t>(line) < lineToAddress.size()) ? lineToAddress[line] : 0;
        showContextMenu(line, addr);
    });

    editorAssembly.SetTextHoverCallback([&] (TextEditor::PopupData data) {
        const int32_t line = static_cast<int32_t>(data.pos.line);
        const uint32_t addr = (line >= 0 && static_cast<size_t>(line) < lineToAddress.size()) ? lineToAddress[line] : 0;
        std::string lineContent = editorAssembly.GetLineText(data.pos.line);
        lineContent.erase(std::remove_if(lineContent.begin(), lineContent.end(), ::isspace), lineContent.end());
        if (!startsWithAsmPrefix(lineContent)) {
            ImGui::CloseCurrentPopup();
            return;
        }
        ImGui::TextDisabled("Line #%i, address $%02X", line, addr);
        ImGui::Separator();
        ImGui::Text("%s", lineContent.c_str());
    });

    updateLineDecorator();
}

void Debugger::showContextMenu(const int32_t line, const uint32_t addr) {
    bool hasBreakpoint = breakpoints.contains(addr);
    if (ImGui::MenuItem("Toggle Breakpoint")) {
        if (hasBreakpoint)
            breakpoints.erase(addr);
        else
            breakpoints[addr] = DebuggerBreakpoint{ addr, line, true, false, editorAssembly.GetLineText(static_cast<size_t>(line)) };
    }
    ImGui::Dummy(ImVec2(1, 10));
    bool breakpointEnabled = false;
    if (hasBreakpoint)
        breakpointEnabled = breakpoints[addr].enabled;
    if (ImGui::MenuItem(breakpointEnabled ? "Disable Breakpoint" : "Enable Breakpoint", nullptr, nullptr, hasBreakpoint))
        breakpoints[addr].enabled = !breakpoints[addr].enabled;
    if (ImGui::MenuItem("Breakpoint properties...")) {
        // TODO
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Run to line")) {
        // TODO
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Show in memory")) {
        // TODO
    }
}

void Debugger::updateLineDecorator() {
    const size_t widthGlyphs = CONST_TriangleMarkerGlyphs + (editorOptionShowAddress ? CONST_AddressColumnsGlyphs : 0) + (editorOptionShowByteCode  ? CONST_ByteCodeColumnsGlyphs : 0);
    editorAssembly.SetLineDecorator(widthGlyphs, [this] (TextEditor::Decorator& decorator) {
        const bool hasData = decorator.line < lineToBytes.size()&&!lineToBytes[decorator.line].empty();
        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        float x = p0.x;

        if (!gameIsRunning && followedLine != SIZE_MAX && decorator.line == followedLine) {
            const float pad = decorator.height * 0.1f;
            const ImVec2 p1(x + pad, p0.y + pad);
            const ImVec2 p2(x + pad, p0.y + decorator.height - pad);
            const ImVec2 p3(x + CONST_TriangleMarkerGlyphs * decorator.glyphSize.x - pad, p0.y + decorator.height * 0.5f);
            ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, IM_COL32(220, 30, 30, 255));
        }
        x += CONST_TriangleMarkerGlyphs * decorator.glyphSize.x;

        if (editorOptionShowAddress) {
            if (hasData) {
                char addr[8];
                snprintf(addr, sizeof(addr), "$%04X", decorator.line < lineToAddress.size() ? lineToAddress[decorator.line] : 0);
                ImGui::GetWindowDrawList()->AddText(ImVec2(x, p0.y), ImGui::GetColorU32(ImGuiCol_Text), addr);
            }
            x += CONST_AddressColumnsGlyphs * decorator.glyphSize.x;
        }
        if (editorOptionShowByteCode) {
            if (hasData)
                ImGui::GetWindowDrawList()->AddText(ImVec2(x, p0.y), ImGui::GetColorU32(ImGuiCol_Text), lineToBytes[decorator.line].c_str());
            x += CONST_ByteCodeColumnsGlyphs * decorator.glyphSize.x;
        }
    });
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
        const int32_t bpLine = resolveAddressLine(static_cast<uint16_t>(addr));
        if (bpLine >= 0)
            editorAssembly.AddMarker(static_cast<size_t>(bpLine), breakpointsDisabled ? IM_COL32(255, 0, 0, 100) : IM_COL32(255, 0, 0, 255), 0, "", "Breakpoint");
    }

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("tabsEditor", tab_bar_flags)) {
        if (ImGui::BeginTabItem("Editor")) {
            if (editorSourceSet) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
                ImGui::Dummy(ImVec2(0, 0));

                const float glyphWidth = editorAssembly.GetGlyphWidth();
                const size_t lineDigits = std::to_string(editorAssembly.GetLineCount()).size();
                const size_t decoratorGlyphs = CONST_TriangleMarkerGlyphs + (editorOptionShowAddress ? CONST_AddressColumnsGlyphs : 0) + (editorOptionShowByteCode ? CONST_ByteCodeColumnsGlyphs : 0);

                const float lineNumberLeftOffset = editorOptionShowLineNumbers ? editorAssembly.GetLineNumberLeftMargin() * glyphWidth : 0.0f;
                const float lineNumberRightOffset = editorOptionShowLineNumbers ? lineNumberLeftOffset + static_cast<float>(lineDigits) * glyphWidth : 0.0f;
                const float decorationOffset = lineNumberRightOffset + editorAssembly.GetDecorationLeftMargin() * glyphWidth;
                const float textLeftOffset = decorationOffset + (static_cast<float>(decoratorGlyphs) + editorAssembly.GetTextLeftMargin()) * glyphWidth;

                float cursorX = lineNumberLeftOffset;

                if (editorOptionShowLineNumbers) {
                    ImGui::SameLine(cursorX + 24.0f);
                    ImGui::Text("#");
                }
                cursorX = decorationOffset + CONST_TriangleMarkerGlyphs * glyphWidth;
                if (editorOptionShowAddress) {
                    ImGui::SameLine(cursorX);
                    ImGui::Text("Address");
                    cursorX += CONST_AddressColumnsGlyphs * glyphWidth;
                }
                if (editorOptionShowByteCode) {
                    ImGui::SameLine(cursorX);
                    ImGui::Text("Bytes");
                    cursorX += CONST_ByteCodeColumnsGlyphs * glyphWidth;
                }
                ImGui::SameLine(textLeftOffset);
                ImGui::Text("Code");
                ImGui::PopStyleColor();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));

                editorAssembly.Render("Assembly");

                if (editorSourceSet && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                    && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyDown(ImGuiMod_Shift)
                    && editorAssembly.IsMousePosOverGlyph(ImGui::GetMousePos())) {
                    const std::string word = editorAssembly.GetWordAtMousePos(ImGui::GetMousePos());
                    uint16_t bank = 0, addr = 0;
                    if (parseLabelIdentifier(word, bank, addr))
                        scrollToBankAddress(bank, addr);
                }

                ImGui::PopStyleVar();
            }
            else if (ImGui::Button("Disassemble ROM"))
                disassemblyRequested = true;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Options")) {
            if (ImGui::Checkbox("Show Line Numbers", &editorOptionShowLineNumbers)) {
                settings.Set("Debuggers - Editor", "editor_option_show_line_numbers", editorOptionShowLineNumbers);
                editorAssembly.SetShowLineNumbersEnabled(editorOptionShowLineNumbers);
            }

            if (ImGui::Checkbox("Show Address", &editorOptionShowAddress)) {
                settings.Set("Debuggers - Editor", "editor_option_show_address", editorOptionShowAddress);
                updateLineDecorator();
            }

            if (ImGui::Checkbox("Show Byte Code", &editorOptionShowByteCode)) {
                settings.Set("Debuggers - Editor", "editor_option_show_byte_code", editorOptionShowByteCode);
                updateLineDecorator();
            }

            if (ImGui::Checkbox("Syntax Highlight", &editorOptionSyntaxHighlight)) {
                settings.Set("Debuggers - Editor", "editor_option_syntax_highlight", editorOptionSyntaxHighlight);
                editorAssembly.SetLanguage(editorOptionSyntaxHighlight ? editorLanguage : nullptr);
            }

            if (ImGui::Checkbox("Mini map", &editorOptionShowMiniMap)) {
                settings.Set("Debuggers - Editor", "editor_option_show_minimap", editorOptionShowMiniMap);
                editorAssembly.SetShowMiniMapEnabled(editorOptionShowMiniMap);
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

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

bool Debugger::parseLabelIdentifier(const std::string& word, uint16_t& bank, uint16_t& address) {
    for (auto prefix : ASM_prefixes) {
        if (word.compare(0, prefix.size(), prefix) != 0)
            continue;
        const std::string rest = word.substr(prefix.size());
        unsigned bb = 0, addr = 0;
        if (sscanf(rest.c_str(), "%2x_%4x", &bb, &addr) == 2) {
            bank = static_cast<uint16_t>(bb);
            address = static_cast<uint16_t>(addr);
            return true;
        }
        if (sscanf(rest.c_str(), "%4x", &addr) == 1) {
            bank = 0;
            address = static_cast<uint16_t>(addr);
            return true;
        }
        return false;
    }
    return false;
}