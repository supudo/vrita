/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_MBC_INCLUDES
#define VRITA_DMG_MBC_INCLUDES

#include <cstdint>
#include <vector>

class DMG_MBC {
public:
    DMG_MBC(uint8_t* rom, size_t romSize, std::vector<uint8_t>& ram)
        : rom(rom), romSize(romSize), ram(ram) {}

    virtual ~DMG_MBC() = default;

    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t value) = 0;

protected:
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
        if (addr < 0x2000)
            ramEnabled = ((value & 0x0F) == 0x0A);
        else if (addr < 0x4000) {
            romBank = value & 0x1F;
            if (romBank == 0) romBank = 1;
        }
        else if (addr < 0x6000)
            ramBank = value & 0x03;
        else if (addr < 0x8000)
            bankingMode = value & 0x01;
    }
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
        if (addr < 0x2000)
            ramRTCEnabled = ((value & 0x0F) == 0x0A);
        else if (addr < 0x4000) {
            romBank = value & 0x7F;
            if (romBank == 0) romBank = 1;
        }
        else if (addr < 0x6000)
            ramBank = value;
        else if (addr >= 0xA000 && addr < 0xC000) {
            if (ramRTCEnabled && ramBank <= 0x03)
                ram[ramBank * 0x2000 + (addr - 0xA000)] = value;
        }
    }
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
        if (addr < 0x2000)
            ramEnabled = ((value & 0x0F) == 0x0A);
        else if (addr < 0x3000)
            romBank = (romBank & 0x100) | value;
        else if (addr < 0x4000)
            romBank = (romBank & 0xFF) | ((value & 0x01) << 8);
        else if (addr < 0x6000)
            ramBank = value & 0x0F;
    }
};

#endif