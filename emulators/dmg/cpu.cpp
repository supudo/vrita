#include "cpu.hpp"

#include "utilities\strings.hpp"

void DMG_CPU::clearResources() {
    Registers.A = 0x01;
    Registers.F = 0xB0;
    Registers.B = 0x00;
    Registers.C = 0x13;
    Registers.D = 0x00;
    Registers.E = 0xD8;
    Registers.H = 0x01;
    Registers.L = 0x4D;
    Registers.SP = 0xFFFE;
    Registers.PC = 0x0100;
}

void DMG_CPU::step(bool ROMFileLoaded) {
    if (!ROMFileLoaded) return;

    uint8_t opcode = mmu.memory[Registers.PC];

    if (!mmu.triggerHaltBug)
        Registers.PC++;
    mmu.triggerHaltBug = false;

    if (opcode == 0xCB)
        executeInstruction16bit(ROMFileLoaded, mmu.read8(Registers.PC++));
    else
        executeInstruction8bit(ROMFileLoaded, opcode);
}

#pragma region instructions
void DMG_CPU::ret(const char* logMessage, bool condition) {
    logCall(true, concatOpcodeStrings(logMessage, " - ret"));
    mmu.tick(4);
    if (condition) {
        Registers.PC = mmu.readStack(&Registers.SP);
        mmu.tick(4);
    }
}

void DMG_CPU::xor_(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - xor_"));
    Registers.A ^= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::inc(const char* logMessage, uint8_t* value) {
    logCall(true, concatOpcodeStrings(logMessage, " - inc"));
    setFlag(FLAG_HALF_CARRY, (*value & 0x0f) == 0x0f);
    *value += 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::dec(const char* logMessage, uint8_t* value) {
    logCall(true, concatOpcodeStrings(logMessage, " - dec"));
    setFlag(FLAG_HALF_CARRY, !(*value & 0x0f));
    *value -= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::add(const char* logMessage, uint8_t* destination, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - ret 8-8"));
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0f) + (value & 0x0f)) > 0x0f);
    *destination = static_cast<uint8_t>(result);
    setFlag(FLAG_ZERO, !*destination);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(const char* logMessage, uint16_t* destination, uint16_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - add 16-16"));
    uint32_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xffff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0fff) + (value & 0x0fff)) > 0x0fff);
    *destination = (uint16_t)result;
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(const char* logMessage, uint16_t* destination, int8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - add 16-8"));
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, ((Registers.SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((Registers.SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
    *destination = result & 0xFFFF;
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
}

void DMG_CPU::ldhl(const char* logMessage, int8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - ldhl"));
    uint16_t result = Registers.SP + value;
    setFlag(FLAG_CARRY, ((Registers.SP ^ value ^ result) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((Registers.SP ^ value ^ result) & 0x10) == 0x10);
    Registers.HL = result;
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
}

void DMG_CPU::adc(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - adc"));
    int carry = getFlag(FLAG_CARRY) ? 1 : 0;
    int result = Registers.A + value + carry;
    setFlag(FLAG_ZERO, !(int8_t)result);
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((Registers.A & 0x0F) + (value & 0x0f) + carry) > 0x0F);
    setFlag(FLAG_SUBTRACT, false);
    Registers.A = (int8_t)(result & 0xff);
}

void DMG_CPU::sbc(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - sbc"));
    bool is_carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, (value + is_carry) > Registers.A);
    setFlag(FLAG_HALF_CARRY, ((value & 0x0f) + is_carry) > (Registers.A & 0x0f));
    Registers.A -= (value + is_carry);
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::sub(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - sub"));
    setFlag(FLAG_CARRY, value > Registers.A);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (Registers.A & 0x0f));
    Registers.A -= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::and_(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - and_"));
    Registers.A = Registers.A & value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT | FLAG_CARRY, false);
}

void DMG_CPU::or_(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - or_"));
    Registers.A |= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::cp(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - cp"));
    uint8_t temp_val = Registers.A;
    setFlag(FLAG_CARRY, value > temp_val);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (temp_val & 0x0f));
    temp_val -= value;
    setFlag(FLAG_ZERO, !temp_val);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::call(const char* logMessage, bool condition) {
    logCall(true, concatOpcodeStrings(logMessage, " - call"));
    uint16_t operand = mmu.read16(Registers.PC);
    Registers.PC += 2;
    if (condition) {
        mmu.tick(4);
        mmu.writeStack(&Registers.SP, Registers.PC);
        Registers.PC = operand;
    }
}

void DMG_CPU::jump(const char* logMessage, bool condition) {
    logCall(true, concatOpcodeStrings(logMessage, " - jump"));
    uint16_t addr = mmu.read16(Registers.PC);
    if (condition) {
        Registers.PC = addr;
        mmu.tick(4);
    }
    else
        Registers.PC += 2;
}

void DMG_CPU::jump_add(const char* logMessage, bool condition) {
    logCall(true, concatOpcodeStrings(logMessage, " - jump_add"));
    int8_t offset = (int8_t)mmu.read8(Registers.PC++);
    if (condition) {
        Registers.PC += offset;
        mmu.tick(4);
    }
}

void DMG_CPU::cp_n(const char* logMessage, uint8_t value) {
    logCall(true, concatOpcodeStrings(logMessage, " - cp_n"));
    setFlag(FLAG_SUBTRACT, true);
    setFlag(FLAG_ZERO, Registers.A == value);
    setFlag(FLAG_CARRY, value > Registers.A);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (Registers.A & 0x0f));
}
#pragma endregion

#pragma region extended instructions
void DMG_CPU::bit(const char* logMessage, uint8_t bit, uint8_t value) {
    logCall(false, concatOpcodeStrings(logMessage, "bit"));
    setFlag(FLAG_ZERO, !(value & bit));
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::res(const char* logMessage, uint8_t bit, uint8_t* rgst) {
    logCall(false, concatOpcodeStrings(logMessage, "res"));
    *rgst &= ~(bit);
}

void DMG_CPU::set(const char* logMessage, uint8_t bit, uint8_t* rgst) {
    logCall(false, concatOpcodeStrings(logMessage, "set"));
    *rgst |= bit;
}

void DMG_CPU::rl(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "rl"));
    int carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    *value += carry;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rlc(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "rlc"));
    int carry = (*value >> 7) & 0x01;
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    *value += carry;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rr(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "rr"));
    int carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, *value & 0x01);
    *value >>= 1;
    *value |= (carry << 7);
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rrc(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "rrc"));
    int carry = *value & 0x01;
    setFlag(FLAG_CARRY, carry);
    *value >>= 1;
    *value |= (carry << 7);
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sla(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "sla"));
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sra(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "sra"));
    setFlag(FLAG_CARRY, *value & 0x01);
    int msb = *value & (1 << 7);
    *value >>= 1;
    *value |= msb;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::srl(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "srl"));
    setFlag(FLAG_CARRY, *value & 0x01);
    *value >>= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::swap(const char* logMessage, uint8_t* value) {
    logCall(false, concatOpcodeStrings(logMessage, "swap"));
    uint8_t lower = *value << 4;
    *value = (*value >> 4) | lower;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY | FLAG_CARRY, false);
}
#pragma endregion

void DMG_CPU::logCall(bool isNormal, std::string msg) {
    // TODO: add instruction filter for debugging
    //static const char* suppress[] = { "0x05", "0x20", "0x32" };
    //for (const char* s : suppress)
    //    if (msg.rfind(s, 0) == 0) return;
    if (mmu.totalCycles % 10000 == 0)
        logger.log("[DMG-CPU] CALL %s %s", msg.c_str(), (isNormal ? "" : "(extended)"));
}
