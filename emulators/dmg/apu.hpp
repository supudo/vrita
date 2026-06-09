/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_APU_INCLUDES
#define VRITA_DMG_APU_INCLUDES

#include "mmu.hpp"

class DMG_APU {
public:
    DMG_APU(DMG_MMU& mmu) : mmu(mmu) {}

    void step(bool ROMFileLoaded, uint32_t cycles);
    void clearResources();

private:
    DMG_MMU& mmu;
};

#endif