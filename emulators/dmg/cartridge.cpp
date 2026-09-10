#include "cartridge.hpp"

void DMG_CARTRIDGE::loadROM(bool isCGB, std::streamsize size) {
    romImage.assign(mmu.memory.data(), mmu.memory.data() + size);
    uint8_t type = mmu.memory[addressCartridgeType]; // cartridge number
    logger.log("[%s-CARTRIDGE] Cartridge type byte: 0x%02X", isCGB ? "CGB" : "DMG", type);

    romBanksCount = (int)(size / 0x4000);
    ramBanksCount = getRamBanksCount(isCGB, mmu.memory[0x149]);

    romHeader.cgbFlag = mmu.memory[0x143];
    if (isCGBOnly() && !isCGB)
        logger.log("[%s-CARTRIDGE] ROM requires CGB, forced DMG.", isCGB ? "CGB" : "DMG");
    if (!supportsCGB() && isCGB)
        logger.log("[%s-CARTRIDGE] DMG ROM running on CGB.", isCGB ? "CGB" : "DMG");

    romHeader.sgbFlag = mmu.memory[0x146];
    romHeader.cartridgeType = mmu.memory[0x147];
    romHeader.romSize = mmu.memory[0x148];

    romHeader.ramSize = mmu.memory[0x149];
    ram.resize(getRamSize(romHeader.ramSize));

    romHeader.destinationCode = mmu.memory[0x14A];
    romHeader.oldLicenseeCode = mmu.memory[0x14B];
    romHeader.version = mmu.memory[0x14C];
    romHeader.title = readHeaderString(romImage, 0x134, 15);
    romHeader.manufacturerCode = readHeaderString(romImage, 0x13F, 4);

    printCartridgeInfo(isCGB);

    switch (type) {
        case 0x00:
            mbc = std::make_unique<DMG_MBC0>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC0 (ROM only, 32KB)", isCGB ? "CGB" : "DMG");
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            mbc = std::make_unique<DMG_MBC1>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC1", isCGB ? "CGB" : "DMG");
            break;
        case 0x05:
        case 0x06:
            mbc = std::make_unique<DMG_MBC2>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC2", isCGB ? "CGB" : "DMG");
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            mbc = std::make_unique<DMG_MBC3>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC3", isCGB ? "CGB" : "DMG");
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            mbc = std::make_unique<DMG_MBC5>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC5", isCGB ? "CGB" : "DMG");
            break;
        case 0x20:
            mbc = std::make_unique<DMG_MBC6>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC6", isCGB ? "CGB" : "DMG");
            break;
        case 0x22:
            mbc = std::make_unique<DMG_MBC7>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: MBC7", isCGB ? "CGB" : "DMG");
            break;
        case 0xFE:
            mbc = std::make_unique<DMG_HuC3>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: HuC3", isCGB ? "CGB" : "DMG");
            break;
        case 0xFF:
            mbc = std::make_unique<DMG_HuC1>(logger, romImage.data(), size, ram);
            logger.log("[%s-CARTRIDGE] MBC: HuC1", isCGB ? "CGB" : "DMG");
            break;
        default:
            logger.log("[%s-CARTRIDGE] Unsupported cartridge type 0x%02X", isCGB ? "CGB" : "DMG", type);
    }
}

bool DMG_CARTRIDGE::supportsCGB() const {
    return (romHeader.cgbFlag & CONST_CGBFlagSupported) != 0;
}

bool DMG_CARTRIDGE::isCGBOnly() const {
    return romHeader.cgbFlag == CONST_CGBFlagOnly;
}

std::string DMG_CARTRIDGE::readHeaderString(const std::vector<uint8_t>& rom, size_t offset, size_t length) {
    std::string result;
    for (size_t i = 0; i < length; ++i) {
        uint8_t c = rom[offset + i];
        if (c == 0)
            break;
        result.push_back(static_cast<char>(c));
    }
    return result;
}

size_t DMG_CARTRIDGE::getRamSize(uint8_t ramSizeCode) {
    switch (ramSizeCode) {
        case 0x00: return 0;
        case 0x01: return 0x0800;
        case 0x02: return 0x2000;
        case 0x03: return 0x8000;
        case 0x04: return 0x20000;
        case 0x05: return 0x10000;
        default: return 0;
    }
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

int DMG_CARTRIDGE::getRamBanksCount(bool isCGB, uint8_t type) {
    switch (type) {
        case 0x00: return 0;
        case 0x01: return 0;
        case 0x02: return 1;
        case 0x03: return 4;
        case 0x04: return 16;
        case 0x05: return 8;
        default:
            logger.log("[%s-CARTRIDGE] Unknown RAM type 0x%02X, assuming 0 banks", isCGB ? "CGB" : "DMG", type);
            return 0;
    }
}

void DMG_CARTRIDGE::printCartridgeInfo(bool isCGB) {
    logger.log("[%s-CARTRIDGE] Rom Title: %s", isCGB ? "CGB" : "DMG", romHeader.title.c_str());
    logger.log("[%s-CARTRIDGE] Manufacturer Code: %s", isCGB ? "CGB" : "DMG", romHeader.manufacturerCode.c_str());
    logger.log("[%s-CARTRIDGE] MBC: %i", isCGB ? "CGB" : "DMG", +romHeader.cartridgeType);
    logger.log("[%s-CARTRIDGE] ROM Banks: %i", isCGB ? "CGB" : "DMG", romBanksCount);
    logger.log("[%s-CARTRIDGE] RAM Banks: %i", isCGB ? "CGB" : "DMG", ramBanksCount);
}
