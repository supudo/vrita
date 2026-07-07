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
#include <string>

#include "cartridge.hpp"
#include "cpu_registers.hpp"
#include "interrupt.hpp"
#include "mmu.hpp"

class DMG_CPU {
public:
    DMG_CPU(Logger& logger, DMG_MMU& mmu, DMG_INTERRUPT& interrupts, bool halted, uint64_t cycles) : logger(logger), mmu(mmu), interrupts(interrupts) {}
    void step(bool ROMFileLoaded);
    void clearResources();

    DMGCpuRegisters Registers;

    inline void printRegisters() {
        logger.log("[DMG-CPU] A: 0x%02X, F: 0x%02X", +Registers.A, +Registers.F);
        logger.log("[DMG-CPU] B: 0x%02X, C: 0x%02X", +Registers.B, +Registers.C);
        logger.log("[DMG-CPU] D: 0x%02X, E: 0x%02X", +Registers.D, +Registers.E);
        logger.log("[DMG-CPU] H: 0x%02X, L: 0x%02X", +Registers.H, +Registers.L);
        logger.log("[DMG-CPU] PC: 0x%04X", +Registers.PC);
        logger.log("[DMG-CPU] SP: 0x%04X", +Registers.SP);
    }

    inline void setFlag(uint8_t flag, bool enabled) { if (enabled) Registers.F |= flag; else Registers.F &= ~flag; Registers.F &= 0xF0; }
    inline bool getFlag(uint8_t flag) const { return (Registers.F & flag) != 0; }
    inline bool isFlagSet(uint8_t flag) const { return Registers.F & (flag); }
    inline void printFlags() { logger.log("[DMG-CPU] Z: 0x%02X, N: 0x%02X, H: 0x%02X, C: 0x%02X", isFlagSet(FLAG_ZERO), isFlagSet(FLAG_SUBTRACT), isFlagSet(FLAG_HALF_CARRY), isFlagSet(FLAG_CARRY)); }
    bool logCalls = false;

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& interrupts;

    void executeInstruction8bit(bool ROMFileLoaded, uint8_t opcode);
    void executeInstruction16bit(bool ROMFileLoaded, uint8_t opcode);

    // instructions
    void ret(const char* logMessage, bool condition);
    void xor_(const char* logMessage, uint8_t value);
    void inc(const char* logMessage, uint8_t* value);
    void dec(const char* logMessage, uint8_t* value);
    void add(const char* logMessage, uint8_t* destination, uint8_t value);
    void add(const char* logMessage, uint16_t* destination, uint16_t value);
    void add(const char* logMessage, uint16_t* destination, int8_t value);
    void ldhl(const char* logMessage, int8_t value);
    void adc(const char* logMessage, uint8_t value);
    void sbc(const char* logMessage, uint8_t value);
    void sub(const char* logMessage, uint8_t value);
    void and_(const char* logMessage, uint8_t value);
    void or_(const char* logMessage, uint8_t value);
    void cp(const char* logMessage, uint8_t value);
    void call(const char* logMessage, bool condition);
    void jump(const char* logMessage, bool condition);
    void jump_add(const char* logMessage, bool condition);
    void cp_n(const char* logMessage, uint8_t value);

    // extended instructions
    void bit(const char* logMessage, uint8_t bit, uint8_t value);
    void res(const char* logMessage, uint8_t bit, uint8_t* rgst);
    void set(const char* logMessage, uint8_t bit, uint8_t* rgst);
    void rl(const char* logMessage, uint8_t* value);
    void rlc(const char* logMessage, uint8_t* value);
    void rr(const char* logMessage, uint8_t* value);
    void rrc(const char* logMessage, uint8_t* value);
    void sla(const char* logMessage, uint8_t* value);
    void sra(const char* logMessage, uint8_t* value);
    void srl(const char* logMessage, uint8_t* value);
    void swap(const char* logMessage, uint8_t* value);

    void logCall(bool isNormal, const char* msg1, const char* msg2 = "");
};

#endif