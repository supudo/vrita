/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_APU_STRUCTS_INCLUDES
#define VRITA_DMG_APU_STRUCTS_INCLUDES

#include <array>
#include <cstdint>
#include <vector>

struct APURegisters {
    uint8_t NR50 = 0;
    uint8_t NR51 = 0;
    uint8_t NR52 = 0;
};

struct ChannelState {
    bool enabled = false;
    bool dacEnabled = false;
};

struct FrequencyTimer {
    uint16_t frequency = 0;
    uint16_t timer = 0;
};

struct FrameSequencer {
    uint16_t counter = 8192;
    uint8_t step = 0;
};

struct Mixer {
    uint8_t volumeLeft = 0;
    uint8_t volumeRight = 0;

    std::array<bool, 4> enabledLeft {};
    std::array<bool, 4> enabledRight {};

    bool masterEnable = false;

    bool vinLeft = false;
    bool vinRight = false;
};

struct LengthCounter {
    uint16_t counter = 0; // wave's counter goes up to 256, needs >8 bits
    bool enabled = false;
};

struct ClockTimer {
    uint8_t period = 0;
    uint8_t remaining = 0;
};

struct Envelope {
    ClockTimer timer;
    uint8_t volumeInitial = 0;
    uint8_t volumeCurrent = 0;
    bool increase = false;
};

inline uint16_t frequencyTimerReload(uint16_t frequency, uint8_t multiplier) {
    return (2048 - frequency) * multiplier;
}

struct AudioOutput {
    uint32_t sampleAccumulator = 0;
    static constexpr uint32_t cpuClock = 4194304;
    static constexpr uint32_t sampleRate = 44100;
    std::vector<int16_t> buffer;
};

struct PulseChannel {
    ChannelState state;
    FrequencyTimer frequency;
    LengthCounter length;
    Envelope envelope;
    
    // waveform
    uint8_t duty = 0; // 0-3
    uint8_t dutyPosition = 0; // 0-7

    // sweep - only CH1
    bool hasSweep = false;

    ClockTimer sweep;

    bool sweepNegate = false;
    uint8_t sweepShift = 0;

    uint16_t shadowFrequency = 0;
    bool sweepEnabled = false;

    uint8_t NRx0 = 0;
    uint8_t NRx1 = 0;
    uint8_t NRx2 = 0;
    uint8_t NRx3 = 0;
    uint8_t NRx4 = 0;
};

struct WaveChannel {
    ChannelState state;
    FrequencyTimer frequency; 
    LengthCounter length;

    uint8_t volumeCode = 0;
    uint8_t wavePosition = 0;
    std::array<uint8_t, 16> waveRAM {};
    uint8_t currentSample = 0;
    uint8_t sampleBuffer = 0;

    uint8_t NR30 = 0;
    uint8_t NR31 = 0;
    uint8_t NR32 = 0;
    uint8_t NR33 = 0;
    uint8_t NR34 = 0;
};

struct NoiseChannel {
    ChannelState state;
    LengthCounter length;
    Envelope envelope;

    uint16_t timer = 0;
    uint16_t lfsr = 0x7FFF;
    
    bool widthMode = false;
    
    uint8_t divisorCode = 0;
    uint8_t clockShift = 0;

    uint8_t NR41 = 0;
    uint8_t NR42 = 0;
    uint8_t NR43 = 0;
    uint8_t NR44 = 0;
};

#endif