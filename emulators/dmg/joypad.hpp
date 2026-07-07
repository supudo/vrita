/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_JOYPAD_INCLUDES
#define VRITA_DMG_JOYPAD_INCLUDES

#include "mmu.hpp"

class Logger;
class DMG_MMU;

class DMG_JOYPAD {
public:
    DMG_JOYPAD(Logger& logger, DMG_MMU& mmu) : logger(logger), mmu(mmu) {}

    void clearResources();

    uint8_t read() const;
    void write(uint8_t value);

    void setDpadState(uint8_t state);
    void setButtonsState(uint8_t state);

private:
    Logger& logger;
    DMG_MMU& mmu;

    uint8_t selectBits = 0x30; // bits 4-5 of JOYP; 1 = group not selected
    uint8_t dpadState = 0x0F; // bits 0-3: Right,Left,Up,Down
    uint8_t buttonsState = 0x0F; // bits 0-3: A,B,Select,Start
};

#endif