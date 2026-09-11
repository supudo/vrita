/*

GameBoy (DMG,CGB)

*/

#ifndef VRITA_DMG_CARTRIDGE_INCLUDES
#define VRITA_DMG_CARTRIDGE_INCLUDES

#include "emulators/dmg/mbc.hpp"
#include "emulators/dmg/mmu.hpp"
#include "utilities/logger.hpp"
#include "emulators/dmg/cartridge_defines.hpp"

class DMG_CARTRIDGE {
public:
    DMG_CARTRIDGE(Logger& logger, DMG_MMU& mmu) : logger(logger), mmu(mmu) {}

    void clearResources();

    void loadROM(bool isCGB, std::streamsize size); // isCGB is the gui var, not the rom value
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

    const uint8_t* romImageData() const { return romImage.data(); }
    size_t romImageSize() const { return romImage.size(); }

    uint16_t currentRomBank() const { return mbc ? mbc->currentRomBank() : 0; }
    uint16_t totalRomBanks() const { return mbc ? mbc->totalRomBanks() : 0; }

    CartridgeHeader romHeader;
    bool supportsCGB() const;
    bool isCGBOnly() const;

private:
    Logger& logger;
    DMG_MMU& mmu;

    std::unique_ptr<DMG_MBC> mbc;
    std::vector<uint8_t> ram;
    std::vector<uint8_t> romImage;

    int romBanksCount = 0;
    int ramBanksCount = 0;

    uint16_t addressCartridgeType = 0x147;

    int getRamBanksCount(bool isCGB, uint8_t type);
    void printCartridgeInfo(bool isCGB);

    size_t getRamSize(uint8_t ramSizeCode);
    std::string readHeaderString(const std::vector<uint8_t>& rom, size_t offset, size_t length);
};

#endif