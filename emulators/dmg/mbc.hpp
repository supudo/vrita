/*

GameBoy

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

class DMG_HuC1 : public DMG_MBC {
private:
    uint8_t romBank = 1;
    uint8_t ramBank = 0;
    bool ramEnabled = false;
    bool irMode = false;
    uint8_t irLED = 0;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        if (addr < 0x8000)
            return rom[(romBank * 0x4000) + (addr - 0x4000)];
        if (irMode)
            return 0xC0; // no IR signal
        if (ramEnabled && !ram.empty())
            return ram[ramBank * 0x2000 + (addr - 0xA000)];
        return 0xFF;
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000) {
            irMode = (value & 0x0f) == 0x0E;
            ramEnabled = (value & 0x0F) == 0x0A;
        }
        else if (addr < 0x4000) {
            uint8_t nb = value & 0x3F; if (nb == 0) nb = 1;
            romBank = nb;
        }
        else if (addr < 0x6000)
            ramBank = value & 0x03;
        else if (addr < 0x8000) {
            // unused - HuC1 doesn't implement MBC1's large-ROM mode
        }
        else if (irMode)
            irLED = value & 0x01; // LED on/off
        else if (ramEnabled && !ram.empty())
            ram[ramBank * 0x2000 + (addr - 0xA000)] = value;
    }
    uint16_t currentRomBank() const { return romBank; }
};

class DMG_HuC3 : public DMG_MBC {
private:
    uint8_t romBank = 1;
    uint8_t ramBank = 0;
    uint8_t mode = 0x00; // 0x00 - 0x03 = RAM banks, 0x0A - 0x0D = RTC/semaphore registers
    uint8_t rtcRegister = 0;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        if (addr < 0x8000)
            return rom[(romBank * 0x4000) + (addr - 0x4000)];
        if (mode <= 0x03 && !ram.empty())
            return ram[mode * 0x2000 + (addr - 0xA000)];
        if (mode >= 0x0A && mode <= 0x0D)
            return rtcRegister; // simplified - real protocol is a nibble-serial command stream
        return 0xFF;
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000) {
            // RAM/RTC enable - HuC3 doesn't gate access as strictly as MBC3, accept unconditionally
        }
        else if (addr < 0x4000) {
            uint8_t nb = value & 0x7F; if (nb == 0) nb = 1;
            romBank = nb;
        }
        else if (addr < 0x6000) {
            mode = value;
        }
        else if (addr < 0x8000) {
            // unused
        }
        else if (mode <= 0x03 && !ram.empty()) {
            ram[mode * 0x2000 + (addr - 0xA000)] = value;
        }
        else if (mode >= 0x0A && mode <= 0x0D) {
            rtcRegister = value & 0x0F; // simplified command accept
        }
    }
    uint16_t currentRomBank() const { return romBank; }
};

// Kirby Tilt 'n' Tumble 1/2, Command Master, Korokoro Kirby
class DMG_MBC7 : public DMG_MBC {
private:
    uint16_t romBank = 1;
    bool ramEnabledA = false, ramEnabledB = false;

    // accelerometer
    static constexpr uint16_t TILT_NEUTRAL = 0x8000;
    uint8_t latchState = 0;

    // 93LC56 serial EEPROM: 256 bytes, 16-bit words, addressed 6 bits
    std::array<uint16_t, 128> eeprom {};
    bool eepromLoaded = false;
    uint8_t eepromCS = 0;
    uint8_t eepromCLK = 0;
    uint8_t eepromDI = 0;
    uint8_t eepromDO = 1;
    enum class EepromState { 
        Idle, 
        Command, 
        Address, 
        WriteData, 
        ReadData 
    } eepromState = EepromState::Idle;
    uint16_t shiftReg = 0;
    int bitCount = 0;
    uint8_t eepromOpcode = 0;
    uint8_t eepromAddr = 0;

    void loadEepromFromRAM() {
        if (eepromLoaded || ram.size() < 256) 
            return;
        for (int i = 0; i < 128; i++)
            eeprom[i] = ram[i * 2] | (ram[i * 2 + 1] << 8);
        eepromLoaded = true;
    }
    void saveEepromToRAM() {
        if (ram.size() < 256) 
            return;
        for (int i = 0; i < 128; i++) {
            ram[i * 2] = eeprom[i] & 0xFF;
            ram[i * 2 + 1] = eeprom[i] >> 8;
        }
    }
    void eepromClockEdge(uint8_t newDI) {
        loadEepromFromRAM();
        shiftReg = ((shiftReg << 1) | (newDI & 1)) & 0xFFFF;
        bitCount++;
        switch (eepromState) {
            case EepromState::Idle:
                if (bitCount >= 2 && (shiftReg & 0x03) == 0x01) { // start bit + first opcode bit
                    eepromState = EepromState::Command;
                    bitCount = 0; shiftReg = 0;
                }
                break;
            case EepromState::Command:
                if (bitCount == 8) { // 2-bit opcode + 6-bit address
                    eepromOpcode = (shiftReg >> 6) & 0x03;
                    eepromAddr = shiftReg & 0x3F;
                    bitCount = 0; shiftReg = 0;
                    if (eepromOpcode == 0x02) { // READ
                        shiftReg = eeprom[eepromAddr];
                        eepromState = EepromState::ReadData;
                        bitCount = 16;
                    }
                    else
                        eepromState = EepromState::WriteData; // WRITE (0x01) or ERASE (0x03)
                }
                break;
            case EepromState::WriteData:
                if (bitCount == 16) {
                    eeprom[eepromAddr] = shiftReg;
                    saveEepromToRAM();
                    eepromState = EepromState::Idle;
                    bitCount = 0; shiftReg = 0;
                }
                break;
            default: break;
        }
    }

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        if (addr < 0x8000)
            return rom[(romBank * 0x4000) + (addr - 0x4000)];
        if (addr >= 0xA000 && addr < 0xC000) {
            uint16_t reg = addr & 0xFF;
            switch (reg) {
                case 0x80: // EEPROM DO bit
                    if (eepromState == EepromState::ReadData && bitCount > 0)
                        return 0xFE | ((shiftReg >> (bitCount - 1)) & 1);
                    return 0xFE | eepromDO;
                case 0x00:
                case 0x10:
                    return 0x00; // latch/reset regs read back 0
                case 0x20:
                    return TILT_NEUTRAL & 0xFF; // X low
                case 0x30:
                    return (TILT_NEUTRAL >> 8) & 0xFF; // X high
                case 0x40:
                    return TILT_NEUTRAL & 0xFF; // Y low
                case 0x50:
                    return (TILT_NEUTRAL >> 8) & 0xFF; // Y high
                default:
                    return 0xFF;
            }
        }
        return 0xFF;
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000)
            ramEnabledA = (value & 0x0F) == 0x0A;
        else if (addr < 0x4000) {
            uint16_t nb = value; if (nb == 0) nb = 1;
            romBank = nb & 0x7F;
        }
        else if (addr < 0x6000) 
            ramEnabledB = (value == 0x40);
        else if (addr < 0x8000) {
            // unused
        }
        else if (addr >= 0xA000 && addr < 0xC000 && ramEnabledA && ramEnabledB) {
            uint16_t reg = addr & 0xFF;
            if (reg == 0x80) {
                uint8_t cs = (value >> 7) & 1;
                uint8_t clk = (value >> 6) & 1;
                uint8_t di = (value >> 1) & 1;
                if (cs == 0) { 
                    eepromState = EepromState::Idle; 
                    bitCount = 0; 
                    shiftReg = 0;
                }
                else if (clk == 1 && eepromCLK == 0) // rising edge
                    eepromClockEdge(di);
                eepromCS = cs;
                eepromCLK = clk;
                eepromDI = di;
            }
            // 0x00/0x10: accelerometer latch sequence (0x55 then 0xAA) - accepted, no-op since stubbed level
        }
    }
    uint16_t currentRomBank() const { return romBank; }
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

// "Net de Get: Minigame @ 100" (Japan-only)
class DMG_MBC6 : public DMG_MBC {
private:
    uint16_t romBankA = 0, romBankB = 1; // two independently-switchable 8KB ROM half-banks, x4000-0x5FFF and 0x6000-0x7FFF selected separately
    bool ramEnabled = false;

public:
    using DMG_MBC::DMG_MBC;
    uint8_t read(uint16_t addr) override {
        if (addr < 0x4000)
            return rom[addr];
        if (addr < 0x6000)
            return rom[(romBankA * 0x2000) + (addr - 0x4000)];
        if (addr < 0x8000)
            return rom[(romBankB * 0x2000) + (addr - 0x6000)];
        if (ramEnabled && !ram.empty())
            return ram[addr - 0xA000]; // flash - plain SRAM
        return 0xFF;
    }
    void write(uint16_t addr, uint8_t value) override {
        if (addr < 0x2000)
            ramEnabled = (value & 0x0F) == 0x0A;
        else if (addr < 0x2800)
            romBankA = value; // bank select for 0x4000-0x5FFF
        else if (addr < 0x3800)
            romBankB = value; // bank select for 0x6000-0x7FFF
        else if (ramEnabled && !ram.empty() && addr >= 0xA000 && addr < 0xC000)
            ram[addr - 0xA000] = value; // no flash program/erase protocol - direct write
    }
    uint16_t currentRomBank() const { return romBankB; }
};

#endif