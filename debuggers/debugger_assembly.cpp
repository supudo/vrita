#include "debugger.hpp"
#include "assembly_dmg.inl"

void Debugger::initEditor() {
    //editorAssembly.SetLanguage(initializeLanguage());
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);

    editorAssembly.SetLineDecorator(
        -2.0f,
        [this] (TextEditor::Decorator& decorator) {
            const int line = decorator.line;
            if (line < 0)
                return;
            if (line == editorAssembly.GetCurrentCursorPosition().line)
                ImGui::GetWindowDrawList()->AddText(ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y), IM_COL32(255, 210, 60, 255), ">");
        }
    );
}

const TextEditor::Language* Debugger::initializeLanguage() {
    if (!editorInitialized) {
        editorInitialized = true;

        editorLanguage.name = "Game Boy Assembly";
        editorLanguage.caseSensitive = false;

        // Game Boy assembly comments
        editorLanguage.singleLineComment = ";";

        // Strings, if your disassembly/source supports them
        editorLanguage.hasSingleQuotedStrings = true;
        editorLanguage.hasDoubleQuotedStrings = true;

        // LR35902 instructions
        editorLanguage.keywords = {
            "nop",

            "ld",
            "ldi",
            "ldd",

            "inc",
            "dec",

            "add",
            "adc",
            "sub",
            "sbc",

            "and",
            "or",
            "xor",
            "cp",

            "daa",
            "cpl",
            "ccf",
            "scf",

            "rlca",
            "rla",
            "rrca",
            "rra",

            "rlc",
            "rl",
            "rrc",
            "rr",

            "sla",
            "sra",
            "srl",

            "swap",

            "bit",
            "set",
            "res",

            "jp",
            "jr",
            "call",
            "ret",
            "reti",
            "rst",

            "push",
            "pop",

            "halt",
            "stop",
            "di",
            "ei"
        };

        // CPU registers / special operands
        editorLanguage.identifiers = {
            "a",
            "f",

            "b",
            "c",

            "d",
            "e",

            "h",
            "l",

            "af",
            "bc",
            "de",
            "hl",

            "sp",
            "pc",

            "hl+",
            "hl-"
        };

        // Optional: directives / assembler declarations
        editorLanguage.declarations = {
            "db",
            "dw",
            "ds",
            "equ",
            "org",
            "section",
            "include",
            "incbin",
            "macro",
            "endm"
        };

        // Punctuation used by GB assembly.
        editorLanguage.isPunctuation = [] (ImWchar c) {
            switch (c) {
                case ',':
                case '(':
                case ')':
                case '[':
                case ']':
                case ':':
                case '+':
                case '-':
                case '*':
                case '/':
                    return true;

                default:
                    return false;
            }
        };
    }

    return &editorLanguage;
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
    editorAssembly.Render("Assembly");
}
