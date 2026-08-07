#include "apu.hpp"

void DMG_APU::initAudioDevice(SDL_AudioDeviceID device) {
    this->audioDevice = device;
}

void DMG_APU::clearResources() {
    ch1 = PulseChannel{ .hasSweep = true };
    ch2 = PulseChannel{ .hasSweep = false };
    wave = WaveChannel{};
    noise = NoiseChannel{};
    uint8_t userVolume = mixer.userVolume;
    bool userMuted = mixer.userMuted;
    mixer = Mixer{};
    mixer.userVolume = userVolume;
    mixer.userMuted = userMuted;
    frame = FrameSequencer{};
    output.buffer.clear();
    output.sampleAccumulator = 0;
    output.oversampleSumLeft = 0;
    output.oversampleSumRight = 0;
    output.oversampleCount = 0;
    registers = APURegisters{};
}

void DMG_APU::powerOff() {
    ch1 = PulseChannel{ .hasSweep = true };
    ch2 = PulseChannel{ .hasSweep = false };
    
    auto waveRAM = wave.waveRAM;
    wave = WaveChannel{};
    wave.waveRAM = waveRAM;

    uint8_t nr41 = noise.NR41;
    uint16_t lengthCounter = noise.length.counter;
    noise = NoiseChannel{};
    noise.NR41 = nr41;
    noise.length.counter = lengthCounter;

    uint8_t userVolume = mixer.userVolume;
    bool userMuted = mixer.userMuted;
    mixer = Mixer{};
    mixer.userVolume = userVolume;
    mixer.userMuted = userMuted;
    
    registers.NR50 = 0;
    registers.NR51 = 0;
}

void DMG_APU::step(bool ROMFileLoaded, uint32_t cycles) {
    if (!ROMFileLoaded)
        return;
#ifdef TRACY_ENABLE
    ZoneScopedN("APU::step");
#endif

    if (!registers.NR52)
        return;

    while (cycles--) {
        // clock each channel's frequency timer
        stepPulseChannel(ch1);
        stepPulseChannel(ch2);
        stepWaveChannel();
        stepNoiseChannel();

        // 512 Hz frame sequencer
        if (--frame.counter == 0) {
            frame.counter = FrameCounterStep;

            if (frame.step % 2 == 0)
                clockLengthCounters();

            if (frame.step == 2 || frame.step == 6)
                clockSweep();

            if (frame.step == 7)
                clockEnvelopes();

            frame.step = (frame.step + 1) & 7;
        }

        output.sampleAccumulator += AudioOutput::sampleRate * AudioOutput::oversample;
        if (output.sampleAccumulator >= AudioOutput::cpuClock) {
            output.sampleAccumulator -= AudioOutput::cpuClock;
            int16_t left, right;
            mixSample(left, right);
            output.oversampleSumLeft += left;
            output.oversampleSumRight += right;
            if (++output.oversampleCount >= AudioOutput::oversample) {
                output.buffer.push_back(static_cast<int16_t>(output.oversampleSumLeft / (int32_t)AudioOutput::oversample));
                output.buffer.push_back(static_cast<int16_t>(output.oversampleSumRight / (int32_t)AudioOutput::oversample));
                output.oversampleSumLeft = 0;
                output.oversampleSumRight = 0;
                output.oversampleCount = 0;
            }
        }
    }

    pushAudio();
}

void DMG_APU::pushAudio() {
    if (!audioDevice || output.buffer.empty())
        return;
#ifdef TRACY_ENABLE
    TracyPlot("APU buffer size", (int64_t)output.buffer.size());
#endif
    SDL_QueueAudio(audioDevice, output.buffer.data(), (Uint32)(output.buffer.size() * sizeof(int16_t)));
    output.buffer.clear();
}