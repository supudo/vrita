#include "cpu.hpp"

void DMG_CPU::executeInstruction16bit(bool ROMFileLoaded, uint8_t opcode) {
    if (!ROMFileLoaded) return;

    switch (opcode) {
        case 0x00: // RLC B
            rotateLeft("0x00 - RLC B", &Registers.B, false);
            break;
        case 0x01: // RLC C
            rotateLeft("0x01 - RLC C", &Registers.C, false);
            break;
        case 0x02: // RLC D
            rotateLeft("0x02 - RLC D", &Registers.D, false);
            break;
        case 0x03: // RLC E
            rotateLeft("0x03 - RLC E", &Registers.E, false);
            break;
        case 0x04: // RLC H
            rotateLeft("0x04 - RLC H", &Registers.H, false);
            break;
        case 0x05: // RLC L
            rotateLeft("0x05 - RLC L", &Registers.L, false);
            break;
        case 0x06: // RLC (HL)
            modifyHL([this](uint8_t& value) { rotateLeft("0x06 - RLC (HL)", &value, false); });
            break;
        case 0x07: // RLC A
            rotateLeft("0x07 - RLC A", &Registers.A, false);
            break;
        case 0x08: // RRC B
            rotateRight("0x08 - RRC B", &Registers.B, false);
            break;
        case 0x09: // RRC C
            rotateRight("0x09 - RRC C", &Registers.C, false);
            break;
        case 0x0A: // RRC D
            rotateRight("0x0A - RRC D", &Registers.D, false);
            break;
        case 0x0B: // RRC E
            rotateRight("0x0B - RRC E", &Registers.E, false);
            break;
        case 0x0C: // RRC H
            rotateRight("0x0C - RRC H", &Registers.H, false);
            break;
        case 0x0D: // RRC L
            rotateRight("0x0D - RRC L", &Registers.L, false);
            break;
        case 0x0E: // RRC (HL)
            modifyHL([this](uint8_t& value) { rotateRight("0x0E - RRC (HL)", &value, false); });
            break;
        case 0x0F: // RRC A
            rotateRight("0x0F - RRC A", &Registers.A, false);
            break;
        case 0x10: // RL B
            rotateLeft("0x10 - RL B", &Registers.B, true);
            break;
        case 0x11: // RL C
            rotateLeft("0x11 - RL C", &Registers.C, true);
            break;
        case 0x12: // RL D
            rotateLeft("0x12 - RL D", &Registers.D, true);
            break;
        case 0x13: // RL E
            rotateLeft("0x13 - RL E", &Registers.E, true);
            break;
        case 0x14: // RL H
            rotateLeft("0x14 - RL H", &Registers.H, true);
            break;
        case 0x15: // RL L
            rotateLeft("0x15 - RL L", &Registers.L, true);
            break;
        case 0x16: // RL (HL)
            modifyHL([this](uint8_t& value) { rotateLeft("0x16 - RL (HL)", &value, true); });
            break;
        case 0x17: // RL A
            rotateLeft("0x17 - RL A", &Registers.A, true);
            break;
        case 0x18: // RR B
            rotateRight("0x18 - RR B", &Registers.B, true);
            break;
        case 0x19: // RR C
            rotateRight("0x19 - RR C", &Registers.C, true);
            break;
        case 0x1A: // RR D
            rotateRight("0x1A - RR D", &Registers.D, true);
            break;
        case 0x1B: // RR E
            rotateRight("0x1B - RR E", &Registers.E, true);
            break;
        case 0x1C: // RR H
            rotateRight("0x1C - RR H", &Registers.H, true);
            break;
        case 0x1D: // RR L
            rotateRight("0x1D - RR L", &Registers.L, true);
            break;
        case 0x1E: // RR (HL)
            modifyHL([this](uint8_t& value) { rotateRight("0x1E - RR (HL)", &value, true); });
            break;
        case 0x1F: // RR A
            rotateRight("0x1F - RR A", &Registers.A, true);
            break;
        case 0x20: // SLA B
            sla("0x20 - SLA B", &Registers.B);
            break;
        case 0x21: // SLA C
            sla("0x21 - SLA C", &Registers.C);
            break;
        case 0x22: // SLA D
            sla("0x22 - SLA D", &Registers.D);
            break;
        case 0x23: // SLA E
            sla("0x23 - SLA E", &Registers.E);
            break;
        case 0x24: // SLA H
            sla("0x24 - SLA H", &Registers.H);
            break;
        case 0x25: // SLA L
            sla("0x25 - SLA L", &Registers.L);
            break;
        case 0x26: // SLA (HL)
            modifyHL([this](uint8_t& value) { sla("0x26 - SLA (HL)", &value); });
            break;
        case 0x27: // SLA A
            sla("0x27 - SLA A", &Registers.A);
            break;
        case 0x28: // SRA B
            sra("0x28 - SRA B", &Registers.B);
            break;
        case 0x29: // SRA C
            sra("0x29 - SRA C", &Registers.C);
            break;
        case 0x2A: // SRA D
            sra("0x2A - SRA D", &Registers.D);
            break;
        case 0x2B: // SRA E
            sra("0x2B - SRA E", &Registers.E);
            break;
        case 0x2C: // SRA H
            sra("0x2C - SRA H", &Registers.H);
            break;
        case 0x2D: // SRA L
            sra("0x2D - SRA L", &Registers.L);
            break;
        case 0x2E: // SRA (HL)
            modifyHL([this](uint8_t& value) { sra("0x2E - SRA (HL)", &value); });
            break;
        case 0x2F: // SRA A
            sra("0x2F - SRA A", &Registers.A);
            break;
        case 0x30: // SWAP B
            swap("0x30 - SWAP B", &Registers.B);
            break;
        case 0x31: // SWAP C
            swap("0x31 - SWAP C", &Registers.C);
            break;
        case 0x32: // SWAP D
            swap("0x32 - SWAP D", &Registers.D);
            break;
        case 0x33: // SWAP E
            swap("0x33 - SWAP E", &Registers.E);
            break;
        case 0x34: // SWAP H
            swap("0x34 - SWAP H", &Registers.H);
            break;
        case 0x35: // SWAP L
            swap("0x35 - SWAP L", &Registers.L);
            break;
        case 0x36: // SWAP (HL)
            modifyHL([this](uint8_t& value) { swap("0x36 - SWAP (HL)", &value); });
            break;
        case 0x37: // SWAP A
            swap("0x37 - SWAP A", &Registers.A);
            break;
        case 0x38: // SRL B
            srl("0x38 - SRL B", &Registers.B);
            break;
        case 0x39: // SRL C
            srl("0x39 - SRL C", &Registers.C);
            break;
        case 0x3A: // SRL D
            srl("0x3A - SRL D", &Registers.D);
            break;
        case 0x3B: // SRL E
            srl("0x3B - SRL E", &Registers.E);
            break;
        case 0x3C: // SRL H
            srl("0x3C - SRL H", &Registers.H);
            break;
        case 0x3D: // SRL L
            srl("0x3D - SRL L", &Registers.L);
            break;
        case 0x3E: // SRL (HL)
            modifyHL([this](uint8_t& value) { srl("0x3E - SRL (HL)", &value); });
            break;
        case 0x3F: // SRL A
            srl("0x3F - SRL A", &Registers.A);
            break;
        case 0x40: // BIT 0, B
            bit("0x40 - BIT 0, B", 1 << 0, Registers.B);
            break;
        case 0x41: // BIT 0, C
            bit("0x41 - BIT 0, C", 1 << 0, Registers.C);
            break;
        case 0x42: // BIT 0, D
            bit("0x42 - BIT 0, D", 1 << 0, Registers.D);
            break;
        case 0x43: // BIT 0, E
            bit("0x43 - BIT 0, E", 1 << 0, Registers.E);
            break;
        case 0x44: // BIT 0, H
            bit("0x44 - BIT 0, H", 1 << 0, Registers.H);
            break;
        case 0x45: // BIT 0, L
            bit("0x45 - BIT 0, L", 1 << 0, Registers.L);
            break;
        case 0x46: // BIT 0, (HL)
            bit("0x46 - BIT 0, (HL)", 1 << 0, mmu.read8(Registers.HL));
            break;
        case 0x47: // BIT 0, A
            bit("0x47 - BIT 0, A", 1 << 0, Registers.A);
            break;
        case 0x48: // BIT 1, B
            bit("0x48 - BIT 1, B", 1 << 1, Registers.B);
            break;
        case 0x49: // BIT 1, C
            bit("0x49 - BIT 1, C", 1 << 1, Registers.C);
            break;
        case 0x4A: // BIT 1, D
            bit("0x4A - BIT 1, D", 1 << 1, Registers.D);
            break;
        case 0x4B: // BIT 1, E
            bit("0x4B - BIT 1, E", 1 << 1, Registers.E);
            break;
        case 0x4C: // BIT 1, H
            bit("0x4C - BIT 1, H", 1 << 1, Registers.H);
            break;
        case 0x4D: // BIT 1, L
            bit("0x4D - BIT 1, L", 1 << 1, Registers.L);
            break;
        case 0x4E: // BIT 1, (HL)
            bit("0x4E - BIT 1, (HL)", 1 << 1, mmu.read8(Registers.HL));
            break;
        case 0x4F: // BIT 1, A
            bit("0x4F - BIT 1, A", 1 << 1, Registers.A);
            break;
        case 0x50: // BIT 2, B
            bit("0x50 - BIT 2, B", 1 << 2, Registers.B);
            break;
        case 0x51: // BIT 2, C
            bit("0x51 - BIT 2, C", 1 << 2, Registers.C);
            break;
        case 0x52: // BIT 2, D
            bit("0x52 - BIT 2, D", 1 << 2, Registers.D);
            break;
        case 0x53: // BIT 2, E
            bit("0x53 - BIT 2, E", 1 << 2, Registers.E);
            break;
        case 0x54: // BIT 2, H
            bit("0x54 - BIT 2, H", 1 << 2, Registers.H);
            break;
        case 0x55: // BIT 2, L
            bit("0x55 - BIT 2, L", 1 << 2, Registers.L);
            break;
        case 0x56: // BIT 2, (HL)
            bit("0x56 - BIT 2, (HL)", 1 << 2, mmu.read8(Registers.HL));
            break;
        case 0x57: // BIT 2, A
            bit("0x57 - BIT 2, A", 1 << 2, Registers.A);
            break;
        case 0x58: // BIT 3, B
            bit("0x58 - BIT 3, B", 1 << 3, Registers.B);
            break;
        case 0x59: // BIT 3, C
            bit("0x59 - BIT 3, C", 1 << 3, Registers.C);
            break;
        case 0x5A: // BIT 3, D
            bit("0x5A - BIT 3, D", 1 << 3, Registers.D);
            break;
        case 0x5B: // BIT 3, E
            bit("0x5B - BIT 3, E", 1 << 3, Registers.E);
            break;
        case 0x5C: // BIT 3, H
            bit("0x5C - BIT 3, H", 1 << 3, Registers.H);
            break;
        case 0x5D: // BIT 3, L
            bit("0x5D - BIT 3, L", 1 << 3, Registers.L);
            break;
        case 0x5E: // BIT 3, (HL)
            bit("0x5E - BIT 3, (HL)", 1 << 3, mmu.read8(Registers.HL));
            break;
        case 0x5F: // BIT 3, A
            bit("0x5F - BIT 3, A", 1 << 3, Registers.A);
            break;
        case 0x60: // BIT 4, B
            bit("0x60 - BIT 4, B", 1 << 4, Registers.B);
            break;
        case 0x61: // BIT 4, C
            bit("0x61 - BIT 4, C", 1 << 4, Registers.C);
            break;
        case 0x62: // BIT 4, D
            bit("0x62 - BIT 4, D", 1 << 4, Registers.D);
            break;
        case 0x63: // BIT 4, E
            bit("0x63 - BIT 4, E", 1 << 4, Registers.E);
            break;
        case 0x64: // BIT 4, H
            bit("0x64 - BIT 4, H", 1 << 4, Registers.H);
            break;
        case 0x65: // BIT 4, L
            bit("0x65 - BIT 4, L", 1 << 4, Registers.L);
            break;
        case 0x66: // BIT 4, (HL)
            bit("0x66 - BIT 4, (HL)", 1 << 4, mmu.read8(Registers.HL));
            break;
        case 0x67: // BIT 4, A
            bit("0x67 - BIT 4, A", 1 << 4, Registers.A);
            break;
        case 0x68: // BIT 5, B
            bit("0x68 - BIT 5, B", 1 << 5, Registers.B);
            break;
        case 0x69: // BIT 5, C
            bit("0x69 - BIT 5, C", 1 << 5, Registers.C);
            break;
        case 0x6A: // BIT 5, D
            bit("0x6A - BIT 5, D", 1 << 5, Registers.D);
            break;
        case 0x6B: // BIT 5, E
            bit("0x6B - BIT 5, E", 1 << 5, Registers.E);
            break;
        case 0x6C: // BIT 5, H
            bit("0x6C - BIT 5, H", 1 << 5, Registers.H);
            break;
        case 0x6D: // BIT 5, L
            bit("0x6D - BIT 5, L", 1 << 5, Registers.L);
            break;
        case 0x6E: // BIT 5, (HL)
            bit("0x6E - BIT 5, (HL)", 1 << 5, mmu.read8(Registers.HL));
            break;
        case 0x6F: // BIT 5, A
            bit("0x6F - BIT 5, A", 1 << 5, Registers.A);
            break;
        case 0x70: // BIT 6, B
            bit("0x70 - BIT 6, B", 1 << 6, Registers.B);
            break;
        case 0x71: // BIT 6, C
            bit("0x71 - BIT 6, C", 1 << 6, Registers.C);
            break;
        case 0x72: // BIT 6, D
            bit("0x72 - BIT 6, D", 1 << 6, Registers.D);
            break;
        case 0x73: // BIT 6, E
            bit("0x73 - BIT 6, E", 1 << 6, Registers.E);
            break;
        case 0x74: // BIT 6, H
            bit("0x74 - BIT 6, H", 1 << 6, Registers.H);
            break;
        case 0x75: // BIT 6, L
            bit("0x75 - BIT 6, L", 1 << 6, Registers.L);
            break;
        case 0x76: // BIT 6, (HL)
            bit("0x76 - BIT 6, (HL)", 1 << 6, mmu.read8(Registers.HL));
            break;
        case 0x77: // BIT 6, A
            bit("0x77 - BIT 6, A", 1 << 6, Registers.A);
            break;
        case 0x78: // BIT 7, B
            bit("0x78 - BIT 7, B", 1 << 7, Registers.B);
            break;
        case 0x79: // BIT 7, C
            bit("0x79 - BIT 7, C", 1 << 7, Registers.C);
            break;
        case 0x7A: // BIT 7, D
            bit("0x7A - BIT 7, D", 1 << 7, Registers.D);
            break;
        case 0x7B: // BIT 7, E
            bit("0x7B - BIT 7, E", 1 << 7, Registers.E);
            break;
        case 0x7C: // BIT 7, H
            bit("0x7C - BIT 7, H", 1 << 7, Registers.H);
            break;
        case 0x7D: // BIT 7, L
            bit("0x7D - BIT 7, L", 1 << 7, Registers.L);
            break;
        case 0x7E: // BIT 7, (HL)
            bit("0x7E - BIT 7, (HL)", 1 << 7, mmu.read8(Registers.HL));
            break;
        case 0x7F: // BIT 7, A
            bit("0x7F - BIT 7, A", 1 << 7, Registers.A);
            break;
        case 0x80: // RES 0, B
            res("0x80 - RES 0, B", 1 << 0, &Registers.B);
            break;
        case 0x81: // RES 0, C
            res("0x81 - RES 0, C", 1 << 0, &Registers.C);
            break;
        case 0x82: // RES 0, D
            res("0x82 - RES 0, D", 1 << 0, &Registers.D);
            break;
        case 0x83: // RES 0, E
            res("0x83 - RES 0, E", 1 << 0, &Registers.E);
            break;
        case 0x84: // RES 0, H
            res("0x84 - RES 0, H", 1 << 0, &Registers.H);
            break;
        case 0x85: // RES 0, L
            res("0x85 - RES 0, L", 1 << 0, &Registers.L);
            break;
        case 0x86: // RES 0, (HL)
            modifyHL([this](uint8_t& value) { res("0x86 - RES 0, (HL)", 1 << 0, &value); });
            break;
        case 0x87: // RES 0, A
            res("0x87 - RES 0, A", 1 << 0, &Registers.A);
            break;
        case 0x88: // RES 1, B
            res("0x88 - RES 1, B", 1 << 1, &Registers.B);
            break;
        case 0x89: // RES 1, C
            res("0x89 - RES 1, C", 1 << 1, &Registers.C);
            break;
        case 0x8A: // RES 1, D
            res("0x8A - RES 1, D", 1 << 1, &Registers.D);
            break;
        case 0x8B: // RES 1, E
            res("0x8B - RES 1, E", 1 << 1, &Registers.E);
            break;
        case 0x8C: // RES 1, H
            res("0x8C - RES 1, H", 1 << 1, &Registers.H);
            break;
        case 0x8D: // RES 1, L
            res("0x8D - RES 1, L", 1 << 1, &Registers.L);
            break;
        case 0x8E: // RES 1, (HL)
            modifyHL([this](uint8_t& value) { res("0x8E - RES 1, (HL)", 1 << 1, &value); });
            break;
        case 0x8F: // RES 1, A
            res("0x8F - RES 1, A", 1 << 1, &Registers.A);
            break;
        case 0x90: // RES 2, B
            res("0x90 - RES 2, B", 1 << 2, &Registers.B);
            break;
        case 0x91: // RES 2, C
            res("0x91 - RES 2, C", 1 << 2, &Registers.C);
            break;
        case 0x92: // RES 2, D
            res("0x92 - RES 2, D", 1 << 2, &Registers.D);
            break;
        case 0x93: // RES 2, E
            res("0x93 - RES 2, E", 1 << 2, &Registers.E);
            break;
        case 0x94: // RES 2, H
            res("0x94 - RES 2, H", 1 << 2, &Registers.H);
            break;
        case 0x95: // RES 2, L
            res("0x95 - RES 2, L", 1 << 2, &Registers.L);
            break;
        case 0x96: // RES 2, (HL)
            modifyHL([this](uint8_t& value) { res("0x96 - RES 2, (HL)", 1 << 2, &value); });
            break;
        case 0x97: // RES 2, A
            res("0x97 - RES 2, A", 1 << 2, &Registers.A);
            break;
        case 0x98: // RES 3, B
            res("0x98 - RES 3, B", 1 << 3, &Registers.B);
            break;
        case 0x99: // RES 3, C
            res("0x99 - RES 3, C", 1 << 3, &Registers.C);
            break;
        case 0x9A: // RES 3, D
            res("0x9A - RES 3, D", 1 << 3, &Registers.D);
            break;
        case 0x9B: // RES 3, E
            res("0x9B - RES 3, E", 1 << 3, &Registers.E);
            break;
        case 0x9C: // RES 3, H
            res("0x9C - RES 3, H", 1 << 3, &Registers.H);
            break;
        case 0x9D: // RES 3, L
            res("0x9D - RES 3, L", 1 << 3, &Registers.L);
            break;
        case 0x9E: // RES 3, (HL)
            modifyHL([this](uint8_t& value) { res("0x9E - RES 3, (HL)", 1 << 3, &value); });
            break;
        case 0x9F: // RES 3, A
            res("0x9F - RES 3, A", 1 << 3, &Registers.A);
            break;
        case 0xA0: // RES 4, B
            res("0xA0 - RES 4, B", 1 << 4, &Registers.B);
            break;
        case 0xA1: // RES 4, C
            res("0xA1 - RES 4, C", 1 << 4, &Registers.C);
            break;
        case 0xA2: // RES 4, D
            res("0xA2 - RES 4, D", 1 << 4, &Registers.D);
            break;
        case 0xA3: // RES 4, E
            res("0xA3 - RES 4, E", 1 << 4, &Registers.E);
            break;
        case 0xA4: // RES 4, H
            res("0xA4 - RES 4, H", 1 << 4, &Registers.H);
            break;
        case 0xA5: // RES 4, L
            res("0xA5 - RES 4, L", 1 << 4, &Registers.L);
            break;
        case 0xA6: // RES 4, (HL)
            modifyHL([this](uint8_t& value) { res("0xA6 - RES 4, (HL)", 1 << 4, &value); });
            break;
        case 0xa7: // RES 4, A
            res("0xa7 - RES 4, A", 1 << 4, &Registers.A);
            break;
        case 0xa8: // RES 5, B
            res("0xa8 - RES 5, B", 1 << 5, &Registers.B);
            break;
        case 0xa9: // RES 5, C
            res("0xa9 - RES 5, C", 1 << 5, &Registers.C);
            break;
        case 0xAA: // RES 5, D
            res("0xAA - RES 5, D", 1 << 5, &Registers.D);
            break;
        case 0xAB: // RES 5, E
            res("0xAB - RES 5, E", 1 << 5, &Registers.E);
            break;
        case 0xAC: // RES 5, H
            res("0xAC - RES 5, H", 1 << 5, &Registers.H);
            break;
        case 0xAD: // RES 5, L
            res("0xAD - RES 5, L", 1 << 5, &Registers.L);
            break;
        case 0xAE: // RES 5, (HL)
            modifyHL([this](uint8_t& value) { res("0xAE - RES 5, (HL)", 1 << 5, &value); });
            break;
        case 0xAF: // RES 5, A
            res("0xAF - RES 5, A", 1 << 5, &Registers.A);
            break;
        case 0xB0: // RES 6, B
            res("0xB0 - RES 6, B", 1 << 6, &Registers.B);
            break;
        case 0xB1: // RES 6, C
            res("0xB1 - RES 6, C", 1 << 6, &Registers.C);
            break;
        case 0xB2: // RES 6, D
            res("0xB2 - RES 6, D", 1 << 6, &Registers.D);
            break;
        case 0xB3: // RES 6, E
            res("0xB3 - RES 6, E", 1 << 6, &Registers.E);
            break;
        case 0xB4: // RES 6, H
            res("0xB4 - RES 6, H", 1 << 6, &Registers.H);
            break;
        case 0xB5: // RES 6, L
            res("0xB5 - RES 6, L", 1 << 6, &Registers.L);
            break;
        case 0xB6: // RES 6, (HL)
            modifyHL([this](uint8_t& value) { res("0xB6 - RES 6, (HL)", 1 << 6, &value); });
            break;
        case 0xb7: // RES 6, A
            res("0xb7 - RES 6, A", 1 << 6, &Registers.A);
            break;
        case 0xb8: // RES 7, B
            res("0xb8 - RES 7, B", 1 << 7, &Registers.B);
            break;
        case 0xb9: // RES 7, C
            res("0xb9 - RES 7, C", 1 << 7, &Registers.C);
            break;
        case 0xBA: // RES 7, D
            res("0xBA - RES 7, D", 1 << 7, &Registers.D);
            break;
        case 0xBB: // RES 7, E
            res("0xBB - RES 7, E", 1 << 7, &Registers.E);
            break;
        case 0xBC: // RES 7, H
            res("0xBC - RES 7, H", 1 << 7, &Registers.H);
            break;
        case 0xBD: // RES 7, L
            res("0xBD - RES 7, L", 1 << 7, &Registers.L);
            break;
        case 0xBE: // RES 7, (HL)
            modifyHL([this](uint8_t& value) { res("0xBE - RES 7, (HL)", 1 << 7, &value); });
            break;
        case 0xBF: // RES 7, A
            res("0xBF - RES 7, A", 1 << 7, &Registers.A);
            break;
        case 0xC0: // SET 0, B
            set("0xC0 - SET 0, B", 1 << 0, &Registers.B);
            break;
        case 0xC1: // SET 0, C
            set("0xC1 - SET 0, C", 1 << 0, &Registers.C);
            break;
        case 0xC2: // SET 0, D
            set("0xC2 - SET 0, D", 1 << 0, &Registers.D);
            break;
        case 0xC3: // SET 0, E
            set("0xC3 - SET 0, E", 1 << 0, &Registers.E);
            break;
        case 0xC4: // SET 0, H
            set("0xC4 - SET 0, H", 1 << 0, &Registers.H);
            break;
        case 0xC5: // SET 0, L
            set("0xC5 - SET 0, L", 1 << 0, &Registers.L);
            break;
        case 0xC6: // SET 0, (HL)
            modifyHL([this](uint8_t& value) { set("0xC6 - SET 0, (HL)", 1 << 0, &value); });
            break;
        case 0xc7: // SET 0, A
            set("0xc7 - SET 0, A", 1 << 0, &Registers.A);
            break;
        case 0xc8: // SET 1, B
            set("0xc8 - SET 1, B", 1 << 1, &Registers.B);
            break;
        case 0xc9: // SET 1, C
            set("0xc9 - SET 1, C", 1 << 1, &Registers.C);
            break;
        case 0xCA: // SET 1, D
            set("0xCA - SET 1, D", 1 << 1, &Registers.D);
            break;
        case 0xCB: // SET 1, E
            set("0xCB - SET 1, E", 1 << 1, &Registers.E);
            break;
        case 0xCC: // SET 1, H
            set("0xCC - SET 1, H", 1 << 1, &Registers.H);
            break;
        case 0xCD: // SET 1, L
            set("0xCD - SET 1, L", 1 << 1, &Registers.L);
            break;
        case 0xCE: // SET 1, (HL)
            modifyHL([this](uint8_t& value) { set("0xCE - SET 1, (HL)", 1 << 1, &value); });
            break;
        case 0xCF: // SET 1, A
            set("0xCF - SET 1, A", 1 << 1, &Registers.A);
            break;
        case 0xD0: // SET 2, B
            set("0xD0 - SET 2, B", 1 << 2, &Registers.B);
            break;
        case 0xD1: // SET 2, C
            set("0xD1 - SET 2, C", 1 << 2, &Registers.C);
            break;
        case 0xD2: // SET 2, D
            set("0xD2 - SET 2, D", 1 << 2, &Registers.D);
            break;
        case 0xD3: // SET 2, E
            set("0xD3 - SET 2, E", 1 << 2, &Registers.E);
            break;
        case 0xD4: // SET 2, H
            set("0xD4 - SET 2, H", 1 << 2, &Registers.H);
            break;
        case 0xD5: // SET 2, L
            set("0xD5 - SET 2, L", 1 << 2, &Registers.L);
            break;
        case 0xD6: // SET 2, (HL)
            modifyHL([this](uint8_t& value) { set("0xD6 - SET 2, (HL)", 1 << 2, &value); });
            break;
        case 0xD7: // SET 2, A
            set("0xD7 - SET 2, A", 1 << 2, &Registers.A);
            break;
        case 0xD8: // SET 3, B
            set("0xD8 - SET 3, B", 1 << 3, &Registers.B);
            break;
        case 0xD9: // SET 3, C
            set("0xD9 - SET 3, C", 1 << 3, &Registers.C);
            break;
        case 0xDA: // SET 3, D
            set("0xDA - SET 3, D", 1 << 3, &Registers.D);
            break;
        case 0xDB: // SET 3, E
            set("0xDB - SET 3, E", 1 << 3, &Registers.E);
            break;
        case 0xDC: // SET 3, H
            set("0xDC - SET 3, H", 1 << 3, &Registers.H);
            break;
        case 0xDD: // SET 3, L
            set("0xDD - SET 3, L", 1 << 3, &Registers.L);
            break;
        case 0xDE: // SET 3, (HL)
            modifyHL([this](uint8_t& value) { set("0xDE - SET 3, (HL)", 1 << 3, &value); });
            break;
        case 0xDF: // SET 3, A
            set("0xDF - SET 3, A", 1 << 3, &Registers.A);
            break;
        case 0xE0: // SET 4, B
            set("0xE0 - SET 4, B", 1 << 4, &Registers.B);
            break;
        case 0xE1: // SET 4, C
            set("0xE1 - SET 4, C", 1 << 4, &Registers.C);
            break;
        case 0xE2: // SET 4, D
            set("0xE2 - SET 4, D", 1 << 4, &Registers.D);
            break;
        case 0xE3: // SET 4, E
            set("0xE3 - SET 4, E", 1 << 4, &Registers.E);
            break;
        case 0xE4: // SET 4, H
            set("0xE4 - SET 4, H", 1 << 4, &Registers.H);
            break;
        case 0xE5: // SET 4, L
            set("0xE5 - SET 4, L", 1 << 4, &Registers.L);
            break;
        case 0xE6: // SET 4, (HL)
            modifyHL([this](uint8_t& value) { set("0xE6 - SET 4, (HL)", 1 << 4, &value); });
            break;
        case 0xE7: // SET 4, A
            set("0xE7 - SET 4, A", 1 << 4, &Registers.A);
            break;
        case 0xE8: // SET 5, B
            set("0xE8 - SET 5, B", 1 << 5, &Registers.B);
            break;
        case 0xE9: // SET 5, C
            set("0xE9 - SET 5, C", 1 << 5, &Registers.C);
            break;
        case 0xEA: // SET 5, D
            set("0xEA - SET 5, D", 1 << 5, &Registers.D);
            break;
        case 0xEB: // SET 5, E
            set("0xEB - SET 5, E", 1 << 5, &Registers.E);
            break;
        case 0xEC: // SET 5, H
            set("0xEC - SET 5, H", 1 << 5, &Registers.H);
            break;
        case 0xED: // SET 5, L
            set("0xED - SET 5, L", 1 << 5, &Registers.L);
            break;
        case 0xEE: // SET 5, (HL)
            modifyHL([this](uint8_t& value) { set("0xEE - SET 5, (HL)", 1 << 5, &value); });
            break;
        case 0xEF: // SET 5, A
            set("0xEF - SET 5, A", 1 << 5, &Registers.A);
            break;
        case 0xF0: // SET 6, B
            set("0xF0 - SET 6, B", 1 << 6, &Registers.B);
            break;
        case 0xF1: // SET 6, C
            set("0xF1 - SET 6, C", 1 << 6, &Registers.C);
            break;
        case 0xF2: // SET 6, D
            set("0xF2 - SET 6, D", 1 << 6, &Registers.D);
            break;
        case 0xF3: // SET 6, E
            set("0xF3 - SET 6, E", 1 << 6, &Registers.E);
            break;
        case 0xF4: // SET 6, H
            set("0xF4 - SET 6, H", 1 << 6, &Registers.H);
            break;
        case 0xF5: // SET 6, L
            set("0xF5 - SET 6, L", 1 << 6, &Registers.L);
            break;
        case 0xF6: // SET 6, (HL)
            modifyHL([this](uint8_t& value) { set("0xF6 - SET 6, (HL)", 1 << 6, &value); });
            break;
        case 0xF7: // SET 6, A
            set("0xF7 - SET 6, A", 1 << 6, &Registers.A);
            break;
        case 0xF8: // SET 7, B
            set("0xF8 - SET 7, B", 1 << 7, &Registers.B);
            break;
        case 0xF9: // SET 7, C
            set("0xF9 - SET 7, C", 1 << 7, &Registers.C);
            break;
        case 0xFA: // SET 7, D
            set("0xFA - SET 7, D", 1 << 7, &Registers.D);
            break;
        case 0xFB: // SET 7, E
            set("0xFB - SET 7, E", 1 << 7, &Registers.E);
            break;
        case 0xFC: // SET 7, H
            set("0xFC - SET 7, H", 1 << 7, &Registers.H);
            break;
        case 0xFD: // SET 7, L
            set("0xFD - SET 7, L", 1 << 7, &Registers.L);
            break;
        case 0xFE: // SET 7, (HL)
            modifyHL([this](uint8_t& value) { set("0xFE - SET 7, (HL)", 1 << 7, &value); });
            break;
        case 0xFF: // SET 7, A
            set("0xFF - SET 7, A", 1 << 7, &Registers.A);
            break;
        default:
            logger.log("[DMG-CPU] ==================================================");
            logger.log("[DMG-CPU] [EXTENDED] Unsupported opcode: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG-CPU] ==================================================");
            return;
            break;
    }
}
