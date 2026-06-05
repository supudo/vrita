/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_CARTRIDGE_INCLUDES
#define VRITA_DMG_CARTRIDGE_INCLUDES

#include "mbc.hpp"
#include "mmu.hpp"
#include "utilities/logger.hpp"

class DMG_CARTRIDGE {
public:
    DMG_CARTRIDGE(Logger& logger, DMG_MMU& mmu) : logger(logger), mmu(mmu) {}

    void clearResources();

    void loadROM(std::streamsize size);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

private:
    Logger& logger;
    DMG_MMU& mmu;

    std::unique_ptr<DMG_MBC> mbc;
    std::vector<uint8_t> ram;

    std::string romTitle;
    bool cgbGame = false;
    uint8_t mbcType = 0x0;
    int romBanksCount = 0;
    int ramBanksCount = 0;

    int getRamBanksCount(uint8_t type);
    void printCartridgeInfo();
};

#endif