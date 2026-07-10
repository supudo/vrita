#include "apu.hpp"

#include <algorithm>

// ------------------------------------------------------------
// Get channel output
//
// channel:
// 0 = CH1
// 1 = CH2
// 2 = CH3
// 3 = CH4
// ------------------------------------------------------------
uint8_t DMG_APU::channelOutput(uint8_t channel) {
    switch (channel) {
        case 0: return pulseOutput(ch1);
        case 1: return pulseOutput(ch2);
        case 2: return waveOutput();
        case 3: return noiseOutput();
    }
    return 0;
}

// Mix a stereo sample pair
void DMG_APU::mixSample(int16_t& left, int16_t& right) {
    if (!(registers.NR52 & 0x80)) {
        left = 0;
        right = 0;
        return;
    }
    int l = 0;
    int r = 0;
    for (uint8_t i = 0; i < 4; i++) {
        if (!mixer.enabledLeft[i] && !mixer.enabledRight[i])
            continue;
        uint8_t value = channelOutput(i);
        if (mixer.enabledLeft[i])
            l += value;
        if (mixer.enabledRight[i])
            r += value;
    }

    // NR50 volume:
    // 0 = minimum
    // 7 = maximum
    // The hardware adds 1 internally.
    l *= (mixer.volumeLeft + 1);
    r *= (mixer.volumeRight + 1);

    // Convert to signed 16-bit, keeping headroom to avoid clipping.
    // (max per side: 4 channels * 15 amplitude * 8 volume = 480; 480*68 ~= 32640)
    l *= 68;
    r *= 68;
    left = static_cast<int16_t>(std::clamp(l, -32768, 32767));
    right = static_cast<int16_t>(std::clamp(r, -32768, 32767));
}