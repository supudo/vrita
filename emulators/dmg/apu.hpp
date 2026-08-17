/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_APU_INCLUDES
#define VRITA_DMG_APU_INCLUDES

#include <SDL2/SDL.h>
#include "apu_structs.hpp"
#include "mmu.hpp"
#include "utilities/logger.hpp"

class DMG_APU {
public:
    DMG_APU(Logger& logger, DMG_MMU& mmu) : logger(logger), mmu(mmu) {}

    void initAudioDevice(SDL_AudioDeviceID device);
    void step(bool ROMFileLoaded, uint32_t cycles);
    void clearResources();
    void powerOff();

    uint8_t readRegister(uint16_t address);
    void writeRegister(uint16_t address, uint8_t value);

    void setUserVolume(uint8_t volume);
    uint8_t getUserVolume() const;
    void setMuted(bool muted);
    bool isMuted() const;
    uint8_t channelOutput(uint8_t channel) const;

    const PulseChannel& getChannel1() const { return ch1; }
    const PulseChannel& getChannel2() const { return ch2; }
    const WaveChannel& getChannelWave() const { return wave; }
    const NoiseChannel& getChannelNoise() const { return noise; }

private:
    Logger& logger;
    DMG_MMU& mmu;
    SDL_AudioDeviceID audioDevice = 0;

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

    template<typename T>
    void extraClockLengthIfNeeded(T& channel) {
        if ((frame.step & 1) == 0)
            return;
        if (!channel.length.enabled || channel.length.counter == 0)
            return;
        channel.length.counter--;
        if (channel.length.counter == 0)
            channel.state.enabled = false;
    }

    template<typename T>
    void triggerCommon(T& channel, uint16_t lengthMax) {
        if (channel.length.counter == 0) {
            channel.length.counter = lengthMax;
            extraClockLengthIfNeeded(channel);
        }
        channel.state.enabled = channel.state.dacEnabled;
    }

    template<typename T>
    void resetEnvelope(T& channel) {
        channel.envelope.volumeCurrent = channel.envelope.volumeInitial;
        channel.envelope.timer.remaining = channel.envelope.timer.period;
    }

    uint8_t pulseOutput(const PulseChannel& channel) const;

    void triggerPulse(PulseChannel& channel);
    uint16_t calculateSweepFrequency();

    // wave
    void stepWaveChannel();
    uint8_t waveOutput() const;
    void triggerWave();

    // noise
    void stepNoiseChannel();
    uint8_t noiseOutput() const;
    void triggerNoise();

    // mixer
    void mixSample(int16_t& left, int16_t& right);
};

#endif