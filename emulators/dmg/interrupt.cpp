#include "interrupt.hpp"

void DMG_INTERRUPT::setInterruptsEnabled(bool state) {
    IME = int(state) << 0;
}

bool DMG_INTERRUPT::areInterruptsEnabled() {
    return IME & 1U;
}
