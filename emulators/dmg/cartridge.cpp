#include "cartridge.hpp"

void DMG_CARTRIDGE::loadROM(std::streamsize size) {
    uint8_t type = mmu.memory[0x147];
    switch (type) {
        case 0x00:
            mbc = std::make_unique<DMG_MBC0>(mmu.memory, size, ram);
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            mbc = std::make_unique<DMG_MBC1>(mmu.memory, size, ram);
            break;
        case 0x05:
        case 0x06:
            mbc = std::make_unique<DMG_MBC2>(mmu.memory, size, ram);
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            mbc = std::make_unique<DMG_MBC3>(mmu.memory, size, ram);
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc = std::make_unique<DMG_MBC5>(mmu.memory, size, ram);
            break;
        default:
            logger.log("[CARTRIDGE] Unsupported cartridge type 0x2%", type);
    }
    rom_banks_count = (int)(size / 0x4000);
    ram_banks_count = get_ram_banks_count(mmu.memory[0x149]);
    const char* titlePtr = reinterpret_cast<const char*>(mmu.memory + 0x134);
    rom_title = std::string(titlePtr, strnlen(titlePtr, 16));
    mbc_type = mmu.memory[0x147]; // the address of the mbc type - 0x147
    printCartridgeInfo();
}

uint8_t DMG_CARTRIDGE::read(uint16_t addr) {
    return mbc->read(addr);
}

void DMG_CARTRIDGE::write(uint16_t addr, uint8_t value) {
    mbc->write(addr, value);
}

int DMG_CARTRIDGE::get_ram_banks_count(uint8_t type) {
    switch (type) {
        case 0x00:
            return 0;
        case 0x01:
            return 0;
        case 0x02:
            return 1;
        case 0x03:
            return 4;
        case 0x04:
            return 16;
        case 0x05:
            return 8;
        default:
            logger.log("Incorrect RAM type:  %i", type);
            exit(1);
    }
}

void DMG_CARTRIDGE::printCartridgeInfo() {
    logger.log("Rom Title: %s", rom_title.c_str());
    logger.log("CGB Game: %s", (cgb_game ? "Yes" : "No"));
    logger.log("MBC: %i", +mbc_type);
    logger.log("ROM Banks: %i", rom_banks_count);
    logger.log("RAM Banks: %i", ram_banks_count);
}
