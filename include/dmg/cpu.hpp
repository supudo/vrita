/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_CPU_INCLUDES
#define VRITA_DMG_CPU_INCLUDES

#include "../emulators.hpp"
#include "../logger.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

#include "cartridge.hpp"

class DMG_CPU {
public:
    void initialize(Logger& logger, std::shared_ptr<DMG_CARTRIDGE> cartridge);
    void stepCPU(bool ROMFileLoaded, uint8_t *memory);

    uint64_t cycles = 0;
    bool halted = false;

    // registers
    struct CpuRegisters {
        uint8_t A, F;
        uint8_t B, C;
        uint8_t D, E;
        uint8_t H, L;

        uint16_t SP = 0; // stack pointer
        uint16_t PC = 0; // program counter/pointer

        // 16-bit registers combos
        uint16_t AF() const { return (A << 8) | F; } // accumulation & flags
        void setAF(uint16_t val) { A = val >> 8; F = val & 0xF0; } // F lower nibble is always 0

        uint16_t BC() const { return (B << 8) | C; } // BC
        void setBC(uint16_t val) { B = val >> 8; C = val & 0xFF; }

        uint16_t DE() const { return (D << 8) | E; } // DE
        void setDE(uint16_t val) { D = val >> 8; E = val & 0xFF; }

        uint16_t HL() const { return (H << 8) | L; } // HL
        void setHL(uint16_t val) { H = val >> 8; L = val & 0xFF; }
    } CpuRegisters;

    enum CpuFlags { // lower 8 buts of AF register
        FLAG_ZERO = 1 << 7, // zero falg
        FLAG_SUBTRACT = 1 << 6, // subsctraction flag (BCD)
        FLAG_HALF_CARRY = 1 << 5, // half carry flag (BCD)
        FLAG_CARRY = 1 << 4 // carry flag
    };

    inline void setFlag(uint8_t flag, bool enabled) { if (enabled) CpuRegisters.F |= flag; else CpuRegisters.F &= ~flag; CpuRegisters.F &= 0xF0; }
    inline bool getFlag(uint8_t flag) const { return (CpuRegisters.F & flag) != 0; }
    inline bool isFlagSet(uint8_t flag) const { return CpuRegisters.F & (flag); }

    // operands
    struct Instruction {
        const char* name;
        void (DMG_CPU::* execute)();
        uint8_t bytes;
        uint8_t cycles;
    };

    Instruction InstructionsTable[256];

    // Miscelanious instructions
    void NOP();
    void HALT(void);
    void STOP(void);
    void DI(void);
    void EI(void);
    // 8-bit load instructions
    // 16-it load instructions
    // 8-bit arithmetic and logical instructions
    void add_a_b(void);
    void add_a_c(void);
    void add_a_d(void);
    void add_a_e(void);
    void add_a_h(void);
    void add_a_l(void);
    void add_a_hl(void);
    void add_a_a(void);
    // 16-bit arithmetic instructions
    // Rotate, shift, and bit operation instructions
    void set_7_a(void);
    // Control flow instructions

private:
    Logger* logger = nullptr;
    void logCall(const char* op);

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
    void extended_execute(uint8_t opcode);
    void bit(uint8_t bit, uint8_t value);
    void res(uint8_t bit, uint8_t* rgst);
    void set(uint8_t bit, uint8_t* rgst);
    void rl(uint8_t* value);
    void rlc(uint8_t* value);
    void rr(uint8_t* value);
    void rrc(uint8_t* value);
    void rra();
    void rla();
    void rlca();
    void sla(uint8_t* value);
    void sra(uint8_t* value);
    void srl(uint8_t* value);
    void swap(uint8_t* value);
};

#endif