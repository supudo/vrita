#include "debugger.hpp"
#include "assembly_dmg.inl"

void Debugger::initEditor() {
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);
}

void Debugger::renderAssembly() {
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
}
