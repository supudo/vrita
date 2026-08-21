#ifndef VRITA_DEBUGGERS_ASSEMBLY_DEFINES_INCLUDES
#define VRITA_DEBUGGERS_ASSEMBLY_DEFINES_INCLUDES

#include <array>

struct CartridgeHeaderField {
    uint16_t address;
    uint8_t length;
    const char* comment;
};

static constexpr CartridgeHeaderField cartridgeHeaderFields[] = {
    { 0x0134, 11, "Title" },
    { 0x013F, 4, "Manufacturer Code" },
    { 0x0143, 1, "CGB Flag" },
    { 0x0144, 2, "New Licensee Code" },
    { 0x0146, 1, "SGB Flag" },
    { 0x0147, 1, "Cartridge Type" },
    { 0x0148, 1, "ROM Size" },
    { 0x0149, 1, "RAM Size" },
    { 0x014A, 1, "Destination Code" },
    { 0x014B, 1, "Old Licensee Code" },
    { 0x014C, 1, "Mask ROM Version" },
    { 0x014D, 1, "Header Checksum" },
    { 0x014E, 2, "Global Checksum" },
};

enum class LabelKind : uint8_t {
    Branch,
    Function,
    EntryPoint
};

struct WorkItem {
    uint16_t bank;
    uint16_t address;
};

static inline uint32_t keyOf(uint16_t bank, uint16_t addr) {
    return (addr < 0x4000) ? static_cast<uint32_t>(addr) : (static_cast<uint32_t>(bank) << 16) | addr;
}

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
    Register16Stack,
    IndirectPointer
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
    ""
};

constexpr std::string_view registerNames8[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
constexpr std::string_view registerNames16[] = { "BC", "DE", "HL", "SP" };
constexpr std::string_view registerNames16Stack[] = { "BC", "DE", "HL", "AF" };
constexpr std::string_view indirectPointerNames[] = { "(BC)", "(DE)", "(HL+)", "(HL-)" };
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

#endif