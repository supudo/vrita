#include "apu.hpp"

uint8_t DMG_APU::readRegister(uint16_t address) {
    switch (address) {
        // CH1
        case 0xFF10: return ch1.NRx0 | 0x80;
        case 0xFF11: return ch1.NRx1 | 0x3F;
        case 0xFF12: return ch1.NRx2;
        case 0xFF13: return ch1.NRx3 | 0xFF;
        case 0xFF14: return ch1.NRx4 | 0xBF;
        // CH2
        case 0xFF16: return ch2.NRx1 | 0x3F;
        case 0xFF17: return ch2.NRx2;
        case 0xFF18: return ch2.NRx3 | 0xFF;
        case 0xFF19: return ch2.NRx4 | 0xBF;
        // Wave
        case 0xFF1A: return wave.NR30 | 0x7F;
        case 0xFF1B: return wave.NR31 | 0xFF;
        case 0xFF1C: return wave.NR32 | 0x9F;
        case 0xFF1D: return wave.NR33 | 0xFF;
        case 0xFF1E: return wave.NR34 | 0xBF;
        // Noise
        case 0xFF20: return noise.NR41 | 0xFF;
        case 0xFF21: return noise.NR42;
        case 0xFF22: return noise.NR43;
        case 0xFF23: return noise.NR44 | 0xBF;
        // Mixer
        case 0xFF24: return registers.NR50;
        case 0xFF25: return registers.NR51;
        case 0xFF26: {
            uint8_t value = 0x70;
            if (registers.NR52 & 0x80)
                value |= 0x80;
            if (ch1.state.enabled)
                value |= 0x01;
            if (ch2.state.enabled)
                value |= 0x02;
            if (wave.state.enabled)
                value |= 0x04;
            if (noise.state.enabled)
                value |= 0x08;
            return value;
        }
        // Wave RAM
        default:
            if (address >= 0xFF30 && address <= 0xFF3F)
                return wave.waveRAM[address - 0xFF30];
            break;
    }
    return 0xFF;
}

void DMG_APU::writeRegister(uint16_t address, uint8_t value) {
    bool isWaveRAM = address >= 0xFF30 && address <= 0xFF3F;
    if (!(registers.NR52 & 0x80) && address != 0xFF26 && !isWaveRAM)
        return;

    switch (address) {
        // CH1
        case 0xFF10:
            ch1.NRx0 = value;
            ch1.sweep.period = (value >> 4) & 0x7;
            ch1.sweepNegate = value & 0x08;
            ch1.sweepShift = value & 0x07;
            break;
        case 0xFF11:
            ch1.NRx1 = value;
            ch1.duty = value >> 6;
            ch1.length.counter = 64 - (value & 0x3F);
            break;
        case 0xFF12:
            ch1.NRx2 = value;
            ch1.state.dacEnabled = (value & 0xF8) != 0;
            if (!ch1.state.dacEnabled)
                ch1.state.enabled = false;
            ch1.envelope.volumeInitial = value >> 4;
            ch1.envelope.increase = value & 0x08;
            ch1.envelope.timer.period = value & 0x07;
            break;
        case 0xFF13:
            ch1.NRx3 = value;
            ch1.frequency.frequency = (ch1.frequency.frequency & 0x700) | value;
            break;
        case 0xFF14:
            ch1.NRx4 = value;
            ch1.frequency.frequency = (ch1.frequency.frequency & 0xFF) | ((value & 0x07) << 8);
            ch1.length.enabled = value & 0x40;
            if (value & 0x80)
                triggerPulse(ch1);
            break;
        // CH2
        case 0xFF16:
            ch2.NRx1 = value;
            ch2.duty = value >> 6;
            ch2.length.counter = 64 - (value & 0x3F);
            break;
        case 0xFF17:
            ch2.NRx2 = value;
            ch2.state.dacEnabled = (value & 0xF8) != 0;
            if (!ch2.state.dacEnabled)
                ch2.state.enabled = false;
            ch2.envelope.volumeInitial = value >> 4;
            ch2.envelope.increase = value & 0x08;
            ch2.envelope.timer.period = value & 0x07;
            break;
        case 0xFF18:
            ch2.NRx3 = value;
            ch2.frequency.frequency = (ch2.frequency.frequency & 0x700) | value;
            break;
        case 0xFF19:
            ch2.NRx4 = value;
            ch2.frequency.frequency = (ch2.frequency.frequency & 0xFF) | ((value & 0x07) << 8);
            ch2.length.enabled = value & 0x40;
            if (value & 0x80)
                triggerPulse(ch2);
            break;
        // Mixer
        case 0xFF24:
            registers.NR50 = value;
            mixer.volumeLeft = (value >> 4) & 7;
            mixer.volumeRight = value & 7;
            mixer.vinLeft = value & 0x80;
            mixer.vinRight = value & 0x08;
            break;
        case 0xFF25:
            registers.NR51 = value;
            for (int i = 0; i < 4; i++) {
                mixer.enabledRight[i] = value & (1 << i);
                mixer.enabledLeft[i] = value & (1 << (i + 4));
            }
            break;
        case 0xFF26:
            if (!(value & 0x80))
                powerOff();
            registers.NR52 = value & 0x80;
            break;
        // Wave
        case 0xFF1A:
            wave.NR30 = value;
            wave.state.dacEnabled = value & 0x80;
            if (!wave.state.dacEnabled)
                wave.state.enabled = false;
            break;
        case 0xFF1B:
            wave.NR31 = value;
            wave.length.counter = 256 - value;
            break;
        case 0xFF1C:
            wave.NR32 = value;
            wave.volumeCode = (value >> 5) & 0x03;
            break;
        case 0xFF1D:
            wave.NR33 = value;
            wave.frequency.frequency = (wave.frequency.frequency & 0x700) | value;
            break;
        case 0xFF1E:
            wave.NR34 = value;
            wave.frequency.frequency = (wave.frequency.frequency & 0xFF) | ((value & 0x07) << 8);
            wave.length.enabled = value & 0x40;
            if (value & 0x80)
                triggerWave();
            break;
        // Noise
        case 0xFF20:
            noise.NR41 = value;
            noise.length.counter = 64 - (value & 0x3F);
            break;
        case 0xFF21:
            noise.NR42 = value;
            noise.state.dacEnabled = (value & 0xF8) != 0;
            if (!noise.state.dacEnabled)
                noise.state.enabled = false;
            noise.envelope.volumeInitial = value >> 4;
            noise.envelope.increase = value & 0x08;
            noise.envelope.timer.period = value & 0x07;
            break;
        case 0xFF22:
            noise.NR43 = value;
            noise.clockShift = value >> 4;
            noise.widthMode = value & 0x08;
            noise.divisorCode = value & 0x07;
            break;
        case 0xFF23:
            noise.NR44 = value;
            noise.length.enabled = value & 0x40;
            if (value & 0x80)
                triggerNoise();
            break;
        default:
            if (address >= 0xFF30 && address <= 0xFF3F)
                wave.waveRAM[address - 0xFF30] = value;
            break;
    }
}