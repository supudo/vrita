/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_MBC_INCLUDES
#define VRITA_DMG_MBC_INCLUDES

#include <stdint.h>
#include <vector>
#include "utilities/logger.hpp"

class DMG_MBC {
public:
    DMG_MBC(Logger& logger, uint8_t* rom, size_t romSize, std::vector<uint8_t>& ram) : logger(logger), rom(rom), romSize(romSize), ram(ram) {}
    virtual ~DMG_MBC() = default;
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t value) = 0;
    virtual uint16_t currentRomBank() const = 0;
    uint16_t totalRomBanks() const { return static_cast<uint16_t>(romSize / 0x4000); }

protected:
    Logger& logger;
    uint8_t* rom;
    size_t romSize;
    std::vector<uint8_t>& ram;
};

class DMG_MBC0 : public DMG_MBC {
public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        return rom[addr];
    }
    void write(uint16_t, uint8_t) override { /* read-only */ }
    uint16_t currentRomBank() const { return 1; }
};

class DMG_MBC1 : public DMG_MBC {
private:
    uint8_t romBank = 1;
    uint8_t ramBank = 0;
    bool ramEnabled = false;
    bool bankingMode = 0;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000) {
            uint32_t bank = (bankingMode ? (ramBank << 5) : 0);
            return rom[(bank * 0x4000) + addr];
        }
        else {
            uint32_t bank = romBank | (bankingMode ? (ramBank << 5) : 0);
            return rom[(bank * 0x4000) + (addr - 0x4000)];
        }
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000) {
            bool en = ((value & 0x0F) == 0x0A);
            ramEnabled = en;
        }
        else if (addr < 0x4000) {
            uint8_t nb = value & 0x1F; if (nb == 0) nb = 1;
            romBank = nb;
        }
        else if (addr < 0x6000) {
            uint8_t nb = value & 0x03;
            ramBank = nb;
        }
        else if (addr < 0x8000) {
            bool nm = value & 0x01;
            bankingMode = nm;
        }
    }
    uint16_t currentRomBank() const { return romBank | (bankingMode ? (ramBank << 5) : 0); }
};

class DMG_MBC2 : public DMG_MBC {
private:
    uint8_t romBank = 1;
    bool ramEnabled = false;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        return rom[(romBank * 0x4000) + (addr - 0x4000)];
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x4000) {
            if (addr & 0x0100)
                romBank = value & 0x0F;
        }
    }
    uint16_t currentRomBank() const { return romBank; }
};

class DMG_MBC3 : public DMG_MBC {
private:
    uint8_t romBank = 1;
    uint8_t ramBank = 0;
    bool ramRTCEnabled = false;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        return rom[(romBank * 0x4000) + (addr - 0x4000)];
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000) {
            bool en = ((value & 0x0F) == 0x0A);
            ramRTCEnabled = en;
        }
        else if (addr < 0x4000) {
            uint8_t nb = value & 0x7F; if (nb == 0) nb = 1;
            romBank = nb;
        }
        else if (addr < 0x6000) {
            ramBank = value;
        }
        else if (addr >= 0xA000 && addr < 0xC000) {
            if (ramRTCEnabled && ramBank <= 0x03)
                ram[ramBank * 0x2000 + (addr - 0xA000)] = value;
        }
    }
    uint16_t currentRomBank() const { return romBank; }
};

class DMG_MBC5 : public DMG_MBC {
private:
    uint16_t romBank = 1;
    uint8_t ramBank = 0;
    bool ramEnabled = false;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        return rom[(romBank * 0x4000) + (addr - 0x4000)];
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000) {
            bool en = ((value & 0x0F) == 0x0A);
            ramEnabled = en;
        }
        else if (addr < 0x3000) {
            uint16_t nb = (romBank & 0x100) | value;
            romBank = nb;
        }
        else if (addr < 0x4000) {
            uint16_t nb = (romBank & 0xFF) | ((value & 0x01) << 8);
            romBank = nb;
        }
        else if (addr < 0x6000) {
            uint8_t nb = value & 0x0F;
            ramBank = nb;
        }
    }
    uint16_t currentRomBank() const { return romBank; }
};

#endif