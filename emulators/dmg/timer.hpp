/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_TIMER_INCLUDES
#define VRITA_DMG_TIMER_INCLUDES

#include <cstdint>
#include <SDL3/SDL.h>

class Logger;
class DMG_INTERRUPT;

class DMG_TIMER {
public:
    explicit DMG_TIMER(Logger& logger, DMG_INTERRUPT& managerInterrupts);

    static constexpr uint32_t CPU_HZ = 4194304;
    static constexpr uint32_t CYCLES_PER_FRAME = 70224; // 154 lines * 456 dots @ 4.194304 MHz / 59.7275 fps

    void reset();
    void tick(uint32_t cycles);
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);

private:
    Logger& logger;
    DMG_INTERRUPT& managerInterrupts;

    // registers
    uint8_t registerDIV = 0;
    uint8_t registerTIMA = 0;
    uint8_t registerTMA = 0;
    uint8_t registerTAC = 0;

    // internal counters
    uint32_t internalDivCounter = 0;
    uint32_t internalTimerCounter = 0;

    // addresses
    static constexpr uint16_t addressDIV = 0xFF04; // DIV - divier register
    static constexpr uint16_t addressTIMA = 0xFF05; // TIMA - timer counter
    static constexpr uint16_t addressTMA = 0xFF06; // TMA - timer modulo
    static constexpr uint16_t addressTAC = 0xFF07; // TAC - timer control

    void incrementTIMA();
};

#endif
