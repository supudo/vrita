#include "debugger.hpp"
#include "assembly_dmg.inl"
#include "utilities/iconfonts/IconsFontAwesome7.h"
#include "utilities/fonts.hpp"

void Debugger::initEditor() {
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);
}

void Debugger::renderAssembly(float height) {
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

    if (!editorSourceSet) {
        editorSourceSet = true;
        editorAssembly.SetText(R"(

; Game Boy boot code

SECTION "Start", ROM0[$0100]

Start:
    nop
    jp $0150

    db $CE
    db %10101010

Main:
    ld   sp,$FFFE
    xor  a
    ld   hl,$C000

Loop:
    ld   (hl+),a
    inc  a
    cp   $10
    jr   nz,Loop

    call $1234
    jp   Main

    halt

)");
    }

    editorAssembly.ClearMarkers();
    editorAssembly.AddMarker(editorAssembly.GetCurrentCursorPosition().line, IM_COL32(55, 55, 60, 255), IM_COL32(55, 55, 60, 255), "", "");

    editorAssembly.Render("Assembly");

    ImGui::EndChild();
}
