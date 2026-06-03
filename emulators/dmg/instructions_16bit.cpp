#include "cpu.hpp"

void DMG_CPU::executeInstruction16bit(bool ROMFileLoaded, uint8_t opcode) {
    if (!ROMFileLoaded) return;

    switch (opcode) {
        case 0x00: // RLC B
            rlc(&Registers.B);
            break;
        case 0x01: // RLC C
            rlc(&Registers.C);
            break;
        case 0x02: // RLC D
            rlc(&Registers.D);
            break;
        case 0x03: // RLC E
            rlc(&Registers.E);
            break;
        case 0x04: // RLC H
            rlc(&Registers.H);
            break;
        case 0x05: // RLC L
            rlc(&Registers.L);
            break;
        case 0x06: // RLC (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                rlc(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x07: // RLC A
            rlc(&Registers.A);
            break;
        case 0x08: // RRC B
            rrc(&Registers.B);
            break;
        case 0x09: // RRC C
            rrc(&Registers.C);
            break;
        case 0x0A: // RRC D
            rrc(&Registers.D);
            break;
        case 0x0B: // RRC E
            rrc(&Registers.E);
            break;
        case 0x0C: // RRC H
            rrc(&Registers.H);
            break;
        case 0x0D: // RRC L
            rrc(&Registers.L);
            break;
        case 0x0E: // RRC (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                rrc(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x0F: // RRC A
            rrc(&Registers.A);
            break;
        case 0x10: // RL B
            rl(&Registers.B);
            break;
        case 0x11: // RL C
            rl(&Registers.C);
            break;
        case 0x12: // RL D
            rl(&Registers.D);
            break;
        case 0x13: // RL E
            rl(&Registers.E);
            break;
        case 0x14: // RL H
            rl(&Registers.H);
            break;
        case 0x15: // RL L
            rl(&Registers.L);
            break;
        case 0x16: // RL (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                rl(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x17: // RL A
            rl(&Registers.A);
            break;
        case 0x18: // RR B
            rr(&Registers.B);
            break;
        case 0x19: // RR C
            rr(&Registers.C);
            break;
        case 0x1A: // RR D
            rr(&Registers.D);
            break;
        case 0x1B: // RR E
            rr(&Registers.E);
            break;
        case 0x1C: // RR H
            rr(&Registers.H);
            break;
        case 0x1D: // RR L
            rr(&Registers.L);
            break;
        case 0x1E: // RR (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                rr(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x1F: // RR A
            rr(&Registers.A);
            break;
        case 0x20: // SLA B
            sla(&Registers.B);
            break;
        case 0x21: // SLA C
            sla(&Registers.C);
            break;
        case 0x22: // SLA D
            sla(&Registers.D);
            break;
        case 0x23: // SLA E
            sla(&Registers.E);
            break;
        case 0x24: // SLA H
            sla(&Registers.H);
            break;
        case 0x25: // SLA L
            sla(&Registers.L);
            break;
        case 0x26: // SLA (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                sla(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x27: // SRA A
            sla(&Registers.A);
            break;
        case 0x28: // SRA B
            sra(&Registers.B);
            break;
        case 0x29: // SRA C
            sra(&Registers.C);
            break;
        case 0x2A: // SRA D
            sra(&Registers.D);
            break;
        case 0x2B: // SRA E
            sra(&Registers.E);
            break;
        case 0x2C: // SRA H
            sra(&Registers.H);
            break;
        case 0x2D: // SRA L
            sra(&Registers.L);
            break;
        case 0x2E: // SRA (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                sra(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x2F: // SRA A
            sra(&Registers.A);
            break;
        case 0x30: // SWAP B
            swap(&Registers.B);
            break;
        case 0x31: // SWAP C
            swap(&Registers.C);
            break;
        case 0x32: // SWAP D
            swap(&Registers.D);
            break;
        case 0x33: // SWAP E
            swap(&Registers.E);
            break;
        case 0x34: // SWAP H
            swap(&Registers.H);
            break;
        case 0x35: // SWAP L
            swap(&Registers.L);
            break;
        case 0x36: // SWAP (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                swap(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x37: // SWAP A
            swap(&Registers.A);
            break;
        case 0x38: // SRL B
            srl(&Registers.B);
            break;
        case 0x39: // SRL C
            srl(&Registers.C);
            break;
        case 0x3A: // SRL D
            srl(&Registers.D);
            break;
        case 0x3B: // SRL E
            srl(&Registers.E);
            break;
        case 0x3C: // SRL H
            srl(&Registers.H);
            break;
        case 0x3D: // SRL L
            srl(&Registers.L);
            break;
        case 0x3E: // SRL (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                srl(&value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x3F: // SRL A
            srl(&Registers.A);
            break;
        case 0x40: // BIT 0, B
            bit(1 << 0, Registers.B);
            break;
        case 0x41: // BIT 0, C
            bit(1 << 0, Registers.C);
            break;
        case 0x42: // BIT 0, D
            bit(1 << 0, Registers.D);
            break;
        case 0x43: // BIT 0, E
            bit(1 << 0, Registers.E);
            break;
        case 0x44: // BIT 0, H
            bit(1 << 0, Registers.H);
            break;
        case 0x45: // BIT 0, L
            bit(1 << 0, Registers.L);
            break;
        case 0x46: // BIT 0, (HL)
            bit(1 << 0, mmu.read8(Registers.HL));
            break;
        case 0x47: // BIT 0, A
            bit(1 << 0, Registers.A);
            break;
        case 0x48: // BIT 1, B
            bit(1 << 1, Registers.B);
            break;
        case 0x49: // BIT 1, C
            bit(1 << 1, Registers.C);
            break;
        case 0x4A: // BIT 1, D
            bit(1 << 1, Registers.D);
            break;
        case 0x4B: // BIT 1, E
            bit(1 << 1, Registers.E);
            break;
        case 0x4C: // BIT 1, H
            bit(1 << 1, Registers.H);
            break;
        case 0x4D: // BIT 1, L
            bit(1 << 1, Registers.L);
            break;
        case 0x4E: // BIT 1, (HL)
            bit(1 << 1, mmu.read8(Registers.HL));
            break;
        case 0x4F: // BIT 1, A
            bit(1 << 1, Registers.A);
            break;
        case 0x50: // BIT 2, B
            bit(1 << 2, Registers.B);
            break;
        case 0x51: // BIT 2, C
            bit(1 << 2, Registers.C);
            break;
        case 0x52: // BIT 2, D
            bit(1 << 2, Registers.D);
            break;
        case 0x53: // BIT 2, E
            bit(1 << 2, Registers.E);
            break;
        case 0x54: // BIT 2, H
            bit(1 << 2, Registers.H);
            break;
        case 0x55: // BIT 2, L
            bit(1 << 2, Registers.L);
            break;
        case 0x56: // BIT 2, (HL)
            bit(1 << 2, mmu.read8(Registers.HL));
            break;
        case 0x57: // BIT 2, A
            bit(1 << 2, Registers.A);
            break;
        case 0x58: // BIT 3, B
            bit(1 << 3, Registers.B);
            break;
        case 0x59: // BIT 3, C
            bit(1 << 3, Registers.C);
            break;
        case 0x5A: // BIT 3, D
            bit(1 << 3, Registers.D);
            break;
        case 0x5B: // BIT 3, E
            bit(1 << 3, Registers.E);
            break;
        case 0x5C: // BIT 3, H
            bit(1 << 3, Registers.H);
            break;
        case 0x5D: // BIT 3, L
            bit(1 << 3, Registers.L);
            break;
        case 0x5E: // BIT 3, (HL)
            bit(1 << 3, mmu.read8(Registers.HL));
            break;
        case 0x5F: // BIT 3, A
            bit(1 << 3, Registers.A);
            break;
        case 0x60: // BIT 4, B
            bit(1 << 4, Registers.B);
            break;
        case 0x61: // BIT 4, C
            bit(1 << 4, Registers.C);
            break;
        case 0x62: // BIT 4, D
            bit(1 << 4, Registers.D);
            break;
        case 0x63: // BIT 4, E
            bit(1 << 4, Registers.E);
            break;
        case 0x64: // BIT 4, H
            bit(1 << 4, Registers.H);
            break;
        case 0x65: // BIT 4, L
            bit(1 << 4, Registers.L);
            break;
        case 0x66: // BIT 4, (HL)
            bit(1 << 4, mmu.read8(Registers.HL));
            break;
        case 0x67: // BIT 4, A
            bit(1 << 4, Registers.A);
            break;
        case 0x68: // BIT 5, B
            bit(1 << 5, Registers.B);
            break;
        case 0x69: // BIT 5, C
            bit(1 << 5, Registers.C);
            break;
        case 0x6A: // BIT 5, D
            bit(1 << 5, Registers.D);
            break;
        case 0x6B: // BIT 5, E
            bit(1 << 5, Registers.E);
            break;
        case 0x6C: // BIT 5, H
            bit(1 << 5, Registers.H);
            break;
        case 0x6D: // BIT 5, L
            bit(1 << 5, Registers.L);
            break;
        case 0x6E: // BIT 5, (HL)
            bit(1 << 5, mmu.read8(Registers.HL));
            break;
        case 0x6F: // BIT 5, A
            bit(1 << 5, Registers.A);
            break;
        case 0x70: // BIT 6, B
            bit(1 << 6, Registers.B);
            break;
        case 0x71: // BIT 6, C
            bit(1 << 6, Registers.C);
            break;
        case 0x72: // BIT 6, D
            bit(1 << 6, Registers.D);
            break;
        case 0x73: // BIT 6, E
            bit(1 << 6, Registers.E);
            break;
        case 0x74: // BIT 6, H
            bit(1 << 6, Registers.H);
            break;
        case 0x75: // BIT 6, L
            bit(1 << 6, Registers.L);
            break;
        case 0x76: // BIT 6, (HL)
            bit(1 << 6, mmu.read8(Registers.HL));
            break;
        case 0x77: // BIT 6, A
            bit(1 << 6, Registers.A);
            break;
        case 0x78: // BIT 7, B
            bit(1 << 7, Registers.B);
            break;
        case 0x79: // BIT 7, C
            bit(1 << 7, Registers.C);
            break;
        case 0x7A: // BIT 7, D
            bit(1 << 7, Registers.D);
            break;
        case 0x7B: // BIT 7, E
            bit(1 << 7, Registers.E);
            break;
        case 0x7C: // BIT 7, H
            bit(1 << 7, Registers.H);
            break;
        case 0x7D: // BIT 7, L
            bit(1 << 7, Registers.L);
            break;
        case 0x7E: // BIT 7, (HL)
            bit(1 << 7, mmu.read8(Registers.HL));
            break;
        case 0x7F: // BIT 7, A
            bit(1 << 7, Registers.A);
            break;
        case 0x80: // RES 0, B
            res(1 << 0, &Registers.B);
            break;
        case 0x81: // RES 0, C
            res(1 << 0, &Registers.C);
            break;
        case 0x82: // RES 0, D
            res(1 << 0, &Registers.D);
            break;
        case 0x83: // RES 0, E
            res(1 << 0, &Registers.E);
            break;
        case 0x84: // RES 0, H
            res(1 << 0, &Registers.H);
            break;
        case 0x85: // RES 0, L
            res(1 << 0, &Registers.L);
            break;
        case 0x86: // RES 0, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 0, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x87: // RES 0, A
            res(1 << 0, &Registers.A);
            break;

        case 0x88: // RES 1, B
            res(1 << 1, &Registers.B);
            break;
        case 0x89: // RES 1, C
            res(1 << 1, &Registers.C);
            break;
        case 0x8A: // RES 1, D
            res(1 << 1, &Registers.D);
            break;
        case 0x8B: // RES 1, E
            res(1 << 1, &Registers.E);
            break;
        case 0x8C: // RES 1, H
            res(1 << 1, &Registers.H);
            break;
        case 0x8D: // RES 1, L
            res(1 << 1, &Registers.L);
            break;
        case 0x8E: // RES 1, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 1, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x8F: // RES 1, A
            res(1 << 1, &Registers.A);
            break;
        case 0x90: // RES 2, B
            res(1 << 2, &Registers.B);
            break;
        case 0x91: // RES 2, C
            res(1 << 2, &Registers.C);
            break;
        case 0x92: // RES 2, D
            res(1 << 2, &Registers.D);
            break;
        case 0x93: // RES 2, E
            res(1 << 2, &Registers.E);
            break;
        case 0x94: // RES 2, H
            res(1 << 2, &Registers.H);
            break;
        case 0x95: // RES 2, L
            res(1 << 2, &Registers.L);
            break;
        case 0x96: // RES 2, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 2, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x97: // RES 2, A
            res(1 << 2, &Registers.A);
            break;
        case 0x98: // RES 3, B
            res(1 << 3, &Registers.B);
            break;
        case 0x99: // RES 3, C
            res(1 << 3, &Registers.C);
            break;
        case 0x9A: // RES 3, D
            res(1 << 3, &Registers.D);
            break;
        case 0x9B: // RES 3, E
            res(1 << 3, &Registers.E);
            break;
        case 0x9C: // RES 3, H
            res(1 << 3, &Registers.H);
            break;
        case 0x9D: // RES 3, L
            res(1 << 3, &Registers.L);
            break;
        case 0x9E: // RES 3, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 3, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0x9F: // RES 3, A
            res(1 << 3, &Registers.A);
            break;
        case 0xA0: // RES 4, B
            res(1 << 4, &Registers.B);
            break;
        case 0xA1: // RES 4, C
            res(1 << 4, &Registers.C);
            break;
        case 0xA2: // RES 4, D
            res(1 << 4, &Registers.D);
            break;
        case 0xA3: // RES 4, E
            res(1 << 4, &Registers.E);
            break;
        case 0xA4: // RES 4, H
            res(1 << 4, &Registers.H);
            break;
        case 0xA5: // RES 4, L
            res(1 << 4, &Registers.L);
            break;
        case 0xA6: // RES 4, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 4, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xa7: // RES 4, A
            res(1 << 4, &Registers.A);
            break;
        case 0xa8: // RES 5, B
            res(1 << 5, &Registers.B);
            break;
        case 0xa9: // RES 5, C
            res(1 << 5, &Registers.C);
            break;
        case 0xAA: // RES 5, D
            res(1 << 5, &Registers.D);
            break;
        case 0xAB: // RES 5, E
            res(1 << 5, &Registers.E);
            break;
        case 0xAC: // RES 5, H
            res(1 << 5, &Registers.H);
            break;
        case 0xAD: // RES 5, L
            res(1 << 5, &Registers.L);
            break;
        case 0xAE: // RES 5, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 5, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xAF: // RES 5, A
            res(1 << 5, &Registers.A);
            break;
        case 0xB0: // RES 6, B
            res(1 << 6, &Registers.B);
            break;
        case 0xB1: // RES 6, C
            res(1 << 6, &Registers.C);
            break;
        case 0xB2: // RES 6, D
            res(1 << 6, &Registers.D);
            break;
        case 0xB3: // RES 6, E
            res(1 << 6, &Registers.E);
            break;
        case 0xB4: // RES 6, H
            res(1 << 6, &Registers.H);
            break;
        case 0xB5: // RES 6, L
            res(1 << 6, &Registers.L);
            break;
        case 0xB6: // RES 6, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 6, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xb7: // RES 6, A
            res(1 << 6, &Registers.A);
            break;
        case 0xb8: // RES 7, B
            res(1 << 7, &Registers.B);
            break;
        case 0xb9: // RES 7, C
            res(1 << 7, &Registers.C);
            break;
        case 0xBA: // RES 7, D
            res(1 << 7, &Registers.D);
            break;
        case 0xBB: // RES 7, E
            res(1 << 7, &Registers.E);
            break;
        case 0xBC: // RES 7, H
            res(1 << 7, &Registers.H);
            break;
        case 0xBD: // RES 7, L
            res(1 << 7, &Registers.L);
            break;
        case 0xBE: // RES 7, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                res(1 << 7, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xBF: // RES 7, A
            res(1 << 7, &Registers.A);
            break;
        case 0xC0: // SET 0, B
            set(1 << 0, &Registers.B);
            break;
        case 0xC1: // SET 0, C
            set(1 << 0, &Registers.C);
            break;
        case 0xC2: // SET 0, D
            set(1 << 0, &Registers.D);
            break;
        case 0xC3: // SET 0, E
            set(1 << 0, &Registers.E);
            break;
        case 0xC4: // SET 0, H
            set(1 << 0, &Registers.H);
            break;
        case 0xC5: // SET 0, L
            set(1 << 0, &Registers.L);
            break;
        case 0xC6: // SET 0, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 0, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xc7: // SET 0, A
            set(1 << 0, &Registers.A);
            break;
        case 0xc8: // SET 1, B
            set(1 << 1, &Registers.B);
            break;
        case 0xc9: // SET 1, C
            set(1 << 1, &Registers.C);
            break;
        case 0xCA: // SET 1, D
            set(1 << 1, &Registers.D);
            break;
        case 0xCB: // SET 1, E
            set(1 << 1, &Registers.E);
            break;
        case 0xCC: // SET 1, H
            set(1 << 1, &Registers.H);
            break;
        case 0xCD: // SET 1, L
            set(1 << 1, &Registers.L);
            break;
        case 0xCE: // SET 1, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 1, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xCF: // SET 1, A
            set(1 << 1, &Registers.A);
            break;
        case 0xD0: // SET 2, B
            set(1 << 2, &Registers.B);
            break;
        case 0xD1: // SET 2, C
            set(1 << 2, &Registers.C);
            break;
        case 0xD2: // SET 2, D
            set(1 << 2, &Registers.D);
            break;
        case 0xD3: // SET 2, E
            set(1 << 2, &Registers.E);
            break;
        case 0xD4: // SET 2, H
            set(1 << 2, &Registers.H);
            break;
        case 0xD5: // SET 2, L
            set(1 << 2, &Registers.L);
            break;
        case 0xD6: // SET 2, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 2, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xD7: // SET 2, A
            set(1 << 2, &Registers.A);
            break;
        case 0xD8: // SET 3, B
            set(1 << 3, &Registers.B);
            break;
        case 0xD9: // SET 3, C
            set(1 << 3, &Registers.C);
            break;
        case 0xDA: // SET 3, D
            set(1 << 3, &Registers.D);
            break;
        case 0xDB: // SET 3, E
            set(1 << 3, &Registers.E);
            break;
        case 0xDC: // SET 3, H
            set(1 << 3, &Registers.H);
            break;
        case 0xDD: // SET 3, L
            set(1 << 3, &Registers.L);
            break;
        case 0xDE: // SET 3, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 3, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xDF: // SET 3, A
            set(1 << 3, &Registers.A);
            break;
        case 0xE0: // SET 4, B
            set(1 << 4, &Registers.B);
            break;
        case 0xE1: // SET 4, C
            set(1 << 4, &Registers.C);
            break;
        case 0xE2: // SET 4, D
            set(1 << 4, &Registers.D);
            break;
        case 0xE3: // SET 4, E
            set(1 << 4, &Registers.E);
            break;
        case 0xE4: // SET 4, H
            set(1 << 4, &Registers.H);
            break;
        case 0xE5: // SET 4, L
            set(1 << 4, &Registers.L);
            break;
        case 0xE6: // SET 4, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 4, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xE7: // SET 4, A
            set(1 << 4, &Registers.A);
            break;
        case 0xE8: // SET 5, B
            set(1 << 5, &Registers.B);
            break;
        case 0xE9: // SET 5, C
            set(1 << 5, &Registers.C);
            break;
        case 0xEA: // SET 5, D
            set(1 << 5, &Registers.D);
            break;
        case 0xEB: // SET 5, E
            set(1 << 5, &Registers.E);
            break;
        case 0xEC: // SET 5, H
            set(1 << 5, &Registers.H);
            break;
        case 0xED: // SET 5, L
            set(1 << 5, &Registers.L);
            break;
        case 0xEE: // SET 5, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 5, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xEF: // SET 5, A
            set(1 << 5, &Registers.A);
            break;
        case 0xF0: // SET 6, B
            set(1 << 6, &Registers.B);
            break;
        case 0xF1: // SET 6, C
            set(1 << 6, &Registers.C);
            break;
        case 0xF2: // SET 6, D
            set(1 << 6, &Registers.D);
            break;
        case 0xF3: // SET 6, E
            set(1 << 6, &Registers.E);
            break;
        case 0xF4: // SET 6, H
            set(1 << 6, &Registers.H);
            break;
        case 0xF5: // SET 6, L
            set(1 << 6, &Registers.L);
            break;
        case 0xF6: // SET 6, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 6, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xF7: // SET 6, A
            set(1 << 6, &Registers.A);
            break;
        case 0xF8: // SET 7, B
            set(1 << 7, &Registers.B);
            break;
        case 0xF9: // SET 7, C
            set(1 << 7, &Registers.C);
            break;
        case 0xFA: // SET 7, D
            set(1 << 7, &Registers.D);
            break;
        case 0xFB: // SET 7, E
            set(1 << 7, &Registers.E);
            break;
        case 0xFC: // SET 7, H
            set(1 << 7, &Registers.H);
            break;
        case 0xFD: // SET 7, L
            set(1 << 7, &Registers.L);
            break;
        case 0xFE: // SET 7, (HL)
            {
                uint8_t value = mmu.read8(Registers.HL);
                set(1 << 7, &value);
                mmu.write8(Registers.HL, value);
            }
            break;
        case 0xFF: // SET 7, A
            set(1 << 7, &Registers.A);
            break;
        default:
            logger.log("[DMG_CPU] ==================================================");
            logger.log("[DMG_CPU] [EXTENDED] Unsupported opcode: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG_CPU] ==================================================");
            return;
            break;
    }
}