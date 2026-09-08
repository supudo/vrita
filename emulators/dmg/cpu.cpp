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

void DMG_CPU::step() {
#ifdef TRACY_ENABLE
    ZoneScopedN("CPU::Step");
#endif

    currentInstructionPC = Registers.PC;

    uint8_t opcode = mmu.read8(Registers.PC);

    if (!mmu.triggerHaltBug)
        Registers.PC++;
    mmu.triggerHaltBug = false;

    if (opcode == 0xCB)
        executeInstruction16bit(mmu.read8(Registers.PC++));
    else
        executeInstruction8bit(opcode);
}

void DMG_CPU::logCall(bool isNormal, const char* msg1, const char* msg2) {
    if (!logCalls) return;
    logger.log("[DMG-CPU] CALL %s%s %s", msg1, msg2, (isNormal ? "" : "(extended)"));
}
