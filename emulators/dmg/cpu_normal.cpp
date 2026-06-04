#include "cpu.hpp"

void DMG_CPU::executeInstruction8bit(bool ROMFileLoaded, uint8_t opcode) {
    if (!ROMFileLoaded) return;

    switch (opcode) {
        case 0x00: // NOP
            logCall(true, "0x00 NOP");
            break;
        case 0x01: // LD BC, nn
            logCall(false, "0x01 LD BC, nn");
            Registers.BC = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x02: // LD (BC), A
            logCall(true, "0x02 LD (BC), A");
            mmu.write8(Registers.BC, Registers.A);
            break;
        case 0x03: // INC BC
            logCall(true, "0x03 INC BC");
            Registers.BC++;
            mmu.tick(4);
            break;
        case 0x04: // INC B
            inc("0x04 - INC B", &Registers.B);
            break;
        case 0x05: // DEC B
            dec("0x05 - DEC B", &Registers.B);
            break;
        case 0x06: // LD B, n
            logCall(true, "0x06 LD B, n");
            Registers.B = mmu.read8(Registers.PC++);
            break;
        case 0x07: // RLCA
            rlc("0x07 - RLCA", &Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x08: // LD (nn), SP
            logCall(true, "0x08 LD (nn), SP");
            mmu.write16(mmu.read16(Registers.PC), Registers.SP);
            Registers.PC += 2;
            break;
        case 0x09: // ADD HL, BC
            add("0x09 - ADD HL, BC", &Registers.HL, Registers.BC);
            mmu.tick(4);
            break;
        case 0x0A: // LD A, (BC)
            logCall(true, "0x0A LD A, (BC)");
            Registers.A = mmu.read8(Registers.BC);
            break;
        case 0x0B: // DEC BC
            logCall(true, "0x0B DEC BC");
            Registers.BC--;
            mmu.tick(4);
            break;
        case 0x0C: // INC C
            inc("0x0C - INC C", &Registers.C);
            break;
        case 0x0D: // DEC C
            logCall(true, "0x0D DEC C");
            dec("0x0D - DEC C", &Registers.C);
            break;
        case 0x0E: // LD C, n
            logCall(true, "0x0E LD C, n");
            Registers.C = mmu.read8(Registers.PC++);
            break;
        case 0x0F: // RRCA
            rrc("0x0F - RRCA", &Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x10: // STOP
            logCall(true, "0x10 STOP");
            break;
        case 0x11: // LD DE, nn
            logCall(true, "0x11 LD DE, nn");
            Registers.DE = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x12: // LD (DE), A
            logCall(true, "0x12 LD (DE), A");
            mmu.write8(Registers.DE, Registers.A);
            break;
        case 0x13: // INC DE
            logCall(true, "0x13 INC DE");
            Registers.DE++;
            mmu.tick(4);
            break;
        case 0x14: // INC D
            inc("0x14 - INC D", &Registers.D);
            break;
        case 0x15: // DEC D
            dec("0x15 - DEC D", &Registers.D);
            break;
        case 0x16: // LD D, n
            logCall(true, "0x16 LD D, n");
            Registers.D = mmu.read8(Registers.PC++);
            break;
        case 0x17: // RLA
            rl("0x17 - RLA", &Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x18: // JR nn
            {
                logCall(true, "0x18 JR nn");
                int8_t offset = (int8_t)mmu.read8(Registers.PC++);
                Registers.PC += offset;
                mmu.tick(4);
            }
            break;
        case 0x19: // ADD HL, DE
            add("0x19 - ADD HL, DE", &Registers.HL, Registers.DE);
            mmu.tick(4);
            break;
        case 0x1A: // LD A, (DE)
            logCall(true, "0x1A LD A, (DE)");
            Registers.A = mmu.read8(Registers.DE);
            break;
        case 0x1B: // DEC DE
            logCall(true, "0x1B DEC DE");
            Registers.DE--;
            mmu.tick(4);
            break;
        case 0x1C: // INC E
            inc("0x1C - INC E", &Registers.E);
            break;
        case 0x1D: // DEC E
            dec("0x1D - DEC E", &Registers.E);
            break;
        case 0x1E: // LD E, n
            logCall(true, "0x1E LD E, n");
            Registers.E = mmu.read8(Registers.PC++);
            break;
        case 0x1F: // RRA
            rr("0x1F - RRA", &Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x20: // JR NZ, *
            jump_add("0x20 - JR NZ, *", !isFlagSet(FLAG_ZERO));
            break;
        case 0x21: // LD HL, nn
            logCall(true, "0x21 LD HL, nn");
            Registers.HL = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x22: // LD (HLI), A | LD (HL+), A | LDI (HL), A
            logCall(true, "0x22 LD (HLI), A | LD (HL+), A | LDI (HL), A");
            mmu.write8(Registers.HL++, Registers.A);
            break;
        case 0x23: // INC HL
            logCall(true, "0x23 INC HL");
            Registers.HL++;
            mmu.tick(4);
            break;
        case 0x24: // INC H
            inc("0x24 - INC H", &Registers.H);
            break;
        case 0x25: // DEC H
            dec("0x25 - DEC H", &Registers.H);
            break;
        case 0x26: // LD H, n
            logCall(true, "LD H, n");
            Registers.H = mmu.read8(Registers.PC++);
            break;
        case 0x27: // DAA
            {
                logCall(true, "0x27 DAA");
                uint16_t value = Registers.A;
                if (isFlagSet(FLAG_SUBTRACT)) {
                    if (isFlagSet(FLAG_CARRY))
                        value -= 0x60;
                    if (isFlagSet(FLAG_HALF_CARRY))
                        value -= 0x6;
                }
                else {
                    if (isFlagSet(FLAG_CARRY) || value > 0x99) {
                        value += 0x60;
                        setFlag(FLAG_CARRY, true);
                    }
                    if (isFlagSet(FLAG_HALF_CARRY) || (value & 0xF) > 0x9)
                        value += 0x6;
                }
                Registers.A = (uint8_t)(value & 0xFF);
                setFlag(FLAG_ZERO, !Registers.A);
                setFlag(FLAG_HALF_CARRY, false);
            }
            break;
        case 0x28: // JR Z, *
            jump_add("0x28 - JR Z, *", isFlagSet(FLAG_ZERO));
            break;
        case 0x29: // ADD HL, HL
            logCall(true, "0x29 ADD HL, HL");
            add("0x29 - ADD HL, HL", &Registers.HL, Registers.HL);
            mmu.tick(4);
            break;
        case 0x2A: // LD A, (HL+)
            logCall(true, "0x2A LD A, (HL+)");
            Registers.A = mmu.read8(Registers.HL++);
            break;
        case 0x2B: // DEC HL
            logCall(true, "0x2B DEC HL");
            Registers.HL--;
            mmu.tick(4);
            break;
        case 0x2C: // INC L
            inc("0x2C - INC L", &Registers.L);
            break;
        case 0x2D: // DEC L
            dec("0x2D - DEC L", &Registers.L);
            break;
        case 0x2E: // LD L, n
            logCall(true, "0x2E LD L, n");
            Registers.L = mmu.read8(Registers.PC++);
            break;
        case 0x2F: // CPL
            logCall(true, "0x2F CPL");
            Registers.A = ~Registers.A;
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, true);
            break;
        case 0x30: // JR NC, *
            jump_add("0x30 - JR NC, *", !isFlagSet(FLAG_CARRY));
            break;
        case 0x31: // LD SP, nn
            logCall(true, "0x31 LD SP, nn");
            Registers.SP = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x32: // LD (HLD), A | LD (HL-), A | LDD (HL), A
            logCall(true, "0x32 LD (HLD), A | LD (HL-), A | LDD (HL), A");
            mmu.write8(Registers.HL--, Registers.A);
            break;
        case 0x33: // INC SP
            logCall(true, "0x33 INC SP");
            Registers.SP++;
            mmu.tick(4);
            break;
        case 0x35: // DEC (HL)
            {
                uint8_t tmp_val = mmu.read8(Registers.HL);
                dec("0x35 - DEC (HL)", &tmp_val);
                mmu.write16(Registers.HL, tmp_val);
            }
            break;
        case 0x34: // INC (HL)
            {
                uint8_t tmp_val = mmu.read8(Registers.HL);
                inc("0x34 - INC (HL)", &tmp_val);
                mmu.write16(Registers.HL, tmp_val);
            }
            break;
        case 0x36: // LD (HL), n
            logCall(true, "0x36 LD (HL), n");
            mmu.write16(Registers.HL, mmu.read16(Registers.PC++));
            break;
        case 0x37: // SCF
            logCall(true, "0x37 SCF");
            setFlag(FLAG_CARRY, true);
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
            break;
        case 0x38: // JR C, *
            jump_add("0x38 - JR C, *", isFlagSet(FLAG_CARRY));
            break;
        case 0x39: // ADD HL, SP
            add("0x39 - ADD HL, SP", &Registers.HL, Registers.SP);
            mmu.tick(4);
            break;
        case 0x3A: // LD A, (HL-)
            logCall(true, "0x3A LD A, (HL-)");
            Registers.A = mmu.read8(Registers.HL--);
            break;
        case 0x3B: // DEC SP
            logCall(true, "0x3B DEC SP");
            Registers.SP--;
            mmu.tick(4);
            break;
        case 0x3C: // INC A
            inc("0x3C - INC A", &Registers.A);
            break;
        case 0x3D: // DEC A
            dec("0x3D - DEC A", &Registers.A);
            break;
        case 0x3E: // LD A, n
            logCall(true, "0x3E LD A, n");
            Registers.A = mmu.read8(Registers.PC++);
            break;
        case 0x3F: // CCF
            logCall(true, "0x3F CCF");
            setFlag(FLAG_CARRY, !isFlagSet(FLAG_CARRY));
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
            break;
        case 0x40: // LD B, B
            logCall(true, "0x40 LD B, B");
            break;
        case 0x41: // LD B, C
            logCall(true, "0x41 LD B, C");
            Registers.B = Registers.C;
            break;
        case 0x42: // LD B, D
            logCall(true, "0x42 LD B, D");
            Registers.B = Registers.D;
            break;
        case 0x43: // LD B, E
            logCall(true, "0x43 LD B, E");
            Registers.B = Registers.E;
            break;
        case 0x44: // LD B, H
            logCall(true, "0x44 LD B, H");
            Registers.B = Registers.H;
            break;
        case 0x45: // LD B, L
            logCall(true, "0x45 LD B, L");
            Registers.B = Registers.L;
            break;
        case 0x46: // LD B, (HL)
            logCall(true, "0x46 LD B, (HL)");
            Registers.B = mmu.read8(Registers.HL);
            break;
        case 0x47: // LD B, A
            logCall(true, "0x47 LD B, A");
            Registers.B = Registers.A;
            break;
        case 0x48: // LD C, B
            logCall(true, "0x48 LD C, B");
            Registers.C = Registers.B;
            break;
        case 0x49: // LD C, C
            logCall(true, "0x49 LD C, C");
            break;
        case 0x4A: // LD C, D
            logCall(true, "0x4A LD C, D");
            Registers.C = Registers.D;
            break;
        case 0x4B: // LD C, E
            logCall(true, "0x4B LD C, E");
            Registers.C = Registers.E;
            break;
        case 0x4C: // LD C, H
            logCall(true, "0x4C LD C, H");
            Registers.C = Registers.H;
            break;
        case 0x4D: // LD C, L
            logCall(true, "0x4D LD C, L");
            Registers.C = Registers.L;
            break;
        case 0x4E: // LD C, (HL)
            logCall(true, "0x4E LD C, (HL)");
            Registers.C = mmu.read8(Registers.HL);
            break;
        case 0x4F: // LD C, A
            logCall(true, "0x4F LD C, A");
            Registers.C = Registers.A;
            break;
        case 0x50: // LD D, B
            logCall(true, "0x50 LD D, B");
            Registers.D = Registers.B;
            break;
        case 0x51: // LD D, C
            logCall(true, "0x51 LD D, C");
            Registers.D = Registers.C;
            break;
        case 0x52: // LD D, D
            logCall(true, "0x52 LD D, D");
            break;
        case 0x53: // LD D, E
            logCall(true, "0x53 LD D, E");
            Registers.D = Registers.E;
            break;
        case 0x54: // LD D, H
            logCall(true, "0x54 LD D, H");
            Registers.D = Registers.H;
            break;
        case 0x55: // LD D, L
            logCall(true, "0x55 LD D, L");
            Registers.D = Registers.L;
            break;
        case 0x56: // LD D, (HL)
            logCall(true, "0x56 LD D, (HL)");
            Registers.D = mmu.read8(Registers.HL);
            break;
        case 0x57: // LD D, A
            logCall(true, "0x57 LD D, A");
            Registers.D = Registers.A;
            break;
        case 0x58: // LD E, B
            logCall(true, "0x58 LD E, B");
            Registers.E = Registers.B;
            break;
        case 0x59: // LD E, C
            logCall(true, "0x59 LD E, C");
            Registers.E = Registers.C;
            break;
        case 0x5A: // LD E, D
            logCall(true, "0x5A LD E, D");
            Registers.E = Registers.D;
            break;
        case 0x5B: // LD E, E
            logCall(true, "0x5B LD E, E");
            break;
        case 0x5C: // LD E, H
            logCall(true, "0x5C LD E, H");
            Registers.E = Registers.H;
            break;
        case 0x5D: // LD E, L
            logCall(true, "0x5D LD E, L");
            Registers.E = Registers.L;
            break;
        case 0x5E: // LD E, (HL)
            logCall(true, "0x5E LD E, (HL)");
            Registers.E = mmu.read8(Registers.HL);
            break;
        case 0x5F: // LD E, A
            logCall(true, "0x5F LD E, A");
            Registers.E = Registers.A;
            break;
        case 0x60: // LD H, B
            logCall(true, "0x60 LD H, B");
            Registers.H = Registers.B;
            break;
        case 0x61: // LD H, C
            logCall(true, "0x61 LD H, C");
            Registers.H = Registers.C;
            break;
        case 0x62: // LD H, D
            logCall(true, "0x62 LD H, D");
            Registers.H = Registers.D;
            break;
        case 0x63: // LD H, E
            logCall(true, "0x63 LD H, E");
            Registers.H = Registers.E;
            break;
        case 0x64: // LD H, H
            logCall(true, "0x64 LD H, H");
            break;
        case 0x65: // LD H, L
            logCall(true, "0x65 LD H, L");
            Registers.H = Registers.L;
            break;
        case 0x66: // LD H, (HL)
            logCall(true, "0x66 LD H, (HL)");
            Registers.H = mmu.read8(Registers.HL);
            break;
        case 0x67: // LD H, A
            logCall(true, "0x67 LD H, A");
            Registers.H = Registers.A;
            break;
        case 0x68: // LD L, B
            logCall(true, "0x68 LD L, B");
            Registers.L = Registers.B;
            break;
        case 0x69: // LD L, C
            logCall(true, "0x69 LD L, C");
            Registers.L = Registers.C;
            break;
        case 0x6A: // LD L, D
            logCall(true, "0x6A LD L, D");
            Registers.L = Registers.D;
            break;
        case 0x6B: // LD L, E
            logCall(true, "0x6B LD L, E");
            Registers.L = Registers.E;
            break;
        case 0x6C: // LD L, H
            logCall(true, "0x6C LD L, H");
            Registers.L = Registers.H;
            break;
        case 0x6D: // LD L, L
            logCall(true, "break LD L, L");
            break;
        case 0x6E: // LD L, (HL)
            logCall(true, "0x6E LD L, (HL)");
            Registers.L = mmu.read8(Registers.HL);
            break;
        case 0x6F: // LD L, A
            logCall(true, "0x6F LD L, A");
            Registers.L = Registers.A;
            break;
        case 0x70: // LD (HL), B
            logCall(true, "0x70 LD (HL), B");
            mmu.write8(Registers.HL, Registers.B);
            break;
        case 0x71: // LD (HL), C
            logCall(true, "0x71 LD (HL), C");
            mmu.write8(Registers.HL, Registers.C);
            break;
        case 0x72: // LD (HL), D
            logCall(true, "0x72 LD (HL), D");
            mmu.write8(Registers.HL, Registers.D);
            break;
        case 0x73: // LD (HL), E
            logCall(true, "0x73 LD (HL), E");
            mmu.write8(Registers.HL, Registers.E);
            break;
        case 0x74: // LD (HL), H
            logCall(true, "0x74 LD (HL), H");
            mmu.write8(Registers.HL, Registers.H);
            break;
        case 0x75: // LD (HL), L
            logCall(true, "0x75 LD (HL), L");
            mmu.write8(Registers.HL, Registers.L);
            break;
        case 0x76: // HALT
            logCall(true, "0x76 HALT");
            if (!interrupts.getIME() && (mmu.memory[0xFF0F] & mmu.memory[0xFFFF] & 0x1F))
                mmu.is_halted = false;
            else
                mmu.is_halted = true;
            break;
        case 0x77: // LD (HL), A
            logCall(true, "0x77 LD (HL), A");
            mmu.write8(Registers.HL, Registers.A);
            break;
        case 0x78: // LD A, B
            logCall(true, "0x78 LD A, B");
            Registers.A = Registers.B;
            break;
        case 0x79: // LD A, C
            logCall(true, "0x79 LD A, C");
            Registers.A = Registers.C;
            break;
        case 0x7A: // LD A, D
            logCall(true, "0x7A LD A, D");
            Registers.A = Registers.D;
            break;
        case 0x7B: // LD A, E
            logCall(true, "0x7B LD A, E");
            Registers.A = Registers.E;
            break;
        case 0x7C: // LD A, H
            logCall(true, "0x7C LD A, H");
            Registers.A = Registers.H;
            break;
        case 0x7D: // LD A, L
            logCall(true, "0x7D LD A, L");
            Registers.A = Registers.L;
            break;
        case 0x7E: // LD A, (HL)
            logCall(true, "0x7E LD A, (HL)");
            Registers.A = mmu.read8(Registers.HL);
            break;
        case 0x7F: // LD A, A
            logCall(true, "0x7F LD A, A");
            break;
        case 0x80: // ADD A, B
            add("0x80 - ADD A, B", &Registers.A, Registers.B);
            break;
        case 0x81: // ADD A, C
            add("0x81 - ADD A, C", &Registers.A, Registers.C);
            break;
        case 0x82: // ADD A, D
            add("0x82 - ADD A, D", &Registers.A, Registers.D);
            break;
        case 0x83: // ADD A, E
            add("0x83 - ADD A, E", &Registers.A, Registers.E);
            break;
        case 0x84: // ADD A, H
            add("0x84 - ADD A, H", &Registers.A, Registers.H);
            break;
        case 0x85: // ADD A, L
            add("0x85 - ADD A, L", &Registers.A, Registers.L);
            break;
        case 0x86: // ADD A, (HL)
            add("0x86 - ADD A, (HL)", &Registers.A, mmu.read8(Registers.HL));
            break;
        case 0x87: // ADD A, A
            add("0x87 - ADD A, A", &Registers.A, Registers.A);
            break;
        case 0x88: // ADC A, B
            adc("0x88 - ADC A, B", Registers.B);
            break;
        case 0x89: // ADC A, C
            adc("0x89 - ADC A, C", Registers.C);
            break;
        case 0x8A: // ADC A, D
            adc("0x8A - ADC A, D", Registers.D);
            break;
        case 0x8B: // ADC A, E
            adc("0x8B - ADC A, E", Registers.E);
            break;
        case 0x8C: // ADC A, H
            adc("0x8C - ADC A, H", Registers.H);
            break;
        case 0x8D: // ADC A, L
            adc("0x8D - ADC A, L", Registers.L);
            break;
        case 0x8E: // ADC A, (HL)
            adc("0x8E - ADC A, (HL)", mmu.read8(Registers.HL));
            break;
        case 0x8F: // ADC A, A
            adc("0x8F - ADC A, A", Registers.A);
            break;
        case 0x90: // SUB B
            sub("0x90 - SUB B", Registers.B);
            break;
        case 0x91: // SUB C
            sub("0x91 - SUB C", Registers.C);
            break;
        case 0x92: // SUB D
            sub("0x92 - SUB D", Registers.D);
            break;
        case 0x93: // SUB E
            sub("0x93 - SUB E", Registers.E);
            break;
        case 0x94: // SUB H
            sub("0x94 - SUB H", Registers.H);
            break;
        case 0x95: // SUB L
            sub("0x95 - SUB L", Registers.L);
            break;
        case 0x96: // SUB (HL)
            sub("0x96 - SUB (HL)", mmu.read8(Registers.HL));
            break;
        case 0x97: // SUB A
            sub("0x97 - SUB A", Registers.A);
            break;
        case 0x98: // SBC A, B
            sbc("0x98 - SBC A, B", Registers.B);
            break;
        case 0x99: // SBC A, C
            sbc("0x99 - SBC A, C", Registers.C);
            break;
        case 0x9A: // SBC A, D
            sbc("0x9A - SBC A, D", Registers.D);
            break;
        case 0x9B: // SBC A, E
            sbc("0x9B - SBC A, E", Registers.E);
            break;
        case 0x9C: // SBC A, H
            sbc("0x9C - SBC A, H", Registers.H);
            break;
        case 0x9D: // SBC A, L
            sbc("0x9D - SBC A, L", Registers.L);
            break;
        case 0x9E: // SBC A, (HL)
            sbc("0x9E - SBC A, (HL)", mmu.read8(Registers.HL));
            break;
        case 0x9F: // SBC A, A
            sbc("0x9F - SBC A, A", Registers.A);
            break;
        case 0xA0: // AND B
            and_("0xA0 - AND B", Registers.B);
            break;
        case 0xA1: // AND C
            and_("0xA1 - AND C", Registers.C);
            break;
        case 0xA2: // AND D
            and_("0xA2 - AND D", Registers.D);
            break;
        case 0xA3: // AND E
            and_("0xA3 - AND E", Registers.E);
            break;
        case 0xA4: // AND H
            and_("0xA4 - AND H", Registers.H);
            break;
        case 0xA5: // AND l
            and_("0xA5 - AND l", Registers.L);
            break;
        case 0xA6: // AND (HL)
            and_("0xA6 - AND (HL)", mmu.read8(Registers.HL));
            break;
        case 0xA7: // AND A
            and_("0xA7 - AND A", Registers.A);
            break;
        case 0xA8: // XOR B
            xor_("0xA8 - XOR B", Registers.B);
            break;
        case 0xA9: // XOR C
            xor_("0xA9 - XOR C", Registers.C);
            break;
        case 0xAA: // XOR D
            xor_("0xAA - XOR D", Registers.D);
            break;
        case 0xAB: // XOR E
            xor_("0xAB - XOR E", Registers.E);
            break;
        case 0xAC: // XOR H
            xor_("0xAC - XOR H", Registers.H);
            break;
        case 0xAD: // XOR L
            xor_("0xAD - XOR L", Registers.L);
            break;
        case 0xAE: // XOR (HL)
            xor_("0xAE - XOR (HL)", mmu.read8(Registers.HL));
            break;
        case 0xAF: // XOR A
            xor_("0xAF - XOR A", Registers.A);
            break;
        case 0xB0: // OR B
            or_("0xB0 - OR B", Registers.B);
            break;
        case 0xB1: // OR C
            or_("0xB1 - OR C", Registers.C);
            break;
        case 0xB2: // OR D
            or_("0xB2 - OR D", Registers.D);
            break;
        case 0xB3: // OR E
            or_("0xB3 - OR E", Registers.E);
            break;
        case 0xB4: // OR H
            or_("0xB4 - OR H", Registers.H);
            break;
        case 0xB5: // OR L
            or_("0xB5 - OR L", Registers.L);
            break;
        case 0xB6: // OR (HL)
            or_("0xB6 - OR (HL)", mmu.read8(Registers.HL));
            break;
        case 0xB7: // OR A
            or_("0xB7 - OR A", Registers.A);
            break;
        case 0xB8: // CP B
            cp("0xB8 - CP B", Registers.B);
            break;
        case 0xB9: // CP C
            cp("0xB9 - CP C", Registers.C);
            break;
        case 0xBA: // CP D
            cp("0xBA - CP D", Registers.D);
            break;
        case 0xBB: // CP E
            cp("0xBB - CP E", Registers.E);
            break;
        case 0xBC: // CP H
            cp("0xBC - CP H", Registers.H);
            break;
        case 0xBD: // CP L
            cp("0xBD - CP L", Registers.L);
            break;
        case 0xBE: // CP (HL)
            cp("0xBE - CP (HL)", mmu.read8(Registers.HL));
            break;
        case 0xBF: // CP A
            cp("0xBF - CP A", Registers.A);
            break;
        case 0xC0: // RET NZ
            ret("0xC0 - RET NZ", !isFlagSet(FLAG_ZERO));
            break;
        case 0xC1: // POP BC
            Registers.BC = mmu.read_stack(&Registers.SP);
            break;
        case 0xC2: // JP NZ, nn
            jump("0xC2 - JP NZ, nn", !isFlagSet(FLAG_ZERO));
            break;
        case 0xC3: // JP nn
            Registers.PC = mmu.read16(Registers.PC);
            mmu.tick(4);
            break;
        case 0xC4: // CALL NZ, nn
            call("0xC4 - CALL NZ, nn", !isFlagSet(FLAG_ZERO));
            break;
        case 0xC5: // PUSH BC
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.BC);
            break;
        case 0xC6: // ADD A, n
            add("0xC6 - ADD A, n", &Registers.A, mmu.read8(Registers.PC++));
            break;
        case 0xC7: // RST $00
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0000;
            break;
        case 0xC8: // RET Z
            ret("0xC8 - RET Z", isFlagSet(FLAG_ZERO));
            break;
        case 0xC9: // RET
            Registers.PC = mmu.read_stack(&Registers.SP);
            mmu.tick(4);
            break;
        case 0xCA: // JP Z, nn
            jump("0xCA - JP Z, nn", isFlagSet(FLAG_ZERO));
            break;
        // handled in cpu
        //case 0xCB:
        //    executeInstruction16bit(ROMFileLoaded, mmu.read8(Registers.PC++));
        //    break;
        case 0xCC: // CALL Z, nn
            call("0xCC - CALL Z, nn", isFlagSet(FLAG_ZERO));
            break;
        case 0xCD: // CALL nn
            {
                logCall(true, "CALL nn");
                uint16_t operand = mmu.read16(Registers.PC);
                Registers.PC += 2;
                mmu.tick(4);
                mmu.write_stack(&Registers.SP, Registers.PC);
                Registers.PC = operand;
            } break;
        case 0xCE: // ADC A, n
            adc("0xCE - ADC A, n", mmu.read8(Registers.PC++));
            break;
        case 0xCF: // RST $08
            logCall(true, "0xCF RST $08");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0008;
            break;
        case 0xD0: // RET NC
            ret("0xD0 - RET NC", !isFlagSet(FLAG_CARRY));
            break;
        case 0xD1: // POP DE
            logCall(true, "0xD1 POP DE");
            Registers.DE = mmu.read_stack(&Registers.SP);
            break;
        case 0xD2: // JP NC, nn
            jump("0xD2 - JP NC, nn", !isFlagSet(FLAG_CARRY));
            break;
        case 0xD4: // CALL NC, nn
            call("0xD4 - CALL NC, nn", !isFlagSet(FLAG_CARRY));
            break;
        case 0xD5: // PUSH DE
            logCall(true, "0xD5 PUSH DE");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.DE);
            break;
        case 0xD6: // SUB n
            sub("0xD6 - SUB n", mmu.read8(Registers.PC++));
            break;
        case 0xD7: // RST $10
            logCall(true, "0xD7 RST $10");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0010;
            break;
        case 0xD8: // RET C
            ret("0xD8 - RET C", isFlagSet(FLAG_CARRY));
            break;
        case 0xD9: // RETI
            logCall(true, "0xD9 RETI");
            interrupts.setIME(true);
            Registers.PC = mmu.read_stack(&Registers.SP);
            mmu.tick(4);
            break;
        case 0xDA: // JP C, nn
            jump("0xDA - JP C, nn", isFlagSet(FLAG_CARRY));
            break;
        case 0xDC: // CALL C, nn
            call("0xDC - CALL C, nn", isFlagSet(FLAG_CARRY));
            break;
        case 0xDE: // SUB n
            sbc("0xDE - SUB n", mmu.read8(Registers.PC++));
            break;
        case 0xDF: // RST $18
            logCall(true, "0xDF RST $18");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0018;
            break;
        case 0xE0: // LD ($FF00+n), A
            logCall(true, "0xE0 LD ($FF00+n), A");
            mmu.write16(0xff00 + mmu.read8(Registers.PC++), Registers.A);
            break;
        case 0xE1: // POP HL
            logCall(true, "0xE1 POP HL");
            Registers.HL = mmu.read_stack(&Registers.SP);
            break;
        case 0xE2: // LD ($FF00+C), A
            logCall(true, "0xE2 LD ($FF00+C), A");
            mmu.write8(0xff00 + Registers.C, Registers.A);
            break;
        case 0xE5: // PUSH HL
            logCall(true, "0xE5 PUSH HL");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.HL);
            break;
        case 0xE6: // AND n
            and_("0xE6 - AND n", mmu.read8(Registers.PC++));
            break;
        case 0xE7: // RST $20
            logCall(true, "0xE7 RST $20");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0020;
            break;
        case 0xE8: // ADD SP, n
            add("0xE8 - ADD SP, n", &Registers.SP, (int8_t)mmu.read8(Registers.PC++));
            mmu.tick(4);
            break;
        case 0xE9: // JP HL
            logCall(true, "0xE9 JP HL");
            Registers.PC = Registers.HL;
            break;
        case 0xEA: // LD (nn), A
            logCall(true, "0xEA LD (nn), A");
            mmu.write16(mmu.read16(Registers.PC), Registers.A);
            Registers.PC += 2;
            break;
        case 0xEE: // XOR n
            xor_("0xEE - XOR n", mmu.read8(Registers.PC++));
            break;
        case 0xEF: // RST $28
            logCall(true, "0xEF RST $28");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0028;
            break;
        case 0xF0: // LD A, ($FF00+n)
            logCall(true, "0xF0 LD A, ($FF00+n)");
            Registers.A = mmu.read8(0xff00 + mmu.read8(Registers.PC++));
            break;
        case 0xF1: // POP AF
            logCall(true, "0xF1 POP AF");
            Registers.AF = mmu.read_stack(&Registers.SP);
            Registers.F &= 0xf0;  // Reset the 4 unused bits
            break;
        case 0xF2: // LD A, (C)
            logCall(true, "0xF2 LD A, (C)");
            Registers.A = mmu.read8(0xff00 + Registers.C);
            break;
        case 0xF3: // DI
            logCall(true, "0xF3 DI");
            interrupts.setIME(false);
            break;
        case 0xF5: // PUSH AF
            logCall(true, "0xF5 PUSH AF");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.AF);
            break;
        case 0xF6: // OR n
            or_("0xF6 - OR n", mmu.read8(Registers.PC++));
            break;
        case 0xF7: // RST $30
            logCall(true, "0xF7 RST $30");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0030;
            break;
        case 0xF8: // LDHL SP, n
            ldhl("0xF8 - LDHL SP, n", mmu.read8(Registers.PC++));
            mmu.tick(4);
            break;
        case 0xF9: // LD SP, HL
            logCall(true, "0xF9 LD SP, HL");
            Registers.SP = Registers.HL;
            mmu.tick(4);
            break;
        case 0xFA: // LD A, (nn)
            logCall(true, "0xFA LD A, (nn)");
            Registers.A = mmu.read8(mmu.read16(Registers.PC));
            Registers.PC += 2;
            break;
        case 0xFB: // NI
            logCall(true, "0xFB NI");
            interrupts.setIME(true);
            break;
        case 0xFE: // CP n
            cp_n("0xFE - CP n", mmu.read8(Registers.PC++));
            break;
        case 0xFF: // RST $38
            logCall(true, "0xFF RST $38");
            mmu.tick(4);
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0038;
            break;
        default:
            logger.log("[DMG-CPU] ==================================================");
            logger.log("[DMG-CPU] FF40: %02X FF41: %02X FF42: %02X FF44: %02X", mmu.memory[0xFF40], mmu.memory[0xFF41], mmu.memory[0xFF42], mmu.memory[0xFF44]);
            printFlags();
            printRegisters();
            logger.log("[DMG-CPU] Unsupported opcode: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG-CPU] DIV: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG-CPU] cycles: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG-CPU] ==================================================");
            return;
            break;
    }
}
