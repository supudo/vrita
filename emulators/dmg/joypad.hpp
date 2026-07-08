/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_JOYPAD_INCLUDES
#define VRITA_DMG_JOYPAD_INCLUDES

#include "mmu.hpp"

class Logger;
class DMG_MMU;
class DMG_INTERRUPT;

class DMG_JOYPAD {
public:
    DMG_JOYPAD(Logger& logger, DMG_MMU& mmu, DMG_INTERRUPT& managerInterrupts) : logger(logger), mmu(mmu), managerInterrupts(managerInterrupts) {}

    void clearResources();

    uint8_t read() const;
    void write(uint8_t value);

    void setDpadState(uint8_t state);
    void setButtonsState(uint8_t state);

    void handleKey(uint32_t type, uint32_t key);

private:
    Logger& logger;
    DMG_MMU& mmu;
    DMG_INTERRUPT& managerInterrupts;

    uint8_t buttonsVal = 0xFF;
    uint8_t selectBits = 0x30; // bits 4-5 of JOYP; 1 = group not selected
    uint8_t dpadState = 0x0F; // bits 0-3: Right,Left,Up,Down
    uint8_t buttonsState = 0x0F; // bits 0-3: A,B,Select,Start

    static constexpr uint8_t JOYPAD_RIGHT = 0;
    static constexpr uint8_t JOYPAD_LEFT = 1;
    static constexpr uint8_t JOYPAD_UP = 2;
    static constexpr uint8_t JOYPAD_DOWN = 3;
    static constexpr uint8_t JOYPAD_A = 4;
    static constexpr uint8_t JOYPAD_B = 5;
    static constexpr uint8_t JOYPAD_SELECT = 6;
    static constexpr uint8_t JOYPAD_START = 7;

    void setButton(uint8_t button, bool keyDown);
};

#endif