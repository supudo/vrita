#include "joypad.hpp"
#include "utilities/logger.hpp"

void DMG_JOYPAD::clearResources() {
    selectBits = 0x30;
    dpadState = 0x0F;
    buttonsState = 0x0F;
}

uint8_t DMG_JOYPAD::read() const {
    uint8_t lowNibble = 0x0F;
    if (!(selectBits & 0x10))
        lowNibble &= dpadState;
    if (!(selectBits & 0x20))
        lowNibble &= buttonsState;
    return 0xC0 | selectBits | lowNibble;
}

void DMG_JOYPAD::write(uint8_t value) {
    selectBits = value & 0x30;
}

void DMG_JOYPAD::setDpadState(uint8_t state) { 
    dpadState = state & 0x0F; 
}

void DMG_JOYPAD::setButtonsState(uint8_t state) { 
    buttonsState = state & 0x0F; 
}