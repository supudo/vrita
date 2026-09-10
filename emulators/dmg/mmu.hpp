#ifndef VRITA_DMG_MMU_INCLUDES
#define VRITA_DMG_MMU_INCLUDES

#include <stdint.h>
#include <array>
#include <vector>

class Logger;
class DMG_APU;
class DMG_CARTRIDGE;
class DMG_CPU;
class DMG_INTERRUPT;
class DMG_JOYPAD;
class DMG_PPU;
class DMG_TIMER;

class DMG_MMU {
public:
    void setUnits(Logger& logger, DMG_CARTRIDGE& cartridge, DMG_CPU& cpu, DMG_TIMER& timer, DMG_INTERRUPT& interrupts, DMG_PPU& ppu, DMG_APU& apu, DMG_JOYPAD& joypad);
    void clearMemory();
    void clearResources();
    void resetRegisters();
    void tick(uint32_t cycles);

    uint32_t memorySize;
    std::vector<uint8_t> memory;

    bool isHalted = false;
    bool triggerHaltBug = false;
    uint64_t totalCycles = 0;
    bool firstRAMWrite = true;

    uint8_t read8(uint16_t address, bool no_tick = false);
    void write8(uint16_t address, uint8_t value, bool no_tick = false);
    inline uint16_t read16(uint16_t address, bool no_tick = false) { return read8(address, no_tick) | (read8(address + 1, no_tick) << 8); }
    inline void write16(uint16_t address, uint16_t value, bool no_tick = false) { write8(address, value & 0xFF, no_tick); write8(address + 1, value >> 8, no_tick); }
    inline void writeStack(uint16_t* sp, uint16_t value) { (*sp)--; write8(*sp, (uint8_t)((value & 0xFF00) >> 8), false); (*sp)--; write8(*sp, (uint8_t)(value & 0x00FF), false); }
    inline uint16_t readStack(uint16_t* sp) { uint16_t value = read16(*sp); *sp += 2; return value; }

    inline void setCGBMode(bool state) { cgbMode = state; }
    inline bool isCGBMode() const { return cgbMode; }
    bool doubleSpeed = false;
    void switchSpeedIfArmed();
    uint8_t vramReadBanked(uint16_t addr) const;
    inline uint8_t vramReadBank(uint16_t addr, uint8_t bank) const { return bank ? vramBank1[addr - 0x8000] : memory[addr]; }
    void onHBlank();
    inline const std::array<uint8_t, 64>& getBGPaletteRAM() const { return bgPaletteRAM; }
    inline const std::array<uint8_t, 64>& getOBJPaletteRAM() const { return objPaletteRAM; }

    std::array<uint16_t, 0x10000> oamWriteSourcePC{};
    uint16_t getOAMWriteSource(uint16_t oamAddress) const;

private:
    uint8_t rawRead(uint16_t address);

    Logger* logger = nullptr;
    DMG_CARTRIDGE* managerCartridge = nullptr;
    DMG_CPU* managerCPU = nullptr;
    DMG_TIMER* managerTimer = nullptr;
    DMG_INTERRUPT* managerInterrupts = nullptr;
    DMG_PPU* managerPPU = nullptr;
    DMG_APU* managerAPU = nullptr;
    DMG_JOYPAD* managerJoypad = nullptr;

    uint16_t addressOAMStart = 0xFE00;

    bool dmaActive = false;
    uint16_t dmaSource = 0;
    int dmaProgress = 0;
    int dmaTCycles = 0;

    // CGB related
    bool cgbMode = false;

    // VBK - VRAM bank 1, bank 0 is 0x8000 - 0x9FFF
    std::vector<uint8_t> vramBank1;
    uint8_t currentVRAMBank = 0;

    // SVBK, WRAM banks 2-7, 1 is 0xD000 - 0xDFFF
    std::array<std::vector<uint8_t>, 6> wramBanks;
    uint8_t currentWRAMBank = 1;

    // KEY1
    uint8_t registerKEY1 = 0x7E;

    // BCPS/BCPD/OCPS/OCPD
    std::array<uint8_t, 64> bgPaletteRAM{};
    std::array<uint8_t, 64> objPaletteRAM{};
    uint8_t registerBCPS = 0;
    uint8_t registerOCPS = 0;

    // HDMA1-5
    bool hdmaActive = false;
    bool hdmaHBlankMode = false;
    uint16_t hdmaSource = 0;
    uint16_t hdmaDestination = 0;
    uint16_t hdmaLength = 0;
    uint16_t hdmaProgress = 0;

    void startHDMATransfer(uint8_t hdma5Value);
    void runHDMAChunk(uint16_t count);

    static constexpr uint16_t addressKEY1 = 0xFF4D;
    static constexpr uint16_t addressVBK = 0xFF4F;
    static constexpr uint16_t addressHDMA1 = 0xFF51;
    static constexpr uint16_t addressHDMA2 = 0xFF52;
    static constexpr uint16_t addressHDMA3 = 0xFF53;
    static constexpr uint16_t addressHDMA4 = 0xFF54;
    static constexpr uint16_t addressHDMA5 = 0xFF55;
    static constexpr uint16_t addressRP = 0xFF56;
    static constexpr uint16_t addressBCPS = 0xFF68;
    static constexpr uint16_t addressBCPD = 0xFF69;
    static constexpr uint16_t addressOCPS = 0xFF6A;
    static constexpr uint16_t addressOCPD = 0xFF6B;
    static constexpr uint16_t addressSVBK = 0xFF70;
    static constexpr uint16_t addressPCM12 = 0xFF76;
    static constexpr uint16_t addressPCM34 = 0xFF77;
};

#endif