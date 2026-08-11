#ifndef VRITA_ASSEMBLY_DMG_INCLUDES
#define VRITA_ASSEMBLY_DMG_INCLUDES

#include <cctype>
#include <string>
#include <unordered_set>

#include "ImGuiColorTextEdit/TextEditor.h"

// advance over an LR35902 identifier
// allows: foo, .local, @local, Function_123, hl+, hl-
inline TextEditor::Iterator GetIdentifier(TextEditor::Iterator start, TextEditor::Iterator end) {
    auto it = start;
    if (it == end)
        return start;
    ImWchar c = *it;
    if (!(std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '@'))
        return start;
    ++it;
    while (it != end) {
        c = *it;
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '@'))
            break;
        ++it;
    }
    return it;
}

// number tokenizer
// supports : $1234 - Game Boy / RGBDS hex, %10101010 - RGBDS binary, 1234 - decimal, 0x1234 - C-style hex, 0b1010 - C-style binary
inline TextEditor::Iterator GetNumber(TextEditor::Iterator start, TextEditor::Iterator end) {
    if (start == end)
        return start;
    
    const ImWchar c = *start;

    // $FFFF
    if (c == '$') {
        auto it = start;
        ++it;
        auto digitsStart = it;
        while (it != end && std::isxdigit(static_cast<unsigned char>(*it))) {
            ++it;
        }
        if (it != digitsStart)
            return it;
        return start;
    }

    // %10101010
    if (c == '%') {
        auto it = start;
        ++it;
        auto digitsStart = it;
        while (it != end && (*it == '0' || *it == '1')) {
            ++it;
        }
        if (it != digitsStart)
            return it;
        return start;
    }

    // 0xFFFF
    if (c == '0') {
        auto it = start;
        ++it;
        if (it != end && (*it == 'x' || *it == 'X')) {
            ++it;
            auto digitsStart = it;
            while (it != end && std::isxdigit(static_cast<unsigned char>(*it))) {
                ++it;
            }
            if (it != digitsStart)
                return it;
            return start;
        }

        // 0b101010
        it = start;
        ++it;

        if (it != end && (*it == 'b' || *it == 'B')) {
            ++it;
            auto digitsStart = it;
            while (it != end && (*it == '0' || *it == '1')) {
                ++it;
            }
            if (it != digitsStart)
                return it;
            return start;
        }
    }

    // decimal
    if (std::isdigit(static_cast<unsigned char>(c))) {
        auto it = start;
        ++it;
        while (it != end && std::isdigit(static_cast<unsigned char>(*it))) {
            ++it;
        }
        return it;
    }

    return start;
}

// punctuation
inline bool IsPunctuation(ImWchar c) {
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
        case '=':
        case '<':
        case '>':
        case '!':
        case '&':
        case '|':
        case '^':
        case '~':
            return true;
        default:
            return false;
    }
}

// LR35902 instructions
inline const std::unordered_set<std::string>& Instructions() {
    static const std::unordered_set<std::string> instructions = {
        // 8-bit / 16-bit loads
        "ld",
        "ldi",
        "ldd",

        // Increment/decrement
        "inc",
        "dec",

        // Arithmetic
        "add",
        "adc",
        "sub",
        "sbc",

        // Logical
        "and",
        "or",
        "xor",
        "cp",

        // Rotate/shift
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

        // Bit operations
        "bit",
        "set",
        "res",

        // Jumps
        "jp",
        "jr",

        // Calls
        "call",

        // Returns
        "ret",
        "reti",

        // Restart
        "rst",

        // Stack
        "push",
        "pop",

        // CPU control
        "nop",
        "halt",
        "stop",
        "di",
        "ei",

        // Misc
        "daa",
        "cpl",
        "ccf",
        "scf"
    };
    return instructions;
}

// CPU registers and condition codes
inline const std::unordered_set<std::string>& Registers() {
    static const std::unordered_set<std::string> retgisters = {
        // 8-bit
        "a",
        "f",

        "b",
        "c",
        "d",
        "e",
        "h",
        "l",

        // 16-bit
        "af",
        "bc",
        "de",
        "hl",
        "sp",

        // pseudo/debugger
        "pc",

        // Conditions
        "nz",
        "z",
        "nc",

        // Carry flag, "c" is also the C register.
        "c"
    };
    return retgisters;
}

// RGBDS / Game Boy assembler directives
inline const std::unordered_set<std::string>& Directives() {
    static const std::unordered_set<std::string> directives = {
        "section",
        "org",

        "db",
        "dw",
        "dd",
        "dl",
        "dq",

        "ds",

        "equ",
        "def",
        "set",

        "include",
        "incbin",

        "macro",
        "endm",

        "if",
        "else",
        "elif",
        "endc",

        "rept",
        "endr",

        "union",
        "nextu",
        "endu",

        "purge",

        "export",
        "import",

        "assert",
        "warn",
        "fail",

        "print"
    };
    return directives;
}

// Game Boy hardware registers / common symbols, "knownIdentifier" coloring
inline const std::unordered_set<std::string>& HardwareRegisters() {
    static const std::unordered_set<std::string> registers = {
        // LCD
        "lcdc",
        "stat",
        "scy",
        "scx",
        "ly",
        "lyc",
        "dma",
        "bgp",
        "obp0",
        "obp1",
        "wy",
        "wx",

        // Joypad
        "p1",

        // Timer
        "div",
        "tima",
        "tma",
        "tac",

        // Interrupts
        "if",
        "ie",

        // Serial
        "sb",
        "sc",

        // Sound
        "nr10",
        "nr11",
        "nr12",
        "nr13",
        "nr14",
        "nr21",
        "nr22",
        "nr23",
        "nr24",
        "nr30",
        "nr31",
        "nr32",
        "nr33",
        "nr34",
        "nr41",
        "nr42",
        "nr43",
        "nr44",
        "nr50",
        "nr51",
        "nr52",

        // Wave RAM
        "wave_pattern_ram",

        // VRAM / WRAM / HRAM symbolic names
        "vram",
        "wram",
        "hram",

        // Common RGBDS symbols
        "r_dmg_compatibility"
    };
    return registers;
}

// tokenizer
inline TextEditor::Iterator CustomTokenizer(TextEditor::Iterator start, TextEditor::Iterator end, TextEditor::Color& color) {
    if (start == end)
        return start;

    const ImWchar c = *start;
    
    // $FFFF
    if (c == '$') {
        auto it = start;
        ++it;
        while (it != end && std::isxdigit(static_cast<unsigned char>(*it))) {
            ++it;
        }
        auto check = start;
        ++check;
        if (it != check) {
            color = TextEditor::Color::number;
            return it;
        }
    }

    // %10101010
    if (c == '%') {
        auto it = start;
        ++it;
        while (it != end && (*it == '0' || *it == '1')) {
            ++it;
        }
        auto check = start;
        ++check;
        if (it != check) {
            color = TextEditor::Color::number;
            return it;
        }
    }

    // 0xFFFF
    if (c == '0') {
        auto it = start;
        ++it;
        if (it != end &&
            (*it == 'x' || *it == 'X')) {
            ++it;
            auto digitsStart = it;
            while (it != end && std::isxdigit(static_cast<unsigned char>(*it))) {
                ++it;
            }
            if (it != digitsStart) {
                color = TextEditor::Color::number;
                return it;
            }
        }

        // 0b101010
        it = start;
        ++it;

        if (it != end && (*it == 'b' || *it == 'B')) {
            ++it;
            auto digitsStart = it;
            while (it != end && (*it == '0' || *it == '1')) {
                ++it;
            }
            if (it != digitsStart) {
                color = TextEditor::Color::number;
                return it;
            }
        }
    }

    return start;
}

// the language definition
inline const TextEditor::Language* CreateDMGLanguage() {
    static TextEditor::Language language;
    static bool initialized = false;

    if (initialized)
        return &language;

    initialized = true;

    language.name = "Game Boy Assembly";
    language.caseSensitive = false;

    // --------------------------------------------------------
    // Comments
    //
    // RGBDS uses ';' for single-line comments.
    // --------------------------------------------------------
    language.singleLineComment = ";";

    // --------------------------------------------------------
    // Strings
    // --------------------------------------------------------
    language.hasSingleQuotedStrings = true;
    language.hasDoubleQuotedStrings = true;

    // --------------------------------------------------------
    // Keywords = CPU instructions
    // --------------------------------------------------------
    language.keywords = Instructions();

    // --------------------------------------------------------
    // Declarations = assembler directives
    // --------------------------------------------------------
    language.declarations = Directives();

    // --------------------------------------------------------
    // Identifiers = registers / conditions
    // --------------------------------------------------------
    language.identifiers = Registers();

    // --------------------------------------------------------
    // Punctuation
    // --------------------------------------------------------
    language.isPunctuation = IsPunctuation;

    // --------------------------------------------------------
    // Custom identifier tokenizer
    // --------------------------------------------------------
    language.getIdentifier = GetIdentifier;

    // --------------------------------------------------------
    // Custom number tokenizer
    // --------------------------------------------------------
    language.getNumber = GetNumber;

    // --------------------------------------------------------
    // Special tokens
    // --------------------------------------------------------
    language.customTokenizer = CustomTokenizer;

    return &language;
}

#endif
