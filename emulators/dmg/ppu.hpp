/*

GameBoy (DMG)

*/

#include "mmu.hpp"

#ifndef VRITA_DMG_PPU_INCLUDES
#define VRITA_DMG_PPU_INCLUDES

class DMG_PPU {
public:
    DMG_PPU(DMG_MMU& mmu) : mmu(mmu) {}

    void stepPPU(bool ROMFileLoaded);
    void clearResources();

private:
    DMG_MMU& mmu;
};

#endif