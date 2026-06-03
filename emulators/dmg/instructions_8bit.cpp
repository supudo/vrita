#include "cpu.hpp"

void DMG_CPU::executeInstruction8bit(bool ROMFileLoaded, uint8_t opcode) {
    if (!ROMFileLoaded) return;

    switch (opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, nn
            Registers.BC = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x02: // LD (BC), A
            mmu.write8(Registers.BC, Registers.A);
            break;
        case 0x03: // INC BC
            Registers.BC++;
            cycles += 4;
            break;
        case 0x04: // INC B
            inc(&Registers.B);
            break;
        case 0x05: // DEC B
            dec(&Registers.B);
            break;
        case 0x06: // LD B, n
            Registers.B = mmu.read8(Registers.PC++);
            break;
        case 0x07: // RLCA
            rlc(&Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x08: // LD (nn), SP
            mmu.write16(mmu.read16(Registers.PC), Registers.SP);
            Registers.PC += 2;
            break;
        case 0x09: // ADD HL, BC
            add(&Registers.HL, Registers.BC);
            cycles += 4;
            break;
        case 0x0A: // LD A, (BC)
            Registers.A = mmu.read8(Registers.BC);
            break;
        case 0x0B: // DEC BC
            Registers.BC--;
            cycles += 4;
            break;
        case 0x0C: // INC C
            inc(&Registers.C);
            break;
        case 0x0D: // DEC C
            dec(&Registers.C);
            break;
        case 0x0E: // LD C, n
            Registers.C = mmu.read8(Registers.PC++);
            break;
        case 0x0F: // RRCA
            rrc(&Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x10: // STOP
            break;
        case 0x11: // LD DE, nn
            Registers.DE = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x12: // LD (DE), A
            mmu.write8(Registers.DE, Registers.A);
            break;
        case 0x13: // INC DE
            Registers.DE++;
            cycles += 4;
            break;
        case 0x14: // INC D
            inc(&Registers.D);
            break;
        case 0x15: // DEC D
            dec(&Registers.D);
            break;
        case 0x16: // LD D, n
            Registers.D = mmu.read8(Registers.PC++);
            break;
        case 0x17: // RLA
            rl(&Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x18: // JR nn
            {
                int8_t offset = (int8_t)mmu.read8(Registers.PC++);
                Registers.PC += offset;
                cycles += 4;
            }
            break;
        case 0x19: // ADD HL, DE
            add(&Registers.HL, Registers.DE);
            cycles += 4;
            break;
        case 0x1A: // LD A, (DE)
            Registers.A = mmu.read8(Registers.DE);
            break;
        case 0x1B: // DEC DE
            Registers.DE--;
            cycles += 4;
            break;
        case 0x1C: // INC E
            inc(&Registers.E);
            break;
        case 0x1D: // DEC E
            dec(&Registers.E);
            break;
        case 0x1E: // LD E, n
            Registers.E = mmu.read8(Registers.PC++);
            break;
        case 0x1F: // RRA
            rr(&Registers.A);
            setFlag(FLAG_ZERO, false);
            break;
        case 0x20: // JR NZ, *
            jump_add(!isFlagSet(FLAG_ZERO));
            break;
        case 0x21: // LD HL, nn
            Registers.HL = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x22: // LD (HLI), A | LD (HL+), A | LDI (HL), A
            mmu.write8(Registers.HL++, Registers.A);
            break;
        case 0x23: // INC HL
            Registers.HL++;
            cycles += 4;
            break;
        case 0x24: // INC H
            inc(&Registers.H);
            break;
        case 0x25: // DEC H
            dec(&Registers.H);
            break;
        case 0x26: // LD H, n
            Registers.H = mmu.read8(Registers.PC++);
            break;
        case 0x27: // DAA
            {
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
            jump_add(isFlagSet(FLAG_ZERO));
            break;
        case 0x29: // ADD HL, HL
            add(&Registers.HL, Registers.HL);
            cycles += 4;
            break;
        case 0x2A: // LD A, (HL+)
            Registers.A = mmu.read8(Registers.HL++);
            break;
        case 0x2B: // DEC HL
            Registers.HL--;
            cycles += 4;
            break;
        case 0x2C: // INC L
            inc(&Registers.L);
            break;
        case 0x2D: // DEC L
            dec(&Registers.L);
            break;
        case 0x2E: // LD L, n
            Registers.L = mmu.read8(Registers.PC++);
            break;
        case 0x2F: // CPL
            Registers.A = ~Registers.A;
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, true);
            break;
        case 0x30: // JR NC, *
            jump_add(!isFlagSet(FLAG_CARRY));
            break;
        case 0x31: // LD SP, nn
            Registers.SP = mmu.read16(Registers.PC);
            Registers.PC += 2;
            break;
        case 0x32: // LD (HLD), A | LD (HL-), A | LDD (HL), A
            mmu.write8(Registers.HL--, Registers.A);
            break;
        case 0x33: // INC SP
            Registers.SP++;
            cycles += 4;
            break;
        case 0x35: // DEC (HL)
            {
                uint8_t tmp_val = mmu.read8(Registers.HL);
                dec(&tmp_val);
                mmu.write16(Registers.HL, tmp_val);
            }
            break;
        case 0x34: // INC (HL)
            {
                uint8_t tmp_val = mmu.read8(Registers.HL);
                inc(&tmp_val);
                mmu.write16(Registers.HL, tmp_val);
            }
            break;
        case 0x36: // LD (HL), n
            mmu.write16(Registers.HL, mmu.read16(Registers.PC++));
            break;
        case 0x37: // SCF
            setFlag(FLAG_CARRY, true);
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
            break;
        case 0x38: // JR C, *
            jump_add(isFlagSet(FLAG_CARRY));
            break;
        case 0x39: // ADD HL, SP
            add(&Registers.HL, Registers.SP);
            cycles += 4;
            break;
        case 0x3A: // LD A, (HL-)
            Registers.A = mmu.read8(Registers.HL--);
            break;
        case 0x3B: // DEC SP
            Registers.SP--;
            cycles += 4;
            break;
        case 0x3C: // INC A
            inc(&Registers.A);
            break;
        case 0x3D: // DEC A
            dec(&Registers.A);
            break;
        case 0x3E: // LD A, n
            Registers.A = mmu.read8(Registers.PC++);
            break;
        case 0x3F: // CCF
            setFlag(FLAG_CARRY, !isFlagSet(FLAG_CARRY));
            setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
            break;
        case 0x40: // LD B, B
            break;
        case 0x41: // LD B, C
            Registers.B = Registers.C;
            break;
        case 0x42: // LD B, D
            Registers.B = Registers.D;
            break;
        case 0x43: // LD B, E
            Registers.B = Registers.E;
            break;
        case 0x44: // LD B, H
            Registers.B = Registers.H;
            break;
        case 0x45: // LD B, L
            Registers.B = Registers.L;
            break;
        case 0x46: // LD B, (HL)
            Registers.B = mmu.read8(Registers.HL);
            break;
        case 0x47: // LD B, A
            Registers.B = Registers.A;
            break;
        case 0x48: // LD C, B
            Registers.C = Registers.B;
            break;
        case 0x49: // LD C, C
            break;
        case 0x4A: // LD C, D
            Registers.C = Registers.D;
            break;
        case 0x4B: // LD C, E
            Registers.C = Registers.E;
            break;
        case 0x4C: // LD C, H
            Registers.C = Registers.H;
            break;
        case 0x4D: // LD C, L
            Registers.C = Registers.L;
            break;
        case 0x4E: // LD C, (HL)
            Registers.C = mmu.read8(Registers.HL);
            break;
        case 0x4F: // LD C, A
            Registers.C = Registers.A;
            break;
        case 0x50: // LD D, B
            Registers.D = Registers.B;
            break;
        case 0x51: // LD D, C
            Registers.D = Registers.C;
            break;
        case 0x52: // LD D, D
            break;
        case 0x53: // LD D, E
            Registers.D = Registers.E;
            break;
        case 0x54: // LD D, H
            Registers.D = Registers.H;
            break;
        case 0x55: // LD D, L
            Registers.D = Registers.L;
            break;
        case 0x56: // LD D, (HL)
            Registers.D = mmu.read8(Registers.HL);
            break;
        case 0x57: // LD D, A
            Registers.D = Registers.A;
            break;
        case 0x58: // LD E, B
            Registers.E = Registers.B;
            break;
        case 0x59: // LD E, C
            Registers.E = Registers.C;
            break;
        case 0x5A: // LD E, D
            Registers.E = Registers.D;
            break;
        case 0x5B: // LD E, E
            break;
        case 0x5C: // LD E, H
            Registers.E = Registers.H;
            break;
        case 0x5D: // LD E, L
            Registers.E = Registers.L;
            break;
        case 0x5E: // LD E, (HL)
            Registers.E = mmu.read8(Registers.HL);
            break;
        case 0x5F: // LD E, A
            Registers.E = Registers.A;
            break;
        case 0x60: // LD H, B
            Registers.H = Registers.B;
            break;
        case 0x61: // LD H, C
            Registers.H = Registers.C;
            break;
        case 0x62: // LD H, D
            Registers.H = Registers.D;
            break;
        case 0x63: // LD H, E
            Registers.H = Registers.E;
            break;
        case 0x64: // LD H, H
            break;
        case 0x65: // LD H, L
            Registers.H = Registers.L;
            break;
        case 0x66: // LD H, (HL)
            Registers.H = mmu.read8(Registers.HL);
            break;
        case 0x67: // LD H, A
            Registers.H = Registers.A;
            break;
        case 0x68: // LD L, B
            Registers.L = Registers.B;
            break;
        case 0x69: // LD L, C
            Registers.L = Registers.C;
            break;
        case 0x6A: // LD L, D
            Registers.L = Registers.D;
            break;
        case 0x6B: // LD L, E
            Registers.L = Registers.E;
            break;
        case 0x6C: // LD L, H
            Registers.L = Registers.H;
            break;
        case 0x6D: // LD L, L
            break;
        case 0x6E: // LD L, (HL)
            Registers.L = mmu.read8(Registers.HL);
            break;
        case 0x6F: // LD L, A
            Registers.L = Registers.A;
            break;
        case 0x70: // LD (HL), B
            mmu.write8(Registers.HL, Registers.B);
            break;
        case 0x71: // LD (HL), C
            mmu.write8(Registers.HL, Registers.C);
            break;
        case 0x72: // LD (HL), D
            mmu.write8(Registers.HL, Registers.D);
            break;
        case 0x73: // LD (HL), E
            mmu.write8(Registers.HL, Registers.E);
            break;
        case 0x74: // LD (HL), H
            mmu.write8(Registers.HL, Registers.H);
            break;
        case 0x75: // LD (HL), L
            mmu.write8(Registers.HL, Registers.L);
            break;
        case 0x76: // HALT
            if (!interrupts.getIME() && (mmu.memory[0xFF0F] & mmu.memory[0xFFFF] & 0x1F))
                halted = false;
            else
                halted = true;
            break;
        case 0x77: // LD (HL), A
            mmu.write8(Registers.HL, Registers.A);
            break;
        case 0x78: // LD A, B
            Registers.A = Registers.B;
            break;
        case 0x79: // LD A, C
            Registers.A = Registers.C;
            break;
        case 0x7A: // LD A, D
            Registers.A = Registers.D;
            break;
        case 0x7B: // LD A, E
            Registers.A = Registers.E;
            break;
        case 0x7C: // LD A, H
            Registers.A = Registers.H;
            break;
        case 0x7D: // LD A, L
            Registers.A = Registers.L;
            break;
        case 0x7E: // LD A, (HL)
            Registers.A = mmu.read8(Registers.HL);
            break;
        case 0x7F: // LD A, A
            break;
        case 0x80: // ADD A, B
            add(&Registers.A, Registers.B);
            break;
        case 0x81: // ADD A, C
            add(&Registers.A, Registers.C);
            break;
        case 0x82: // ADD A, D
            add(&Registers.A, Registers.D);
            break;
        case 0x83: // ADD A, E
            add(&Registers.A, Registers.E);
            break;
        case 0x84: // ADD A, H
            add(&Registers.A, Registers.H);
            break;
        case 0x85: // ADD A, L
            add(&Registers.A, Registers.L);
            break;
        case 0x86: // ADD A, (HL)
            add(&Registers.A, mmu.read8(Registers.HL));
            break;
        case 0x87: // ADD A, A
            add(&Registers.A, Registers.A);
            break;
        case 0x88: // ADC A, B
            adc(Registers.B);
            break;
        case 0x89: // ADC A, C
            adc(Registers.C);
            break;
        case 0x8A: // ADC A, D
            adc(Registers.D);
            break;
        case 0x8B: // ADC A, E
            adc(Registers.E);
            break;
        case 0x8C: // ADC A, H
            adc(Registers.H);
            break;
        case 0x8D: // ADC A, L
            adc(Registers.L);
            break;
        case 0x8E: // ADC A, (HL)
            adc(mmu.read8(Registers.HL));
            break;
        case 0x8F: // ADC A, A
            adc(Registers.A);
            break;
        case 0x90: // SUB B
            sub(Registers.B);
            break;
        case 0x91: // SUB C
            sub(Registers.C);
            break;
        case 0x92: // SUB D
            sub(Registers.D);
            break;
        case 0x93: // SUB E
            sub(Registers.E);
            break;
        case 0x94: // SUB H
            sub(Registers.H);
            break;
        case 0x95: // SUB L
            sub(Registers.L);
            break;
        case 0x96: // SUB (HL)
            sub(mmu.read8(Registers.HL));
            break;
        case 0x97: // SUB A
            sub(Registers.A);
            break;
        case 0x98: // SBC A, B
            sbc(Registers.B);
            break;
        case 0x99: // SBC A, C
            sbc(Registers.C);
            break;
        case 0x9A: // SBC A, D
            sbc(Registers.D);
            break;
        case 0x9B: // SBC A, E
            sbc(Registers.E);
            break;
        case 0x9C: // SBC A, H
            sbc(Registers.H);
            break;
        case 0x9D: // SBC A, L
            sbc(Registers.L);
            break;
        case 0x9E: // SBC A, (HL)
            sbc(mmu.read8(Registers.HL));
            break;
        case 0x9F: // SBC A, A
            sbc(Registers.A);
            break;
        case 0xA0: // AND B
            and_(Registers.B);
            break;
        case 0xA1: // AND C
            and_(Registers.C);
            break;
        case 0xA2: // AND D
            and_(Registers.D);
            break;
        case 0xA3: // AND E
            and_(Registers.E);
            break;
        case 0xA4: // AND H
            and_(Registers.H);
            break;
        case 0xA5: // AND l
            and_(Registers.L);
            break;
        case 0xA6: // AND (HL)
            and_(mmu.read8(Registers.HL));
            break;
        case 0xA7: // AND A
            and_(Registers.A);
            break;
        case 0xA8: // XOR B
            xor_(Registers.B);
            break;
        case 0xA9: // XOR C
            xor_(Registers.C);
            break;
        case 0xAA: // XOR D
            xor_(Registers.D);
            break;
        case 0xAB: // XOR E
            xor_(Registers.E);
            break;
        case 0xAC: // XOR H
            xor_(Registers.H);
            break;
        case 0xAD: // XOR L
            xor_(Registers.L);
            break;
        case 0xAE: // XOR (HL)
            xor_(mmu.read8(Registers.HL));
            break;
        case 0xAF: // XOR A
            xor_(Registers.A);
            break;
        case 0xB0: // OR B
            or_(Registers.B);
            break;
        case 0xB1: // OR C
            or_(Registers.C);
            break;
        case 0xB2: // OR D
            or_(Registers.D);
            break;
        case 0xB3: // OR E
            or_(Registers.E);
            break;
        case 0xB4: // OR H
            or_(Registers.H);
            break;
        case 0xB5: // OR L
            or_(Registers.L);
            break;
        case 0xB6: // OR (HL)
            or_(mmu.read8(Registers.HL));
            break;
        case 0xB7: // OR A
            or_(Registers.A);
            break;
        case 0xB8: // CP B
            cp(Registers.B);
            break;
        case 0xB9: // CP C
            cp(Registers.C);
            break;
        case 0xBA: // CP D
            cp(Registers.D);
            break;
        case 0xBB: // CP E
            cp(Registers.E);
            break;
        case 0xBC: // CP H
            cp(Registers.H);
            break;
        case 0xBD: // CP L
            cp(Registers.L);
            break;
        case 0xBE: // CP (HL)
            cp(mmu.read8(Registers.HL));
            break;
        case 0xBF: // CP A
            cp(Registers.A);
            break;
        case 0xC0: // RET NZ
            ret(!isFlagSet(FLAG_ZERO));
            break;
        case 0xC1: // POP BC
            Registers.BC = mmu.read_stack(&Registers.SP);
            break;
        case 0xC2: // JP NZ, nn
            jump(!isFlagSet(FLAG_ZERO));
            break;
        case 0xC3: // JP nn
            Registers.PC = mmu.read16(Registers.PC);
            cycles += 4;
            break;
        case 0xC4: // CALL NZ, nn
            call(!isFlagSet(FLAG_ZERO));
            break;
        case 0xC5: // PUSH BC
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.BC);
            break;
        case 0xC6: // ADD A, n
            add(&Registers.A, mmu.read8(Registers.PC++));
            break;
        case 0xC7: // RST $00
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0000;
            break;
        case 0xC8: // RET Z
            ret(isFlagSet(FLAG_ZERO));
            break;
        case 0xC9: // RET
            Registers.PC = mmu.read_stack(&Registers.SP);
            cycles += 4;
            break;
        case 0xCA: // JP Z, nn
            jump(isFlagSet(FLAG_ZERO));
            break;
        case 0xCB:
            executeInstruction16bit(ROMFileLoaded, mmu.read8(Registers.PC++));
            break;
        case 0xCC: // CALL Z, nn
            call(isFlagSet(FLAG_ZERO));
            break;
        case 0xCD: // CALL nn
            {
                uint16_t operand = mmu.read16(Registers.PC);
                Registers.PC += 2;
                cycles += 4;
                mmu.write_stack(&Registers.SP, Registers.PC);
                Registers.PC = operand;
            } break;
        case 0xCE: // ADC A, n
            adc(mmu.read8(Registers.PC++));
            break;
        case 0xCF: // RST $08
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0008;
            break;
        case 0xD0: // RET NC
            ret(!isFlagSet(FLAG_CARRY));
            break;
        case 0xD1: // POP DE
            Registers.DE = mmu.read_stack(&Registers.SP);
            break;
        case 0xD2: // JP NC, nn
            jump(!isFlagSet(FLAG_CARRY));
            break;
        case 0xD4: // CALL NC, nn
            call(!isFlagSet(FLAG_CARRY));
            break;
        case 0xD5: // PUSH DE
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.DE);
            break;
        case 0xD6: // SUB n
            sub(mmu.read8(Registers.PC++));
            break;
        case 0xD7: // RST $10
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0010;
            break;
        case 0xD8: // RET C
            ret(isFlagSet(FLAG_CARRY));
            break;
        case 0xD9: // RETI
            interrupts.setIME(true);
            Registers.PC = mmu.read_stack(&Registers.SP);
            cycles += 4;
            break;
        case 0xDA: // JP C, nn
            jump(isFlagSet(FLAG_CARRY));
            break;
        case 0xDC: // CALL C, nn
            call(isFlagSet(FLAG_CARRY));
            break;
        case 0xDE: // SUB n
            sbc(mmu.read8(Registers.PC++));
            break;
        case 0xDF: // RST $18
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0018;
            break;
        case 0xE0: // LD ($FF00+n), A
            mmu.write16(0xff00 + mmu.read8(Registers.PC++), Registers.A);
            break;
        case 0xE1: // POP HL
            Registers.HL = mmu.read_stack(&Registers.SP);
            break;
        case 0xE2: // LD ($FF00+C), A
            mmu.write8(0xff00 + Registers.C, Registers.A);
            break;
        case 0xE5: // PUSH HL
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.HL);
            break;
        case 0xE6: // AND n
            and_(mmu.read8(Registers.PC++));
            break;
        case 0xE7: // RST $20
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0020;
            break;
        case 0xE8: // ADD SP, n
            add(&Registers.SP, (int8_t)mmu.read8(Registers.PC++));
            cycles += 8;
            break;
        case 0xE9: // JP HL
            Registers.PC = Registers.HL;
            break;
        case 0xEA: // LD (nn), A
            mmu.write16(mmu.read16(Registers.PC), Registers.A);
            Registers.PC += 2;
            break;
        case 0xEE: // XOR n
            xor_(mmu.read8(Registers.PC++));
            break;
        case 0xEF: // RST $28
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0028;
            break;
        case 0xF0: // LD A, ($FF00+n)
            Registers.A = mmu.read8(0xff00 + mmu.read8(Registers.PC++));
            break;
        case 0xF1: // POP AF
            Registers.AF = mmu.read_stack(&Registers.SP);
            Registers.F &= 0xf0;  // Reset the 4 unused bits
            break;
        case 0xF2: // LD A, (C)
            Registers.A = mmu.read8(0xff00 + Registers.C);
            break;
        case 0xF3: // DI
            interrupts.setIME(false);
            break;
        case 0xF5: // PUSH AF
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.AF);
            break;
        case 0xF6: // OR n
            or_(mmu.read8(Registers.PC++));
            break;
        case 0xF7: // RST $30
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0030;
            break;
        case 0xF8: // LDHL SP, n
            ldhl(mmu.read8(Registers.PC++));
            cycles += 4;
            break;
        case 0xF9: // LD SP, HL
            Registers.SP = Registers.HL;
            cycles += 4;
            break;
        case 0xFA: // LD A, (nn)
            Registers.A = mmu.read8(mmu.read16(Registers.PC));
            Registers.PC += 2;
            break;
        case 0xFB: // NI
            interrupts.setIME(true);
            break;
        case 0xFE: // CP n
            cp_n(mmu.read8(Registers.PC++));
            break;
        case 0xFF: // RST $38
            cycles += 4;
            mmu.write_stack(&Registers.SP, Registers.PC);
            Registers.PC = 0x0038;
            break;
        default:
            logger.log("[DMG_CPU] ==================================================");
            logger.log("[DMG_CPU] FF40: %02X FF41: %02X FF42: %02X FF44: %02X", mmu.memory[0xFF40], mmu.memory[0xFF41], mmu.memory[0xFF42], mmu.memory[0xFF44]);
            printFlags();
            printRegisters();
            logger.log("[DMG_CPU] Unsupported opcode: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG_CPU] DIV: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG_CPU] cycles: 0x%02X at 0x%04X", opcode, Registers.PC);
            logger.log("[DMG_CPU] ==================================================");
            return;
            break;
    }
}
