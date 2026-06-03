/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_TIMER_INCLUDES
#define VRITA_DMG_TIMER_INCLUDES

#include <cstdint>
#include <SDL3/SDL.h>

class DMG_INTERRUPT;

class DMG_TIMER {
public:
    explicit DMG_TIMER(DMG_INTERRUPT& managerInterrupts);

    static constexpr uint32_t CPU_HZ = 4194304;
    static constexpr uint32_t CYCLES_PER_FRAME = 70224; // 154 scanlines * 456 T-cycles

    void reset();
    void tick(uint32_t cycles);
    uint8_t read(uint16_t address) const;
    void write(uint16_t address, uint8_t value);

private:
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
    static constexpr uint16_t addressDIV = 0xFF04;
    static constexpr uint16_t addressTIMA = 0xFF05;
    static constexpr uint16_t addressTMA = 0xFF06;
    static constexpr uint16_t addressTAC = 0xFF07;
    static constexpr uint8_t addressTIMER_INTERRUPT = 0x04;

    void incrementTIMA();
};

#endif
