#include "interrupt.hpp"

void DMG_INTERRUPT::initialize(DMG_MMU* mmu) {
    this->mmu = mmu;
}

bool DMG_INTERRUPT::isInterruptEnabled(uint8_t flag) {
    return mmu->memory[0xFFFF] & flag;
}

bool DMG_INTERRUPT::isInterruptFlagSet(uint8_t flag) {
    return mmu->memory[0xFF0F] & flag;
}

void DMG_INTERRUPT::triggerInterrupt(Interrupts interrupt, uint8_t jump_pc) {
}

bool DMG_INTERRUPT::checkForInterrupts() {
    if (mmu->memory[0xFFFF] & mmu->memory[0xFF0F] & 0x0F) {

    }

    if (!getIME())
        return false;

    // VLBANK
    if (isInterruptEnabled(INTERRUPT_VBLANK) && isInterruptFlagSet(INTERRUPT_VBLANK)) {
        triggerInterrupt(INTERRUPT_VBLANK, 0x40);
        return true;
    }

    // LCD
    if (isInterruptEnabled(INTERRUPT_LCD) && isInterruptFlagSet(INTERRUPT_LCD)) {
        triggerInterrupt(INTERRUPT_LCD, 0x48);
        return true;
    }

    // Timer
    if (isInterruptEnabled(INTERRUPT_TIMER) && isInterruptFlagSet(INTERRUPT_TIMER)) {
        triggerInterrupt(INTERRUPT_TIMER, 0x50);
        return true;
    }

    // Serial
    if (isInterruptEnabled(INTERRUPT_SERIAL) && isInterruptFlagSet(INTERRUPT_SERIAL)) {
        triggerInterrupt(INTERRUPT_SERIAL, 0x58);
        return true;
    }

    // Joypad
    if (isInterruptEnabled(INTERRUPT_JOYPAD) && isInterruptFlagSet(INTERRUPT_JOYPAD)) {
        triggerInterrupt(INTERRUPT_JOYPAD, 0x60);
        return true;
    }

    return false;
}
