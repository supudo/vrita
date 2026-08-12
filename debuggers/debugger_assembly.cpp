#include "debugger.hpp"
#include "assembly_dmg.inl"
#include "utilities/iconfonts/IconsFontAwesome7.h"

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

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    if (ImGui::Button(gameIsRunning ? ICON_FA_PAUSE : ICON_FA_PLAY, ImVec2(40, 32))) {
        gameIsRunning = !gameIsRunning;
        if (gameIsRunning) funcStartGame();
        else funcStopGame();
    }
    ImGui::SetItemTooltip(gameIsRunning ? "Pause" : "Run");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_DOWN, ImVec2(40, 32))) {
    }
    ImGui::SetItemTooltip("Step Over");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_DOWN, ImVec2(40, 32))) {
    }
    ImGui::SetItemTooltip("Step In");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_UP, ImVec2(40, 32))) {
    }
    ImGui::SetItemTooltip("Step Back");
    ImGui::SameLine();
    if (ImGui::Button(logCPUCalls ? "Log CPU Calls OFF" : "Log CPU Calls ON", ImVec2(120, 32))) {
        logCPUCalls = !logCPUCalls;
        funcLogCPUCalls(logCPUCalls);
    }
    ImGui::SetItemTooltip("Log CPU calls");
    ImGui::PopStyleColor(3);

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
