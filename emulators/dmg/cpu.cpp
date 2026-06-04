#include "cpu.hpp"

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
    
    if (!mmu.trigger_halt_bug)
        Registers.PC++;
    mmu.trigger_halt_bug = false;

    if (opcode == 0xCB)
        executeInstruction16bit(ROMFileLoaded, mmu.read8(Registers.PC++));
    else
        executeInstruction8bit(ROMFileLoaded, opcode);
}

#pragma region instructions
void DMG_CPU::ret(bool condition) {
    logCall(true, "ret");
    mmu.tick(4);
    if (condition) {
        Registers.PC = mmu.read_stack(&Registers.SP);
        mmu.tick(4);
    }
}

void DMG_CPU::xor_(uint8_t value) {
    logCall(true, "xor_");
    Registers.A ^= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::inc(uint8_t* value) {
    logCall(true, "inc");
    setFlag(FLAG_HALF_CARRY, (*value & 0x0f) == 0x0f);
    *value += 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::dec(uint8_t* value) {
    logCall(true, "dec");
    setFlag(FLAG_HALF_CARRY, !(*value & 0x0f));
    *value -= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::add(uint8_t* destination, uint8_t value) {
    logCall(true, "ret 8-8");
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0f) + (value & 0x0f)) > 0x0f);
    *destination = static_cast<uint8_t>(result);
    setFlag(FLAG_ZERO, !*destination);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(uint16_t* destination, uint16_t value) {
    logCall(true, "add 16-16");
    uint32_t result = *destination + value;
    setFlag(FLAG_CARRY, result > 0xffff);
    setFlag(FLAG_HALF_CARRY, ((*destination & 0x0fff) + (value & 0x0fff)) > 0x0fff);
    *destination = (uint16_t)result;
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::add(uint16_t* destination, int8_t value) {
    logCall(true, "add 16-8");
    uint16_t result = *destination + value;
    setFlag(FLAG_CARRY, ((Registers.SP ^ value ^ (result & 0xFFFF)) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((Registers.SP ^ value ^ (result & 0xFFFF)) & 0x10) == 0x10);
    *destination = result & 0xFFFF;
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
}

void DMG_CPU::ldhl(int8_t value) {
    logCall(true, "ldhl");
    uint16_t result = Registers.SP + value;
    setFlag(FLAG_CARRY, ((Registers.SP ^ value ^ result) & 0x100) == 0x100);
    setFlag(FLAG_HALF_CARRY, ((Registers.SP ^ value ^ result) & 0x10) == 0x10);
    Registers.HL = result;
    setFlag(FLAG_SUBTRACT | FLAG_ZERO, false);
}

void DMG_CPU::adc(uint8_t value) {
    logCall(true, "adc");
    int carry = getFlag(FLAG_CARRY) ? 1 : 0;
    int result = Registers.A + value + carry;
    setFlag(FLAG_ZERO, !(int8_t)result);
    setFlag(FLAG_CARRY, result > 0xff);
    setFlag(FLAG_HALF_CARRY, ((Registers.A & 0x0F) + (value & 0x0f) + carry) > 0x0F);
    setFlag(FLAG_SUBTRACT, false);
    Registers.A = (int8_t)(result & 0xff);
}

void DMG_CPU::sbc(uint8_t value) {
    logCall(true, "sbc");
    bool is_carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, (value + is_carry) > Registers.A);
    setFlag(FLAG_HALF_CARRY, ((value & 0x0f) + is_carry) > (Registers.A & 0x0f));
    Registers.A -= (value + is_carry);
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::sub(uint8_t value) {
    logCall(true, "sub");
    setFlag(FLAG_CARRY, value > Registers.A);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (Registers.A & 0x0f));
    Registers.A -= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::and_(uint8_t value) {
    logCall(true, "and_");
    Registers.A = Registers.A & value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT | FLAG_CARRY, false);
}

void DMG_CPU::or_(uint8_t value) {
    logCall(true, "or_");
    Registers.A |= value;
    setFlag(FLAG_ZERO, !Registers.A);
    setFlag(FLAG_CARRY | FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::cp(uint8_t value) {
    logCall(true, "cp");
    uint8_t temp_val = Registers.A;
    setFlag(FLAG_CARRY, value > temp_val);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (temp_val & 0x0f));
    temp_val -= value;
    setFlag(FLAG_ZERO, !temp_val);
    setFlag(FLAG_SUBTRACT, true);
}

void DMG_CPU::call(bool condition) {
    logCall(true, "call");
    uint16_t operand = mmu.read16(Registers.PC);
    Registers.PC += 2;
    if (condition) {
        mmu.tick(4);
        mmu.write_stack(&Registers.SP, Registers.PC);
        Registers.PC = operand;
    }
}

void DMG_CPU::jump(bool condition) {
    logCall(true, "jump");
    uint16_t addr = mmu.read16(Registers.PC);
    if (condition) {
        Registers.PC = addr;
        mmu.tick(4);
    }
    else
        Registers.PC += 2;
}

void DMG_CPU::jump_add(bool condition) {
    logCall(true, "jump_add");
    int8_t offset = (int8_t)mmu.read8(Registers.PC++);
    if (condition) {
        Registers.PC += offset;
        mmu.tick(4);
    }
}

void DMG_CPU::cp_n(uint8_t value) {
    logCall(true, "cp_n");
    setFlag(FLAG_SUBTRACT, true);
    setFlag(FLAG_ZERO, Registers.A == value);
    setFlag(FLAG_CARRY, value > Registers.A);
    setFlag(FLAG_HALF_CARRY, (value & 0x0f) > (Registers.A & 0x0f));
}
#pragma endregion

#pragma region extended instructions
void DMG_CPU::bit(uint8_t bit, uint8_t value) {
    logCall(false, "bit");
    setFlag(FLAG_ZERO, !(value & bit));
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::res(uint8_t bit, uint8_t* rgst) {
    logCall(false, "res");
    *rgst &= ~(bit);
}

void DMG_CPU::set(uint8_t bit, uint8_t* rgst) {
    logCall(false, "set");
    *rgst |= bit;
}

void DMG_CPU::rl(uint8_t* value) {
    logCall(false, "rl");
    int carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    *value += carry;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rlc(uint8_t* value) {
    logCall(false, "rlc");
    int carry = (*value >> 7) & 0x01;
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    *value += carry;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rr(uint8_t* value) {
    logCall(false, "rr");
    int carry = getFlag(FLAG_CARRY);
    setFlag(FLAG_CARRY, *value & 0x01);
    *value >>= 1;
    *value |= (carry << 7);
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rrc(uint8_t* value) {
    logCall(false, "rrc");
    int carry = *value & 0x01;
    setFlag(FLAG_CARRY, carry);
    *value >>= 1;
    *value |= (carry << 7);
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sla(uint8_t* value) {
    logCall(false, "sla");
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sra(uint8_t* value) {
    logCall(false, "sra");
    setFlag(FLAG_CARRY, *value & 0x01);
    int msb = *value & (1 << 7);
    *value >>= 1;
    *value |= msb;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::srl(uint8_t* value) {
    logCall(false, "srl");
    setFlag(FLAG_CARRY, *value & 0x01);
    *value >>= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::swap(uint8_t* value) {
    logCall(false, "swap");
    uint8_t lower = *value << 4;
    *value = (*value >> 4) | lower;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY | FLAG_CARRY, false);
}
#pragma endregion

void DMG_CPU::logCall(bool is8bit, const char* msg) {
    logger.log("[DMG-CPU] CALL %ibit %s", (is8bit ? 8 : 16), msg);
}
