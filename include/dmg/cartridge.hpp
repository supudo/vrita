/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_CARTRIDGE_INCLUDES
#define VRITA_DMG_CARTRIDGE_INCLUDES

#include "mbc.hpp"
#include "../logger.hpp"

class DMG_CARTRIDGE {
private:
    std::unique_ptr<DMG_MBC> mbc;
    std::vector<uint8_t> ram;

    std::string rom_title;
    bool cgb_game = false;
    uint8_t mbc_type = 0x0;
    int rom_banks_count;
    int ram_banks_count;

public:
    DMG_CARTRIDGE(DMG_MBC* mbc) : mbc(mbc) {}
    DMG_CARTRIDGE(uint8_t* rom, size_t romSize, Logger *logger) : ram(0x8000, 0) {
        uint8_t type = rom[0x147];
        switch (type) {
            case 0x00:
                mbc = std::make_unique<DMG_MBC0>(rom, romSize, ram);
                break;
            case 0x01:
            case 0x02:
            case 0x03:
                mbc = std::make_unique<DMG_MBC1>(rom, romSize, ram);
                break;
            case 0x05:
            case 0x06:
                mbc = std::make_unique<DMG_MBC2>(rom, romSize, ram);
                break;
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                mbc = std::make_unique<DMG_MBC3>(rom, romSize, ram);
                break;
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
                mbc = std::make_unique<DMG_MBC5>(rom, romSize, ram);
                break;
            default:
                logger->log("[CARTRIDGE] Unsupported cartridge type");
        }
        rom_banks_count = (int)(romSize / 0x4000);
        ram_banks_count = get_ram_banks_count(rom[0x149]);
        rom_title = std::string(rom + 0x134, rom + 0x143);
        mbc_type = rom[0x147];
        printCartridgeInfo(logger);
    }
    uint8_t read(uint16_t addr) {
        return mbc->read(addr);
    }
    void write(uint16_t addr, uint8_t value) {
        mbc->write(addr, value);
    }
    int get_ram_banks_count(uint8_t type) {
        switch (type) {
            case 0x00:
                return 0;
                break;
            case 0x01:
                return 0;
                break;
            case 0x02:
                return 1;
                break;
            case 0x03:
                return 4;
                break;
            case 0x04:
                return 16;
                break;
            case 0x05:
                return 8;
                break;
            default:
                std::cout << "Incorrect RAM type: " << type << std::endl;
                exit(1);
        }
    }
    void printCartridgeInfo(Logger* logger) {
        logger->log("Rom Title: %s", rom_title.c_str());
        logger->log("CGB Game: %s", (cgb_game ? "Yes" : "No"));
        logger->log("MBC: %i", +mbc_type);
        logger->log("ROM Banks: %i", rom_banks_count);
        logger->log("RAM Banks: %i", ram_banks_count);
    }
};

#endif