/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_APU_INCLUDES
#define VRITA_DMG_APU_INCLUDES

#include <SDL3/SDL.h>
#include "apu_structs.hpp"
#include "mmu.hpp"

class DMG_APU {
public:
    DMG_APU(DMG_MMU& mmu) : mmu(mmu) {}

    void initAudioStream(SDL_AudioStream* audioStream);
    void step(bool ROMFileLoaded, uint32_t cycles);
    void clearResources();

    uint8_t readRegister(uint16_t address);
    void writeRegister(uint16_t address, uint8_t value);

private:
    DMG_MMU& mmu;
    SDL_AudioStream* audioStream = nullptr;

    PulseChannel ch1 { .hasSweep = true }; // pulse + sweep, NR10 -NR14
    PulseChannel ch2 { .hasSweep = false }; // pulse, NR21 - NR24
    WaveChannel wave; // wave, NR30 - NR34 + Wave RAM
    NoiseChannel noise; // noise, NR41 - NR44

    Mixer mixer;
    FrameSequencer frame;
    AudioOutput output;

    APURegisters registers;

    uint16_t FrameCounterStep = 8192;

    void pushAudio();

    // pulse
    void stepPulseChannel(PulseChannel&);

    template<typename T>
    void clockLength(T& channel);

    void clockLengthCounters();
    void clockSweep();
    void clockEnvelopes();

    template<typename T>
    void clockEnvelope(T& channel);

    // shared trigger boilerplate (DAC-off disables channel, length reload-on-zero);
    // returns false if the channel was disabled by the DAC-off check
    template<typename T>
    bool triggerCommon(T& channel, uint16_t lengthMax) {
        if (!channel.state.dacEnabled) {
            channel.state.enabled = false;
            return false;
        }
        channel.state.enabled = true;
        if (channel.length.counter == 0)
            channel.length.counter = lengthMax;
        return true;
    }

    template<typename T>
    void resetEnvelope(T& channel) {
        channel.envelope.volumeCurrent = channel.envelope.volumeInitial;
        channel.envelope.timer.remaining = channel.envelope.timer.period;
    }

    uint8_t pulseOutput(PulseChannel& channel);

    void triggerPulse(PulseChannel& channel);
    uint16_t calculateSweepFrequency();

    // wave
    void stepWaveChannel();
    uint8_t waveOutput();
    void triggerWave();

    // noise
    void stepNoiseChannel();
    uint8_t noiseOutput();
    void triggerNoise();

    // mixer
    void mixSample(int16_t& left, int16_t& right);
    uint8_t channelOutput(uint8_t channel);
};

#endif