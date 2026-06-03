/*

GameBoy (DMG)

*/

#include "mmu.hpp"

#ifndef VRITA_DMG_APU_INCLUDES
#define VRITA_DMG_APU_INCLUDES

class DMG_APU {
public:
    DMG_APU(DMG_MMU& mmu) : mmu(mmu) {}

    void stepAPU(bool ROMFileLoaded);
    void clearResources();

private:
    DMG_MMU& mmu;
};

#endif