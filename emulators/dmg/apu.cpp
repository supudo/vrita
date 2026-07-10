#include "apu.hpp"

void DMG_APU::initAudioStream(SDL_AudioStream* audioStream) {
    this->audioStream = audioStream;
}

void DMG_APU::clearResources() {
}

void DMG_APU::step(bool ROMFileLoaded, uint32_t cycles) {
    if (!ROMFileLoaded)
        return;

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

        // sample generation
        output.sampleAccumulator += AudioOutput::sampleRate;
        if (output.sampleAccumulator >= AudioOutput::cpuClock) {
            output.sampleAccumulator -= AudioOutput::cpuClock;
            int16_t left, right;
            mixSample(left, right);
            output.buffer.push_back(left);
            output.buffer.push_back(right);
        }
    }

    pushAudio();
}

void DMG_APU::pushAudio() {
    if (!audioStream || output.buffer.empty())
        return;
    SDL_PutAudioStreamData(audioStream, output.buffer.data(), output.buffer.size() * sizeof(int16_t));
    output.buffer.clear();
}