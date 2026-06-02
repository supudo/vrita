#include "../../include/dmg/cpu.hpp"

void DMG_CPU::initialize(Logger& logger, std::shared_ptr<DMG_CARTRIDGE> cartridge) {
    this->logger = &logger;

    // initial register values
    CpuRegisters.A = 0x01;
    CpuRegisters.F = 0xB0;

    CpuRegisters.B = 0x00;
    CpuRegisters.C = 0x13;

    CpuRegisters.D = 0x00;
    CpuRegisters.E = 0xD8;

    CpuRegisters.H = 0x01;
    CpuRegisters.L = 0x4D;

    CpuRegisters.SP = 0xFFFE;
    CpuRegisters.PC = 0x0100;

    // instructions =================

    // Miscelanious
    InstructionsTable[0x00] = { "NOP", &DMG_CPU::NOP, 1, 4 };
    InstructionsTable[0x76] = { "HALT", &DMG_CPU::HALT, 1, 4 };
    InstructionsTable[0x10] = { "STOP", &DMG_CPU::STOP, 1, 4 };
    InstructionsTable[0xF3] = { "DI", &DMG_CPU::DI, 1, 4 };
    InstructionsTable[0xFB] = { "EI", &DMG_CPU::EI, 1, 4 };

    // 8-bit arithmetic and logical instructions
    InstructionsTable[0x80] = { "ADD A,B", &DMG_CPU::add_a_a, 1, 4 };

    // Rotate, shift, and bit operation instructions
    InstructionsTable[0xFF] = { "FF", &DMG_CPU::set_7_a, 2, 2 };
}

void DMG_CPU::stepCPU(bool ROMFileLoaded, uint8_t *memory) {
    if (!ROMFileLoaded) return;

    uint8_t opcode = memory[CpuRegisters.PC++];
    const DMG_CPU::Instruction& instruction = InstructionsTable[opcode];
    if (instruction.name != nullptr) {
        (this->*instruction.execute)();
        cycles += instruction.cycles;
    }
    else
        this->logger->log("[DMG_CPU] Unsupported opcode: %02X", opcode);
}

// BEGIN instruction functions
// Miscelanious instructions
void DMG_CPU::NOP() {
    this->logCall("NOP");
}

void DMG_CPU::HALT() {
    this->logCall("HALT");
    halted = true;
}

void DMG_CPU::STOP() {
    this->logCall("STOP");
    halted = true;
}

void DMG_CPU::DI(void) {
    this->logCall("DI");
}

void DMG_CPU::EI(void) {
    this->logCall("EI");
}

// 8-bit load instructions

// 16-it load instructions

// 8-bit arithmetic and logical instructions
void DMG_CPU::add_a_b(void) {
    add(&CpuRegisters.A, CpuRegisters.B);
}

void DMG_CPU::add_a_c(void) {
    add(&CpuRegisters.A, CpuRegisters.C);
}

void DMG_CPU::add_a_d(void) {
    add(&CpuRegisters.A, CpuRegisters.D);
}

void DMG_CPU::add_a_e(void) {
    add(&CpuRegisters.A, CpuRegisters.E);
}

void DMG_CPU::add_a_h(void) {
    add(&CpuRegisters.A, CpuRegisters.H);
}

void DMG_CPU::add_a_l(void) {
    add(&CpuRegisters.A, CpuRegisters.L);
}

void DMG_CPU::add_a_hl(void) {
    //add(&CpuRegisters.A, CpuRegisters.HL);
}

void DMG_CPU::add_a_a(void) {
    add(&CpuRegisters.A, CpuRegisters.A);
}

// 16-bit arithmetic instructions

// Rotate, shift, and bit operation instructions

// Control flow instructions

void DMG_CPU::set_7_a(void) {
    set(1 << 7, &CpuRegisters.A);
}

// END instruction functions

// instructions
void DMG_CPU::ret(bool condition) {}

void DMG_CPU::xor_(uint8_t value) {}

void DMG_CPU::inc(uint8_t* value) {}

void DMG_CPU::dec(uint8_t* value) {}

void DMG_CPU::add(uint8_t* destination, uint8_t value) {
    this->logger->log("[DMG_CPU] add (8-8) %02X", destination);
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0f) + (value & 0x0f)) > 0x0f);
    *destination = result;
    setFlag(FLAG_ZERO, !*destination);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(uint16_t* destination, uint16_t value) {
    this->logger->log("[DMG_CPU] add (16-16) %02X", destination);
    uint32_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xffff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0fff) + (value & 0x0fff)) > 0x0fff);
    *destination = (uint16_t)result;
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(uint16_t* destination, int8_t value) {
    this->logger->log("[DMG_CPU] add (16-8) %02X", destination);
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, ((CpuRegisters.SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((CpuRegisters.SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
    *destination = result & 0xFFFF;
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
}

void DMG_CPU::ldhl(int8_t value) {}

void DMG_CPU::adc(uint8_t value) {}

void DMG_CPU::sbc(uint8_t value) {}

void DMG_CPU::sub(uint8_t value) {}

void DMG_CPU::and_(uint8_t value) {}

void DMG_CPU::or_(uint8_t value) {}

void DMG_CPU::cp(uint8_t value) {}

void DMG_CPU::call(bool condition) {}

void DMG_CPU::jump(bool condition) {}

void DMG_CPU::jump_add(bool condition) {}

void DMG_CPU::cp_n(uint8_t value) {}

// extended instructions
void DMG_CPU::extended_execute(uint8_t opcode) {}

void DMG_CPU::bit(uint8_t bit, uint8_t value) {}

void DMG_CPU::res(uint8_t bit, uint8_t* rgst) {}

void DMG_CPU::set(uint8_t bit, uint8_t* rgst) {
    *rgst |= bit;
}

void DMG_CPU::rl(uint8_t* value) {}

void DMG_CPU::rlc(uint8_t* value) {}

void DMG_CPU::rr(uint8_t* value) {}

void DMG_CPU::rrc(uint8_t* value) {}

void DMG_CPU::rra() {}

void DMG_CPU::rla() {}

void DMG_CPU::rlca() {}

void DMG_CPU::sla(uint8_t* value) {}

void DMG_CPU::sra(uint8_t* value) {}

void DMG_CPU::srl(uint8_t* value) {}

void DMG_CPU::swap(uint8_t* value) {}

void DMG_CPU::logCall(const char* op) {
    this->logger->log("[DMG_CPU] CALL %s", op);
}