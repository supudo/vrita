/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_TIMER_INCLUDES
#define VRITA_DMG_TIMER_INCLUDES

#include <cstdint>
#include <SDL3/SDL.h>

class DMG_TIMER {
public:
    static constexpr uint32_t CPU_HZ = 4194304;
    static constexpr uint32_t CYCLES_PER_FRAME = 70224; // 154 scanlines * 456 T-cycles

    void reset();
    uint32_t tickFrame(); // returns T-cycles to execute; call once per display frame

private:
    uint64_t perfFreq = 0;
    uint64_t lastTime = 0;
    double accumulator = 0.0;
};

#endif
