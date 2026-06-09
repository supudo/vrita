#include "apu.hpp"

void DMG_APU::clearResources() {
}

void DMG_APU::step(bool ROMFileLoaded, uint32_t cycles) {
    if (!ROMFileLoaded) return;
}