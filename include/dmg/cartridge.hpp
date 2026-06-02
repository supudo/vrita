/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_CARTRIDGE_INCLUDES
#define VRITA_DMG_CARTRIDGE_INCLUDES

#include "mbc.hpp"
#include "../logger.hpp"

class DMG_CARTRIDGE {
public:
    DMG_CARTRIDGE(uint8_t* rom, size_t romSize, Logger* logger);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

private:
    Logger* logger = nullptr;

    std::unique_ptr<DMG_MBC> mbc;
    std::vector<uint8_t> ram;

    std::string rom_title;
    bool cgb_game = false;
    uint8_t mbc_type = 0x0;
    int rom_banks_count;
    int ram_banks_count;

    int get_ram_banks_count(uint8_t type);
    void printCartridgeInfo();
};

#endif