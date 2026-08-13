#ifndef VRITA_ASSEMBLY_DMG_INCLUDES
#define VRITA_ASSEMBLY_DMG_INCLUDES

#include <cctype>
#include <cstdint>
#include <format>
#include <string>
#include <unordered_set>

#include "ImGuiColorTextEdit/TextEditor.h"
#include "debugger_assembly_defines.hpp"

// disassembly

inline uint8_t instructionLengths(uint8_t opcode) {
    static constexpr uint8_t lengths[256] = {
        // 0x00-0x0F
        1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
        // 0x10-0x1F
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        // 0x20-0x2F
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        // 0x30-0x3F
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        // 0x40-0x4F (LD r,r' block)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0x50-0x5F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0x60-0x6F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0x70-0x7F (0x76 = HALT)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0x80-0x8F (ADD/ADC A,r)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0x90-0x9F (SUB/SBC A,r)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0xA0-0xAF (AND/XOR r)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0xB0-0xBF (OR/CP r)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        // 0xC0-0xCF (CB prefix at 0xCB)
        1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
        // 0xD0-0xDF
        1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1,
        // 0xE0-0xEF
        2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
        // 0xF0-0xFF
        2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
    };
    return lengths[opcode];
}

constexpr std::string_view instructionToString(InstructionMnemonic m) {
    return InstructionMnemonicNames[static_cast<uint8_t>(m)];
}

inline std::string getRegisterName8(uint8_t index) {
    return std::string(registerNames8[index & 7]);
}

inline std::string getRegisterName16(uint8_t index) {
    return std::string(registerNames16[index & 3]);
}

constexpr std::string getConditionName(uint8_t index) {
    return std::string(conditionNames[index & 3]);
}

inline std::string instructionFormatOperand(const Operand& operand) {
    switch (operand.type) {
        case OperandType::None:
            return "";
        case OperandType::Register8:
            return getRegisterName8(operand.value);
        case OperandType::Register16:
            return getRegisterName16(operand.value);
        case OperandType::Immediate8:
            return std::format("${:02X}", operand.value);
        case OperandType::Immediate16:
            return std::format("${:04X}", operand.value);
        case OperandType::Address16:
            return std::format("(${:04X})", operand.value);
        case OperandType::Relative8:
            return std::format("${:04X}", operand.value);
        case OperandType::Condition:
            return getConditionName(operand.value);
        case OperandType::Bit:
            return std::to_string(operand.value);
    }
    return "???";
}

inline std::string instructionFormat(const DisassembledInstruction& instruction) {
    std::string result = std::string(instructionToString(instruction.mnemonic));
    bool first = true;
    for (const auto& operand : instruction.operands) {
        if (operand.type == OperandType::None)
            continue;
        result += first ? " " : ", ";
        result += instructionFormatOperand(operand);
        first = false;
    }
    return result;
}

std::string formatBytes(const DisassembledInstruction& instruction) {
    std::string result;
    for (uint8_t i = 0; i < instruction.length; ++i) {
        if (i > 0)
            result += ' ';
        result += std::format("{:02X}", instruction.bytes[i]);
    }
    return result;
}

inline DisassembledInstruction disassembleInstruction(uint16_t address, uint8_t opcode, const std::function<uint8_t(uint32_t)>& read8) {
    DisassembledInstruction instruction;
    instruction.address = address;
    instruction.bytes[0] = opcode;
    instruction.opcode = opcode;
    instruction.length = instructionLengths(opcode);
    instruction.mnemonic = InstructionMnemonic::UNKNOWN;
    instruction.flags = InstructionFlags::None;

    const uint8_t dv1 = read8(address + 1);
    const uint8_t dv2 = read8(address + 2);

    const uint8_t d8 = instruction.length >= 2 ? dv1 : 0;
    const uint16_t d16 = instruction.length >= 3 ? static_cast<uint16_t>(d8) | (static_cast<uint16_t>(dv2) << 8) : 0;

    if (instruction.length >= 2)
        instruction.bytes[1] = dv1;

    if (instruction.length >= 3)
        instruction.bytes[2] = dv2;

    if (opcode == 0x00)
        instruction.mnemonic = InstructionMnemonic::NOP;
    else if (opcode == 0x76) {
        instruction.mnemonic = InstructionMnemonic::HALT;
        instruction.flags = InstructionFlags::Terminates;
    }
    else if (opcode == 0xC9) {
        instruction.mnemonic = InstructionMnemonic::RET;
        instruction.flags = InstructionFlags::Terminates;
    }
    else if (opcode >= 0x40 && opcode <= 0x7F) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>((opcode >> 3) & 7) };
        instruction.operands[1] = { OperandType::Register8, static_cast<uint16_t>(opcode & 7) };
    }
    else
        instruction.mnemonic = InstructionMnemonic::UNKNOWN;

    return instruction;
}

// editor

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
