#ifndef VRITA_DEBUGGERS_ASSEMBLY_DEFINES_INCLUDES
#define VRITA_DEBUGGERS_ASSEMBLY_DEFINES_INCLUDES

#include <array>

enum InstructionFlags {
    None = 0,
    Branch = 1 << 0, // JP, JR
    Call = 1 << 1, // CALL, RST
    Return = 1 << 2, // RET, RETI
    Conditional = 1 << 3, // JP cc, JR cc, CALL cc, RET cc
    Terminates = 1 << 4, // HALT, STOP
};

enum class InstructionMnemonic : uint8_t {
    ADC,
    ADD,
    AND,
    BIT,
    CALL,
    CCF,
    CP,
    CPL,
    DAA,
    DEC,
    DI,
    EI,
    HALT,
    INC,
    JP,
    JR,
    LD,
    LDH,
    NOP,
    OR,
    POP,
    PUSH,
    RES,
    RET,
    RETI,
    RL,
    RLA,
    RLC,
    RLCA,
    RR,
    RRA,
    RRC,
    RRCA,
    RST,
    SBC,
    SCF,
    SET,
    SLA,
    SRA,
    SRL,
    STOP,
    SUB,
    SWAP,
    XOR,
    UNKNOWN
};

enum class OperandType : uint8_t {
    None,
    Register8,
    Register16,
    Immediate8,
    Immediate16,
    Address16,
    Relative8,
    Condition,
    Bit,
    SPRelative8,
};

struct Operand {
    OperandType type = OperandType::None;
    uint16_t value = 0;
};

struct DisassembledInstruction {
    uint16_t address;
    // if applicable, target of JP/JR/CALL/RST
    std::optional<uint16_t> target;

    uint8_t opcode;
    uint8_t length;
    uint8_t flags;

    InstructionMnemonic mnemonic;

    Operand operands[2];

    std::array<uint8_t, 3> bytes;
};

constexpr std::string_view InstructionMnemonicNames[] = {
    "ADC",
    "ADD",
    "AND",
    "BIT",
    "CALL",
    "CCF",
    "CP",
    "CPL",
    "DAA",
    "DEC",
    "DI",
    "EI",
    "HALT",
    "INC",
    "JP",
    "JR",
    "LD",
    "LDH",
    "NOP",
    "OR",
    "POP",
    "PUSH",
    "RES",
    "RET",
    "RETI",
    "RL",
    "RLA",
    "RLC",
    "RLCA",
    "RR",
    "RRA",
    "RRC",
    "RRCA",
    "RST",
    "SBC",
    "SCF",
    "SET",
    "SLA",
    "SRA",
    "SRL",
    "STOP",
    "SUB",
    "SWAP",
    "XOR",
    "???"
};

constexpr std::string_view registerNames8[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
constexpr std::string_view registerNames16[] = { "BC", "DE", "HL", "SP" };
constexpr std::string_view conditionNames[] = { "NZ", "Z", "NC", "C" };
static constexpr InstructionMnemonic rotateShiftMnemonics[] = {
    InstructionMnemonic::RLC,
    InstructionMnemonic::RRC,
    InstructionMnemonic::RL,
    InstructionMnemonic::RR,
    InstructionMnemonic::SLA,
    InstructionMnemonic::SRA,
    InstructionMnemonic::SWAP,
    InstructionMnemonic::SRL
};

std::string assemblySampleDMG = R"(; Game Boy boot code

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

)";

#endif