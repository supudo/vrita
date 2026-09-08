#include "cpu.hpp"

void DMG_CPU::ret(const char* logMessage, bool condition) {
    logCall(true, logMessage, " - ret");
    mmu.tick(4);
    if (condition) {
        Registers.PC = mmu.readStack(&Registers.SP);
        mmu.tick(4);
    }
}

void DMG_CPU::xor_(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - xor_");
    Registers.A ^= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::inc(const char* logMessage, uint8_t* value) {
    logCall(true, logMessage, " - inc");
    setFlag(FLAG_HALF_CARRY, (*value & 0x0f) == 0x0f);
    *value += 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::dec(const char* logMessage, uint8_t* value) {
    logCall(true, logMessage, " - dec");
    setFlag(FLAG_HALF_CARRY, !(*value & 0x0f));
    *value -= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::add(const char* logMessage, uint8_t* destination, uint8_t value) {
    logCall(true, logMessage, " - add 8-8");
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0f) + (value & 0x0f)) > 0x0f);
    *destination = static_cast<uint8_t>(result);
    setFlag(FLAG_ZERO, !*destination);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(const char* logMessage, uint16_t* destination, uint16_t value) {
    logCall(true, logMessage, " - add 16-16");
    uint32_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xffff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0fff) + (value & 0x0fff)) > 0x0fff);
    *destination = (uint16_t)result;
    setFlag(FLAG_SUBTRACT, false);
}

uint16_t DMG_CPU::addSignedToSP(const char* logMessage, int8_t value) {
    logCall(true, logMessage, " - addSignedToSP");
    uint16_t result = Registers.SP + value;
    setFlag(FLAG_CARRY, ((Registers.SP ^ value ^ result) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((Registers.SP ^ value ^ result) & 0x10) == 0x10);
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
    return result;
}

void DMG_CPU::adc(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - adc");
    int carry = isFlagSet(FLAG_CARRY) ? 1 : 0;
    int result = Registers.A + value + carry;
    setFlag(FLAG_ZERO, !(int8_t)result);
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((Registers.A & 0x0F) + (value & 0x0f) + carry) > 0x0F);
    setFlag(FLAG_SUBTRACT, false);
    Registers.A = (int8_t)(result & 0xff);
}

void DMG_CPU::sbc(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - sbc");
    bool is_carry = isFlagSet(FLAG_CARRY);
    setFlag(FLAG_CARRY, (value + is_carry) > Registers.A);
    setFlag(FLAG_HALF_CARRY, ((value & 0x0f) + is_carry) > (Registers.A & 0x0f));
    Registers.A -= (value + is_carry);
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::sub(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - sub");
    setFlag(FLAG_CARRY, value > Registers.A);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (Registers.A & 0x0f));
    Registers.A -= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::and_(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - and_");
    Registers.A = Registers.A & value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT | FLAG_CARRY, false);
}

void DMG_CPU::or_(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - or_");
    Registers.A |= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::cp(const char* logMessage, uint8_t value) {
    logCall(true, logMessage, " - cp");
    uint8_t temp_val = Registers.A;
    setFlag(FLAG_CARRY, value > temp_val);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (temp_val & 0x0f));
    temp_val -= value;
    setFlag(FLAG_ZERO, !temp_val);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::call(const char* logMessage, bool condition) {
    logCall(true, logMessage, " - call");
    uint16_t operand = mmu.read16(Registers.PC);
    Registers.PC += 2;
    if (condition) {
        mmu.tick(4);
        mmu.writeStack(&Registers.SP, Registers.PC);
        Registers.PC = operand;
    }
}

void DMG_CPU::jump(const char* logMessage, bool condition) {
    logCall(true, logMessage, " - jump");
    uint16_t addr = mmu.read16(Registers.PC);
    if (condition) {
        Registers.PC = addr;
        mmu.tick(4);
    }
    else
        Registers.PC += 2;
}

void DMG_CPU::jump_add(const char* logMessage, bool condition) {
    logCall(true, logMessage, " - jump_add");
    int8_t offset = (int8_t)mmu.read8(Registers.PC++);
    if (condition) {
        Registers.PC += offset;
        mmu.tick(4);
    }
}