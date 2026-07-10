#include "apu.hpp"

// Step CH3 frequency timer
void DMG_APU::stepWaveChannel() {
    if (!wave.state.enabled)
        return;
    if (wave.frequency.timer == 0) {
        wave.frequency.timer = frequencyTimerReload(wave.frequency.frequency, 2);
        wave.wavePosition++;
        wave.wavePosition &= 31;
        uint8_t byteIndex = wave.wavePosition / 2;
        uint8_t sampleByte = wave.waveRAM[byteIndex];
        if ((wave.wavePosition & 1) == 0) // high nibble
            wave.sampleBuffer = sampleByte >> 4;
        else // low nibble
            wave.sampleBuffer = sampleByte & 0x0F;
        wave.currentSample = wave.sampleBuffer;
    }
    else
        wave.frequency.timer--;
}

// CH3 output
uint8_t DMG_APU::waveOutput() {
    if (!wave.state.enabled || !wave.state.dacEnabled)
        return 0;
    uint8_t sample = wave.currentSample;
    switch (wave.volumeCode) {
        case 0: return 0;
        case 1: return sample; // 100%
        case 2: return sample >> 1; // 50%
        case 3: return sample >> 2; // 25%
    }
    return 0;
}

// Trigger CH3
void DMG_APU::triggerWave() {
    if (!triggerCommon(wave, 256))
        return;
    wave.frequency.timer = frequencyTimerReload(wave.frequency.frequency, 2);
    wave.wavePosition = 0;
    wave.sampleBuffer = wave.waveRAM[0] >> 4;
    wave.currentSample = wave.sampleBuffer;
}