#include "timer.hpp"

#include "utilities\logger.hpp"
#include "interrupt.hpp"

DMG_TIMER::DMG_TIMER(Logger& logger, DMG_INTERRUPT& managerInterrupts) : logger(logger), managerInterrupts(managerInterrupts) {
    reset();
}

void DMG_TIMER::reset() {
    registerDIV = 0;
    registerTIMA = 0;
    registerTMA = 0;
    registerTAC = 0;

    internalDivCounter = 0;
    internalTimerCounter = 0;
}

void DMG_TIMER::tick(uint32_t cycles) {
    internalDivCounter += cycles;

    while (internalDivCounter >= 256) {
        internalDivCounter -= 256;
        ++registerDIV;
    }

    if (!(registerTAC & 0x04))
        return; // timer disabled

    uint32_t frequencyCycles = 1024;

    switch (registerTAC & 0x03) {
        case 0: frequencyCycles = 1024; break; // 4096 Hz
        case 1: frequencyCycles = 16; break; // 262144 Hz
        case 2: frequencyCycles = 64; break; // 65536 Hz
        case 3: frequencyCycles = 256; break; // 16384 Hz
    }

    internalTimerCounter += cycles;

    while (internalTimerCounter >= frequencyCycles) {
        internalTimerCounter -= frequencyCycles;
        incrementTIMA();
    }
}

void DMG_TIMER::incrementTIMA() {
    if (registerTIMA == 0xFF) {
        registerTIMA = registerTMA;
        managerInterrupts.setInterruptFlag(INTERRUPT_TIMER);
    }
    else
        ++registerTIMA;
}

namespace {
    constexpr uint16_t DIV_ADDR = 0xFF04;
    constexpr uint16_t TIMA_ADDR = 0xFF05;
    constexpr uint16_t TMA_ADDR = 0xFF06;
    constexpr uint16_t TAC_ADDR = 0xFF07;
    constexpr uint8_t TIMER_INTERRUPT = 0x04;
}

uint8_t DMG_TIMER::read(uint16_t address) const {
    switch (address) {
        case addressDIV: return registerDIV;
        case addressTIMA: return registerTIMA;
        case addressTMA: return registerTMA;
        case addressTAC: return registerTAC | 0xF8;
        default: return 0xFF;
    }
}

void DMG_TIMER::write(uint16_t address, uint8_t value) {
    switch (address) {
        case DIV_ADDR:
            registerDIV = 0;
            internalDivCounter = 0;
            break;
        case TIMA_ADDR:
            registerTIMA = value;
            break;
        case TMA_ADDR:
            registerTMA = value;
            break;
        case TAC_ADDR:
            registerTAC = value & 0x07;
            break;
    }
}
