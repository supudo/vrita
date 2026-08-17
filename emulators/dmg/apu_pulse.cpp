#include "apu.hpp"

void DMG_APU::stepPulseChannel(PulseChannel& channel) {
    if (!channel.state.enabled)
        return;

    if (channel.frequency.timer == 0) {
        channel.frequency.timer = frequencyTimerReload(channel.frequency.frequency, 4);
        channel.dutyPosition++;
        channel.dutyPosition &= 7;
    }
    else
        channel.frequency.timer--;
}

uint8_t DMG_APU::pulseOutput(const PulseChannel& channel) const {
    if (!channel.state.enabled || !channel.state.dacEnabled)
        return 0;
    uint8_t bit = dutyTable[channel.duty][channel.dutyPosition];
    if (!bit)
        return 0;
    return channel.envelope.volumeCurrent;
}

void DMG_APU::clockLengthCounters() {
    clockLength(ch1);
    clockLength(ch2);
    clockLength(wave);
    clockLength(noise);
}

template<typename T>
void DMG_APU::clockLength(T& channel) {
    if (!channel.length.enabled)
        return;
    if (channel.length.counter > 0) {
        channel.length.counter--;
        if (channel.length.counter == 0)
            channel.state.enabled = false;
    }
}

void DMG_APU::clockEnvelopes() {
    clockEnvelope(ch1);
    clockEnvelope(ch2);
    clockEnvelope(noise);
}

template<typename T>
void DMG_APU::clockEnvelope(T& channel) {
    if (!channel.state.enabled || channel.envelope.timer.period == 0)
        return;
    if (--channel.envelope.timer.remaining == 0) {
        channel.envelope.timer.remaining = channel.envelope.timer.period;
        if (channel.envelope.increase) {
            if (channel.envelope.volumeCurrent < 15)
                channel.envelope.volumeCurrent++;
        }
        else {
            if (channel.envelope.volumeCurrent > 0)
                channel.envelope.volumeCurrent--;
        }
    }
}

void DMG_APU::clockSweep() {
    if (!ch1.hasSweep || !ch1.state.enabled)
        return;
    if (ch1.sweep.remaining > 0)
        ch1.sweep.remaining--;
    if (ch1.sweep.remaining != 0)
        return;
    ch1.sweep.remaining = ch1.sweep.period != 0 ? ch1.sweep.period : 8;
    if (!ch1.sweepEnabled || ch1.sweep.period == 0)
        return;
    uint16_t newFrequency = calculateSweepFrequency();
    if (newFrequency > 2047) {
        ch1.state.enabled = false;
        return;
    }
    if (ch1.sweepShift != 0) {
        ch1.shadowFrequency = newFrequency;
        ch1.frequency.frequency = newFrequency;
        if (calculateSweepFrequency() > 2047)
            ch1.state.enabled = false;
    }
}

uint16_t DMG_APU::calculateSweepFrequency() {
    uint16_t offset = ch1.shadowFrequency >> ch1.sweepShift;
    if (ch1.sweepNegate) {
        ch1.didNegateCalculation = true;
        return ch1.shadowFrequency - offset;
    }
    return ch1.shadowFrequency + offset;
}

void DMG_APU::triggerPulse(PulseChannel& channel) {
    triggerCommon(channel, 64);
    channel.frequency.timer = frequencyTimerReload(channel.frequency.frequency, 4);
    channel.dutyPosition = 0;
    resetEnvelope(channel);
    if (channel.hasSweep) {
        channel.shadowFrequency = channel.frequency.frequency;
        channel.didNegateCalculation = false;
        channel.sweep.remaining = channel.sweep.period != 0 ? channel.sweep.period : 8;
        channel.sweepEnabled = (channel.sweep.period != 0 || channel.sweepShift != 0);
        if (channel.sweepShift != 0 && calculateSweepFrequency() > 2047)
            channel.state.enabled = false;
    }
}