#include "cartridge.hpp"

void DMG_CARTRIDGE::loadROM(std::streamsize size) {
    ram.assign(0x8000, 0);
    uint8_t type = mmu.memory[0x147]; // cartridge number
    logger.log("[DMG-CARTRIDGE] Cartridge type byte: 0x%02X", type);
    switch (type) {
        case 0x00:
            mbc = std::make_unique<DMG_MBC0>(mmu.memory.data(), size, ram);
            logger.log("[DMG-CARTRIDGE] MBC: MBC0 (ROM only, 32KB)");
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            mbc = std::make_unique<DMG_MBC1>(mmu.memory.data(), size, ram);
            logger.log("[DMG-CARTRIDGE] MBC: MBC1");
            break;
        case 0x05:
        case 0x06:
            mbc = std::make_unique<DMG_MBC2>(mmu.memory.data(), size, ram);
            logger.log("[DMG-CARTRIDGE] MBC: MBC2");
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            mbc = std::make_unique<DMG_MBC3>(mmu.memory.data(), size, ram);
            logger.log("[DMG-CARTRIDGE] MBC: MBC3");
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc = std::make_unique<DMG_MBC5>(mmu.memory.data(), size, ram);
            logger.log("[DMG-CARTRIDGE] MBC: MBC5");
            break;
        default:
            logger.log("[DMG-CARTRIDGE] Unsupported cartridge type 0x%02X", type);
    }
    romBanksCount = (int)(size / 0x4000);
    ramBanksCount = getRamBanksCount(mmu.memory[0x149]);
    
    // title
    const char* titlePtr = reinterpret_cast<const char*>(mmu.memory.data() + 0x134);
    romTitle = std::string(titlePtr, strnlen(titlePtr, 16));

    // manufacturer code
    const char* manufacturerCodePtr = reinterpret_cast<const char*>(mmu.memory.data() + 0x13F);
    romManufacturerCode = std::string(manufacturerCodePtr, strnlen(manufacturerCodePtr, 4));

    mbcType = mmu.memory[0x147]; // the address of the mbc type - 0x147
    printCartridgeInfo();
}

void DMG_CARTRIDGE::clearResources() {
    mbc.reset();
}

uint8_t DMG_CARTRIDGE::read(uint16_t addr) {
    if (!mbc)
        return 0xFF;
    return mbc->read(addr);
}

void DMG_CARTRIDGE::write(uint16_t addr, uint8_t value) {
    if (!mbc)
        return;
    mbc->write(addr, value);
}

int DMG_CARTRIDGE::getRamBanksCount(uint8_t type) {
    switch (type) {
        case 0x00: return 0;
        case 0x01: return 0;
        case 0x02: return 1;
        case 0x03: return 4;
        case 0x04: return 16;
        case 0x05: return 8;
        default:
            logger.log("[DMG-CARTRIDGE] Unknown RAM type 0x%02X, assuming 0 banks", type);
            return 0;
    }
}

void DMG_CARTRIDGE::printCartridgeInfo() {
    logger.log("[DMG-CARTRIDGE] Rom Title: %s", romTitle.c_str());
    logger.log("[DMG-CARTRIDGE] Manufacturer Code: %s", romManufacturerCode.c_str());
    logger.log("[DMG-CARTRIDGE] CGB Game: %s", (cgbGame ? "Yes" : "No"));
    logger.log("[DMG-CARTRIDGE] MBC: %i", +mbcType);
    logger.log("[DMG-CARTRIDGE] ROM Banks: %i", romBanksCount);
    logger.log("[DMG-CARTRIDGE] RAM Banks: %i", ramBanksCount);
}
