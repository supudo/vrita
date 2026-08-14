#ifndef VRITA_ASSEMBLY_DMG_INCLUDES
#define VRITA_ASSEMBLY_DMG_INCLUDES

#include <cctype>
#include <cstdint>
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

constexpr std::string_view getRegisterName8(uint8_t index) {
    return registerNames8[index & 7];
}

constexpr std::string_view getRegisterName16(uint8_t index) {
    return registerNames16[index & 3];
}

constexpr std::string_view getConditionName(uint8_t index) {
    return conditionNames[index & 3];
}

inline std::string formatHex8(uint8_t value) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "$%02X", value);
    return buffer;
}

inline std::string formatHex16(uint16_t value) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "$%04X", value);
    return buffer;
}

inline std::string formatHex16Brackets(uint16_t value) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "($%04X)", value);
    return buffer;
}

inline std::string instructionFormatOperand(const Operand& operand) {
    switch (operand.type) {
        case OperandType::None:
            return "";
        case OperandType::Register8:
            return std::string(getRegisterName8(static_cast<uint8_t>(operand.value)));
        case OperandType::Register16:
            return std::string(getRegisterName16(static_cast<uint8_t>(operand.value)));
        case OperandType::Immediate8:
            return formatHex8(static_cast<uint8_t>(operand.value));
        case OperandType::Immediate16:
            return formatHex16(operand.value);
        case OperandType::Address16:
            return formatHex16Brackets(operand.value);
        case OperandType::Relative8:
            return formatHex16(operand.value);
        case OperandType::Condition:
            return std::string(getConditionName(static_cast<uint8_t>(operand.value)));
        case OperandType::Bit:
            return std::to_string(operand.value);
        case OperandType::SPRelative8: {
            const int8_t offset = static_cast<int8_t>(operand.value);
            return offset >= 0 ? "SP+" + formatHex8(offset) : "SP-" + formatHex8(-offset);
        }
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
        result += formatHex8(instruction.bytes[i]);
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

    uint8_t dv1 = 0;
    uint8_t dv2 = 0;

    if (instruction.length >= 2)
        dv1 = read8(address + 1);
    if (instruction.length >= 3)
        dv2 = read8(address + 2);

    const uint8_t d8 = dv1;
    const uint16_t d16 = instruction.length >= 3 ? static_cast<uint16_t>(dv1) | (static_cast<uint16_t>(dv2) << 8) : 0;

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
        instruction.flags = InstructionFlags::Return;
    }
    else if (opcode >= 0x40 && opcode <= 0x7F) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>((opcode >> 3) & 7) };
        instruction.operands[1] = { OperandType::Register8, static_cast<uint16_t>(opcode & 7) };
    }
    else if (opcode == 0x06 || opcode == 0x0E || opcode == 0x16 || opcode == 0x1E || opcode == 0x26 || opcode == 0x2E || opcode == 0x36 || opcode == 0x3E) {
        const uint8_t dst = (opcode >> 3) & 7;
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(dst) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xC6) {
        instruction.mnemonic = InstructionMnemonic::ADD;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xCE) {
        instruction.mnemonic = InstructionMnemonic::ADC;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xD6) {
        instruction.mnemonic = InstructionMnemonic::SUB;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xDE) {
        instruction.mnemonic = InstructionMnemonic::SBC;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xE6) {
        instruction.mnemonic = InstructionMnemonic::AND;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xEE) {
        instruction.mnemonic = InstructionMnemonic::XOR;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xF6) {
        instruction.mnemonic = InstructionMnemonic::OR;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xFE) {
        instruction.mnemonic = InstructionMnemonic::CP;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xE0) {
        instruction.mnemonic = InstructionMnemonic::LDH;
        instruction.operands[0] = { OperandType::Address16, static_cast<uint16_t>(0xFF00 + d8) };
        instruction.operands[1] = { OperandType::Register8, static_cast<uint16_t>(7) };
    }
    else if (opcode == 0xF0) {
        instruction.mnemonic = InstructionMnemonic::LDH;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Address16, static_cast<uint16_t>(0xFF00 + d8) };
    }
    else if (opcode == 0x01) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(0) };
        instruction.operands[1] = { OperandType::Immediate16, static_cast<uint16_t>(d16) };
    }
    else if (opcode == 0x11) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(1) };
        instruction.operands[1] = { OperandType::Immediate16, static_cast<uint16_t>(d16) };
    }
    else if (opcode == 0x21) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(2) };
        instruction.operands[1] = { OperandType::Immediate16, static_cast<uint16_t>(d16) };
    }
    else if (opcode == 0x31) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(3) };
        instruction.operands[1] = { OperandType::Immediate16, static_cast<uint16_t>(d16) };
    }
    else if (opcode == 0x08) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Address16, static_cast<uint16_t>(d16) };
        instruction.operands[1] = { OperandType::Register16, static_cast<uint16_t>(3) };
    }
    else if (opcode == 0xEA) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Address16, static_cast<uint16_t>(d16) };
        instruction.operands[1] = { OperandType::Register8, static_cast<uint16_t>(7) };
    }
    else if (opcode == 0xFA) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(7) };
        instruction.operands[1] = { OperandType::Address16, static_cast<uint16_t>(d16) };
    }
    else if (opcode == 0x18) {
        instruction.mnemonic = InstructionMnemonic::JR;
        const uint16_t jumpTarget = static_cast<uint16_t>(address + 2 + static_cast<int8_t>(d8));
        instruction.operands[0] = { OperandType::Relative8, jumpTarget };
        instruction.target = jumpTarget;
        instruction.flags = InstructionFlags::Branch;
    }
    else if (opcode == 0x20 || opcode == 0x28 || opcode == 0x30 || opcode == 0x38) {
        instruction.mnemonic = InstructionMnemonic::JR;
        const uint16_t jumpTarget = static_cast<uint16_t>(address + 2 + static_cast<int8_t>(d8));
        const uint8_t condition = (opcode >> 3) & 3;
        instruction.operands[0] = { OperandType::Condition, condition };
        instruction.operands[1] = { OperandType::Relative8, jumpTarget };
        instruction.target = jumpTarget;
        instruction.flags = InstructionFlags::Branch | InstructionFlags::Conditional;
    }
    else if (opcode == 0xC3) {
        instruction.mnemonic = InstructionMnemonic::JP;
        instruction.operands[0] = { OperandType::Immediate16, d16 };
        instruction.target = d16;
        instruction.flags = InstructionFlags::Branch;
    }
    else if (opcode == 0xC2 || opcode == 0xCA || opcode == 0xD2 || opcode == 0xDA) {
        instruction.mnemonic = InstructionMnemonic::JP;
        const uint8_t condition = (opcode >> 3) & 3;
        instruction.operands[0] = { OperandType::Condition, condition };
        instruction.operands[1] = { OperandType::Immediate16, d16 };
        instruction.target = d16;
        instruction.flags = InstructionFlags::Branch | InstructionFlags::Conditional;
    }
    else if (opcode == 0xE9) {
        instruction.mnemonic = InstructionMnemonic::JP;
        instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(6) };
        instruction.flags = InstructionFlags::Branch;
    }
    else if (opcode == 0xCD) {
        instruction.mnemonic = InstructionMnemonic::CALL;
        instruction.operands[0] = { OperandType::Immediate16, d16 };
        instruction.target = d16;
        instruction.flags = InstructionFlags::Call;
    }
    else if (opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC) {
        instruction.mnemonic = InstructionMnemonic::CALL;
        const uint8_t condition = (opcode >> 3) & 3;
        instruction.operands[0] = { OperandType::Condition, condition };
        instruction.operands[1] = { OperandType::Immediate16, d16 };
        instruction.target = d16;
        instruction.flags = InstructionFlags::Call | InstructionFlags::Conditional;
    }
    else if (opcode == 0xC0 || opcode == 0xC8 || opcode == 0xD0 || opcode == 0xD8) {
        instruction.mnemonic = InstructionMnemonic::RET;
        const uint8_t condition = (opcode >> 3) & 3;
        instruction.operands[0] = { OperandType::Condition, condition };
        instruction.flags = InstructionFlags::Return | InstructionFlags::Conditional;
    }
    else if (opcode == 0xD9) {
        instruction.mnemonic = InstructionMnemonic::RETI;
        instruction.flags = InstructionFlags::Return;
    }
    else if (opcode == 0xC7 || opcode == 0xCF || opcode == 0xD7 || opcode == 0xDF || opcode == 0xE7 || opcode == 0xEF || opcode == 0xF7 || opcode == 0xFF) {
        instruction.mnemonic = InstructionMnemonic::RST;
        const uint16_t vector = opcode & 0x38;
        instruction.target = vector;
        instruction.operands[0] = { OperandType::Immediate8, vector };
        instruction.flags = InstructionFlags::Call;
    }
    else if (opcode == 0xE8) {
        instruction.mnemonic = InstructionMnemonic::ADD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(3) };
        instruction.operands[1] = { OperandType::Immediate8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xF8) {
        instruction.mnemonic = InstructionMnemonic::LD;
        instruction.operands[0] = { OperandType::Register16, static_cast<uint16_t>(2) };
        instruction.operands[1] = { OperandType::SPRelative8, static_cast<uint16_t>(d8) };
    }
    else if (opcode == 0xCB) {
        const uint8_t group = (d8 >> 6) & 3;
        const uint8_t field = (d8 >> 3) & 7;
        const uint8_t reg = d8 & 7;

        if (group == 0) {
            instruction.mnemonic = rotateShiftMnemonics[field];
            instruction.operands[0] = { OperandType::Register8, static_cast<uint16_t>(reg) };
        }
        else {
            instruction.mnemonic = group == 1 ? InstructionMnemonic::BIT : group == 2 ? InstructionMnemonic::RES : InstructionMnemonic::SET;
            instruction.operands[0] = { OperandType::Bit, static_cast<uint16_t>(field) };
            instruction.operands[1] = { OperandType::Register8, static_cast<uint16_t>(reg) };
        }
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
        "ldh",
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

    // Comments
    language.singleLineComment = ";";

    // Strings
    language.hasSingleQuotedStrings = true;
    language.hasDoubleQuotedStrings = true;

    // Keywords = CPU instructions
    language.keywords = Instructions();

    // Declarations = assembler directives
    language.declarations = Directives();

    // Identifiers = registers / conditions
    language.identifiers = Registers();

    // Punctuation
    language.isPunctuation = IsPunctuation;

    // Custom identifier tokenizer
    language.getIdentifier = GetIdentifier;

    // Custom number tokenizer
    language.getNumber = GetNumber;

    // Special tokens
    language.customTokenizer = CustomTokenizer;

    return &language;
}

#endif
