/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_CPU_INCLUDES
#define VRITA_DMG_CPU_INCLUDES

#include "emulators/emulators.hpp"
#include "utilities/logger.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

#include "cartridge.hpp"
#include "interrupt.hpp"
#include "mmu.hpp"

class DMG_CPU {
public:
    DMG_CPU(Logger& logger, DMG_MMU& mmu, DMG_INTERRUPT& interrupts, bool halted, uint64_t cycles) : logger(logger), mmu(mmu), interrupts(interrupts), halted(halted), cycles(cycles) {}
    void stepCPU(bool ROMFileLoaded);
    void clearResources();

    uint64_t cycles = 0;
    bool halted = false;

    // registers
    struct Registers {
        uint8_t A, F;
        uint8_t B, C;
        uint8_t D, E;
        uint8_t H, L;

        uint16_t SP = 0; // stack pointer
        uint16_t PC = 0; // program counter/pointer

        // 16-bit registers combos
        union {
            struct { uint8_t F; uint8_t A; };
            uint16_t AF;
        };

        union {
            struct { uint8_t C; uint8_t B; };
            uint16_t BC;
        };

        union {
            struct { uint8_t E; uint8_t D; };
            uint16_t DE;
        };

        union {
            struct { uint8_t L; uint8_t H; };
            uint16_t HL;
        };
    } Registers;

    inline void printRegisters() {
        logger.log("A: 0x%02X, F: 0x%02X", +Registers.A, +Registers.F);
        logger.log("B: 0x%02X, C: 0x%02X", +Registers.B, +Registers.C);
        logger.log("D: 0x%02X, E: 0x%02X", +Registers.D, +Registers.E);
        logger.log("H: 0x%02X, L: 0x%02X", +Registers.H, +Registers.L);
        logger.log("PC: 0x%04X", +Registers.PC);
        logger.log("SP: 0x%04X", +Registers.SP);
    }

    enum CpuFlags { // lower 8 buts of AF register
        FLAG_ZERO = 1 << 7, // zero falg
        FLAG_SUBTRACT = 1 << 6, // subsctraction flag (BCD)
        FLAG_HALF_CARRY = 1 << 5, // half carry flag (BCD)
        FLAG_CARRY = 1 << 4 // carry flag
    };

    inline void setFlag(uint8_t flag, bool enabled) { if (enabled) Registers.F |= flag; else Registers.F &= ~flag; Registers.F &= 0xF0; }
    inline bool getFlag(uint8_t flag) const { return (Registers.F & flag) != 0; }
    inline bool isFlagSet(uint8_t flag) const { return Registers.F & (flag); }
    inline void printFlags() { logger.log("Z: 0x%02X, N: 0x%02X, H: 0x%02X, C: 0x%02X", isFlagSet(FLAG_ZERO), isFlagSet(FLAG_SUBTRACT), isFlagSet(FLAG_HALF_CARRY), isFlagSet(FLAG_CARRY)); }

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& interrupts;

    void executeInstruction8bit(bool ROMFileLoaded, uint8_t opcode);
    void executeInstruction16bit(bool ROMFileLoaded, uint8_t opcode);

    // instructions
    void ret(bool condition);
    void xor_(uint8_t value);
    void inc(uint8_t* value);
    void dec(uint8_t* value);
    void add(uint8_t* destination, uint8_t value);
    void add(uint16_t* destination, uint16_t value);
    void add(uint16_t* destination, int8_t value);
    void ldhl(int8_t value);
    void adc(uint8_t value);
    void sbc(uint8_t value);
    void sub(uint8_t value);
    void and_(uint8_t value);
    void or_(uint8_t value);
    void cp(uint8_t value);
    void call(bool condition);
    void jump(bool condition);
    void jump_add(bool condition);
    void cp_n(uint8_t value);

    // extended instructions
    void bit(uint8_t bit, uint8_t value);
    void res(uint8_t bit, uint8_t* rgst);
    void set(uint8_t bit, uint8_t* rgst);
    void rl(uint8_t* value);
    void rlc(uint8_t* value);
    void rr(uint8_t* value);
    void rrc(uint8_t* value);
    void sla(uint8_t* value);
    void sra(uint8_t* value);
    void srl(uint8_t* value);
    void swap(uint8_t* value);
};

#endif