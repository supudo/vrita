#include "joypad.hpp"

#include <SDL3/SDL.h>

#include "utilities/logger.hpp"
#include "interrupt.hpp"

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

void DMG_JOYPAD::handleKey(uint32_t type, uint32_t key) {
    switch (key) {
        case SDLK_RETURN:
            setButton(JOYPAD_START, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_SPACE:
            setButton(JOYPAD_SELECT, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_A:
            setButton(JOYPAD_A, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_B:
            setButton(JOYPAD_B, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_LEFT:
            setButton(JOYPAD_LEFT, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_RIGHT:
            setButton(JOYPAD_RIGHT, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_UP:
            setButton(JOYPAD_UP, type == SDL_EVENT_KEY_DOWN);
            break;
        case SDLK_DOWN:
            setButton(JOYPAD_DOWN, type == SDL_EVENT_KEY_DOWN);
            break;
    }
}

void DMG_JOYPAD::setButton(uint8_t button, bool keyDown) {
    uint8_t mask = 1 << button;
    bool wasPressed = !(buttonsVal & mask);
    
    if (keyDown)
        buttonsVal &= ~mask;
    else
        buttonsVal |= mask;

    bool isPressed = !(buttonsVal & mask);

    if (!wasPressed && isPressed)
        managerInterrupts.setInterruptFlag(INTERRUPT_JOYPAD);

    if (wasPressed != isPressed)
        switch (button) {
            case JOYPAD_START: logger.log("[JOYPAD] START %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_SELECT: logger.log("[JOYPAD] SELECT %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_A: logger.log("[JOYPAD] A %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_B: logger.log("[JOYPAD] B %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_LEFT: logger.log("[JOYPAD] LEFT %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_RIGHT: logger.log("[JOYPAD] RIGHT %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_UP: logger.log("[JOYPAD] UP %s.", (keyDown ? "pressed" : "released")); break;
            case JOYPAD_DOWN: logger.log("[JOYPAD] DOWN %s.", (keyDown ? "pressed" : "released")); break;
        }
}