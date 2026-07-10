#include "apu.hpp"

// Noise timer reload
static uint16_t noiseTimerReload(uint8_t divisorCode, uint8_t shift) {
    static constexpr uint8_t divisors[8] = {
        8, 16, 32, 48,
        64, 80, 96, 112
    };
    return divisors[divisorCode & 0x07] << shift;
}

// Step CH4
void DMG_APU::stepNoiseChannel() {
    if (!noise.state.enabled)
        return;

    if (noise.timer == 0) {
        noise.timer = noiseTimerReload(noise.divisorCode, noise.clockShift);
        // LFSR algorithm:
        // 
        // bit0 XOR bit1
        // shift right
        // put result into bit14
        // 
        // If width mode:
        // also put result into bit6
        uint16_t xorBit = (noise.lfsr & 1) ^ ((noise.lfsr >> 1) & 1);
        noise.lfsr >>= 1;
        noise.lfsr |= (xorBit << 14);
        if (noise.widthMode) {
            noise.lfsr &= ~(1 << 6);
            noise.lfsr |= (xorBit << 6);
        }
    }
    else
        noise.timer--;
}

// Output
uint8_t DMG_APU::noiseOutput() {
    if (!noise.state.enabled || !noise.state.dacEnabled)
        return 0;
    // LFSR bit 0 is inverted output.
    if (noise.lfsr & 1)
        return 0;
    return noise.envelope.volumeCurrent;
}

// Trigger CH4
void DMG_APU::triggerNoise() {
    if (!triggerCommon(noise, 64))
        return;
    noise.lfsr = 0x7FFF;
    noise.timer = noiseTimerReload(noise.divisorCode, noise.clockShift);
    resetEnvelope(noise);
}