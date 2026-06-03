#ifndef VRITA_DMG_INTERRUPT_INCLUDES
#define VRITA_DMG_INTERRUPT_INCLUDES

#include <stdint.h>

#include "mmu.hpp"

enum Interrupts { // IF
    INTERRUPT_VBLANK = (1 << 0),
    INTERRUPT_LCD = (1 << 1),
    INTERRUPT_TIMER = (1 << 2),
    INTERRUPT_SERIAL = (1 << 3),
    INTERRUPT_JOYPAD = (1 << 4),
    // 5, 6 and 7 are not used
};

class DMG_INTERRUPT {
public:
    DMG_INTERRUPT(DMG_MMU& mmu) : mmu(mmu) {}

    bool checkForInterrupts();
    bool isInterruptEnabled(uint8_t flag); // if IE - 0xFFFF is set
    bool isInterruptFlagSet(uint8_t flag); // if IF - 0xFF0F is set
    void triggerInterrupt(Interrupts interrupt, uint8_t jump_pc);

    inline void setIME(bool state) { IME = int(state) << 0; }
    inline bool getIME() { return IME & 1U; }

private:
    DMG_MMU& mmu;

    bool IME;

    uint16_t addressInterruptEnabled = 0xFFFF; // IE
    uint16_t addressInterruptFlag = 0xFF0F; // IF
};

#endif