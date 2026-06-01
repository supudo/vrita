#include "../../include/dmg/cpu.hpp"

void DMG_CPU::initialize(Logger& logger) {
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

    // instructions
    // Miscelanious
    InstructionsTable[0x00] = { "NOP", &DMG_CPU::NOP, 1, 4 };
    InstructionsTable[0x76] = { "HALT", &DMG_CPU::HALT, 1, 4 };
    InstructionsTable[0x10] = { "STOP", &DMG_CPU::STOP, 1, 4 };
    InstructionsTable[0xF3] = { "DI", &DMG_CPU::DI, 1, 4 };
    InstructionsTable[0xFB] = { "EI", &DMG_CPU::EI, 1, 4 };
}

void DMG_CPU::stepCPU(uint8_t *memory) {
    // =========================
    // FETCH
    // =========================
    uint8_t opcode = memory[CpuRegisters.PC++];
    const DMG_CPU::Instruction& instr = InstructionsTable[opcode];
    this->logger->log("[%02X] Executing %s", opcode, instr.name);
    //(this->*instr.execute)();

    // =========================
    // DECODE + EXECUTE
    // =========================
    switch (opcode) {
        // -------------------------
        // NOP
        // -------------------------
        case 0x00:
            cycles += 4;
            break;

        // -------------------------
        // LD BC, d16
        // -------------------------
        case 0x01: {
            uint8_t lo = memory[CpuRegisters.PC++];
            uint8_t hi = memory[CpuRegisters.PC++];
            CpuRegisters.setBC((hi << 8) | lo);
            cycles += 12;
            break;
        }

        // -------------------------
        // LD (BC), A
        // -------------------------
        case 0x02:
            memory[CpuRegisters.BC()] = CpuRegisters.A;
            cycles += 8;
            break;

            // -------------------------
            // INC BC
            // -------------------------
        case 0x03:
            CpuRegisters.setBC(CpuRegisters.BC() + 1);
            cycles += 8;
            break;

            // -------------------------
            // INC B
            // -------------------------
        case 0x04:
            CpuRegisters.B++;

            // flags: Z=1 if result is 0
            setFlag(FLAG_ZERO, CpuRegisters.B == 0);
            setFlag(FLAG_SUBSTRACT, false);
            setFlag(FLAG_HALF_CARRY, (CpuRegisters.B & 0x0F) == 0x00);

            cycles += 4;
            break;

            // -------------------------
            // STOP / HALT-like behavior (simplified)
            // -------------------------
        case 0x76:
            halted = true;
            cycles += 4;
            break;

            // =========================
            // CB PREFIX (extended opcodes)
            // =========================
        case 0xCB: {
            uint8_t cb = memory[CpuRegisters.PC++];

            switch (cb) {
                case 0x11: { // RL C (example)
                    uint8_t carry = getFlag(FLAG_CARRY);
                    uint8_t newCarry = (CpuRegisters.C & 0x80) >> 7;

                    CpuRegisters.C = (CpuRegisters.C << 1) | carry;

                    setFlag(FLAG_ZERO, CpuRegisters.C == 0);
                    setFlag(FLAG_SUBSTRACT, false);
                    setFlag(FLAG_HALF_CARRY, false);
                    setFlag(FLAG_CARRY, newCarry);

                    cycles += 8;
                    break;
                }

                default:
                    this->logger->log("Unimplemented CB opcode: 0x%X", (int)cb);
                    //std::cerr << "Unimplemented CB opcode: " << std::hex << (int)cb << "\n";
                    break;
            }
            break;
        }

        // =========================
        // UNKNOWN OPCODE
        // =========================
        default:
            this->logger->log("Unknown opcode: 0x%X", (int)opcode); 
            //std::cerr << "Unknown opcode: " << std::hex << (int)opcode << "\n";
            break;
    }
}

// BEGIN instruction functions
// Miscelanious instructions
void DMG_CPU::NOP() {
    this->logger->log("NOP");
}

void DMG_CPU::HALT() {
    halted = true;
}

void DMG_CPU::STOP() {
    halted = true;
}

void DMG_CPU::DI(void) {}

void DMG_CPU::EI(void) {}
// 8-bit load instructions
// 16-it load instructions
// 8-bit arithmetic and logical instructions
// 16-bit arithmetic instructions
// Rotate, shift, and bit operation instructions
// Control flow instructions
// END instruction functions