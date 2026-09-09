#include "mmu.hpp"

#include "utilities/logger.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "timer.hpp"
#include "interrupt.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "joypad.hpp"

void DMG_MMU::setUnits(Logger& log, DMG_CARTRIDGE& cartridge, DMG_CPU& cpu, DMG_TIMER& timer, DMG_INTERRUPT& interrupts, DMG_PPU& ppu, DMG_APU& apu, DMG_JOYPAD& joypad) {
    logger = &log;
    managerCartridge = &cartridge;
    managerCPU = &cpu;
    managerTimer = &timer;
    managerInterrupts = &interrupts;
    managerPPU = &ppu;
    managerAPU = &apu;
    managerJoypad = &joypad;
}

void DMG_MMU::resetRegisters() {
    memory[0xFF01] = 0x00; // SB
    memory[0xFF02] = 0x7E; // SC

    memory[0xFF04] = 0xAB; // DIV - divider register
    memory[0xFF05] = 0x00; // TIMA - timer counter
    memory[0xFF06] = 0x00; // TMA - timer modulo
    memory[0xFF07] = 0xF8; // TAC - timer control

    memory[0xFF0F] = 0xE1; // IF - interrupt flag

    memory[0xFF10] = 0x80;
    memory[0xFF11] = 0xBF;
    memory[0xFF12] = 0xF3;
    memory[0xFF13] = 0xFF;
    memory[0xFF14] = 0xBF;

    memory[0xFF16] = 0x3F;
    memory[0xFF17] = 0x00;
    memory[0xFF18] = 0xFF;
    memory[0xFF19] = 0xBF;

    memory[0xFF1A] = 0x7F;
    memory[0xFF1B] = 0xFF;
    memory[0xFF1C] = 0x9F;
    memory[0xFF1D] = 0xFF;
    memory[0xFF1E] = 0xBF;

    memory[0xFF20] = 0xFF;
    memory[0xFF21] = 0x00;
    memory[0xFF22] = 0x00;
    memory[0xFF23] = 0xBF;

    memory[0xFF24] = 0x77;
    memory[0xFF25] = 0xF3;
    memory[0xFF26] = 0xF1;

    memory[0xFF40] = 0x91; // LCDC
    memory[0xFF41] = 0x85; // STAT
    memory[0xFF42] = 0x00; // SCY
    memory[0xFF43] = 0x00; // SCX
    memory[0xFF44] = 0x00; // LY
    memory[0xFF45] = 0x00; // LYC

    memory[0xFF47] = 0xFC; // BGP
    memory[0xFF48] = 0xFF; // OBP0
    memory[0xFF49] = 0xFF; // OBP1

    memory[0xFF4A] = 0x00; // WY
    memory[0xFF4B] = 0x00; // WX

    memory[0xFFFF] = 0x00; // IE

    oamWriteSourcePC.fill(0);

    if (cgbMode) {
        memory[0xFF4D] = 0x7E; // KEY1
        memory[0xFF4F] = 0xFE; // VBK
        memory[0xFF55] = 0xFF; // HDMA5
        memory[0xFF70] = 0xF9; // SVBK
    }
    registerKEY1 = 0x7E;
    registerBCPS = 0;
    registerOCPS = 0;
}

void DMG_MMU::clearMemory() {
    memorySize = 0x10000;
    memory.assign(memorySize, 0);
    resetRegisters();
    oamWriteSourcePC.fill(0);

    currentVRAMBank = 0;
    currentWRAMBank = 1;
    vramBank1.assign(0x2000, 0);
    for (auto& bank : wramBanks)
        bank.assign(0x1000, 0);
}

void DMG_MMU::clearResources() {
    totalCycles = 0;
    firstRAMWrite = true;
    isHalted = false;
    triggerHaltBug = false;
    dmaActive = false;
    dmaProgress = 0;
    dmaTCycles = 0;
    doubleSpeed = false;
    hdmaActive = false;
    hdmaProgress = 0;
    clearMemory();
}

uint16_t DMG_MMU::getOAMWriteSource(uint16_t oamAddress) const {
    if (oamAddress < addressOAMStart || oamAddress > 0xFE9F) return 0;
    return oamWriteSourcePC[oamAddress];
}

uint8_t DMG_MMU::read8(uint16_t address, bool no_tick) {
    if (!no_tick)
        tick(4);
    if (address < 0x8000 || (address > 0xA000 && address < 0xC000))
        return managerCartridge->read(address);
    if (address == 0xFF00)
        return managerJoypad->read();
    if (address >= 0xFF04 && address <= 0xFF07)
        return managerTimer->read(address);
    if (address >= 0xFF10 && address <= 0xFF3F)
        return managerAPU->readRegister(address);
    if (dmaActive && address >= addressOAMStart && address <= 0xFE9F)
        return 0xFF;
    if (cgbMode) {
        if (address >= 0x8000 && address < 0xA000 && currentVRAMBank == 1)
            return vramBank1[address - 0x8000];
        if (address >= 0xD000 && address < 0xE000 && currentWRAMBank >= 2)
            return wramBanks[currentWRAMBank - 2][address - 0xD000];
        if (address == addressKEY1)
            return (uint8_t)((doubleSpeed << 7) | (registerKEY1 & 0x01) | 0x7E);
        if (address == addressVBK)
            return currentVRAMBank | 0xFE;
        if (address == addressSVBK)
            return currentWRAMBank | 0xF8;
        if (address == addressBCPS)
            return registerBCPS | 0x40;
        if (address == addressBCPD)
            return bgPaletteRAM[registerBCPS & 0x3F];
        if (address == addressOCPS)
            return registerOCPS | 0x40;
        if (address == addressOCPD)
            return objPaletteRAM[registerOCPS & 0x3F];
        if (address == addressRP)
            return 0xFF; // no IR hardware
    }
    return memory[address];
}

uint8_t DMG_MMU::rawRead(uint16_t address) {
    if (address < 0x8000 || (address > 0xA000 && address < 0xC000))
        return managerCartridge->read(address);
    if (cgbMode && address >= 0xD000 && address < 0xE000 && currentWRAMBank >= 2)
        return wramBanks[currentWRAMBank - 2][address - 0xD000];
    return memory[address];
}

void DMG_MMU::write8(uint16_t address, uint8_t value, bool no_tick) {
    if (!no_tick)
        tick(4);
    if (address < 0x8000) { // MBC write, no memory store
        managerCartridge->write(address, value);
        return;
    }
    if (address > 0xA000 && address < 0xC000) { // external cartridge RAM
        if (firstRAMWrite) {
            logger->log("[MMU] First external RAM write @ 0x%04X", address);
            firstRAMWrite = false;
        }
        managerCartridge->write(address, value);
        return;
    }
    if (address == 0xFF00) {
        managerJoypad->write(value);
        return;
    }
    if (address >= 0xFF04 && address <= 0xFF07) {
        managerTimer->write(address, value);
        return;
    }
    if (address >= 0xFF10 && address <= 0xFF3F) {
        managerAPU->writeRegister(address, value);
        return;
    }
    if (address == 0xFF46) {
        memory[address] = value;
        dmaSource = (uint16_t)value << 8;
        dmaActive = true;
        dmaProgress = 0;
        dmaTCycles = -8;
        return;
    }
    if (cgbMode) {
        if (address >= 0x8000 && address < 0xA000 && currentVRAMBank == 1) {
            vramBank1[address - 0x8000] = value;
            return;
        }
        if (address >= 0xD000 && address < 0xE000 && currentWRAMBank >= 2) {
            wramBanks[currentWRAMBank - 2][address - 0xD000] = value;
            return;
        }
        if (address == addressKEY1) {
            registerKEY1 = (registerKEY1 & 0x80) | (value & 0x01);
            return;
        }
        if (address == addressVBK) {
            currentVRAMBank = value & 0x01;
            return;
        }
        if (address == addressSVBK) {
            uint8_t bank = value & 0x07;
            currentWRAMBank = (bank == 0) ? 1 : bank;
            return;
        }
        if (address == addressBCPS) {
            registerBCPS = value & 0xBF;
            return;
        }
        if (address == addressBCPD) {
            bgPaletteRAM[registerBCPS & 0x3F] = value;
            if (registerBCPS & 0x80)
                registerBCPS = (registerBCPS & 0x80) | ((registerBCPS + 1) & 0x3F);
            return;
        }
        if (address == addressOCPS) {
            registerOCPS = value & 0xBF;
            return;
        }
        if (address == addressOCPD) {
            objPaletteRAM[registerOCPS & 0x3F] = value;
            if (registerOCPS & 0x80)
                registerOCPS = (registerOCPS & 0x80) | ((registerOCPS + 1) & 0x3F);
            return;
        }
        if (address == addressHDMA5) {
            startHDMATransfer(value);
            return;
        }
        if (address == addressRP)
            return; // no IR hardware, ignore
    }
    memory[address] = value;
    oamWriteSourcePC[address] = managerCPU->currentInstructionPC;
}

void DMG_MMU::tick(uint32_t cycles) {
    totalCycles += cycles;
    managerTimer->tick(cycles);
    if (dmaActive) {
        dmaTCycles += cycles;
        while (dmaActive && dmaTCycles >= 4) {
            dmaTCycles -= 4;
            uint16_t src = dmaSource + dmaProgress;
            memory[addressOAMStart + dmaProgress] = rawRead(src);
            oamWriteSourcePC[addressOAMStart + dmaProgress] = oamWriteSourcePC[src] ? oamWriteSourcePC[src] : managerCPU->currentInstructionPC;
            dmaProgress++;
            if (dmaProgress >= 160)
                dmaActive = false;
        }
    }
}

void DMG_MMU::switchSpeedIfArmed() {
    if (!(registerKEY1 & 0x01))
        return;
    doubleSpeed = !doubleSpeed;
    registerKEY1 &= ~0x01;
    managerTimer->write(0xFF04, 0); // reset DIV on speed switch
}

uint8_t DMG_MMU::vramReadBanked(uint16_t addr) const {
    if (currentVRAMBank == 1 && addr >= 0x8000 && addr < 0xA000)
        return vramBank1[addr - 0x8000];
    return memory[addr];
}

void DMG_MMU::startHDMATransfer(uint8_t hdma5Value) {
    uint16_t src = ((memory[addressHDMA1] << 8) | memory[addressHDMA2]) & 0xFFF0;
    uint16_t dst = 0x8000 + (((memory[addressHDMA3] & 0x1F) << 8) | (memory[addressHDMA4] & 0xF0));
    hdmaSource = src;
    hdmaDestination = dst;
    hdmaLength = ((hdma5Value & 0x7F) + 1) * 0x10;
    hdmaProgress = 0;
    hdmaHBlankMode = (hdma5Value & 0x80) != 0;
    hdmaActive = true;
    if (!hdmaHBlankMode)
        runHDMAChunk(hdmaLength);
}

void DMG_MMU::runHDMAChunk(uint16_t count) {
    for (uint16_t i = 0; i < count && hdmaProgress < hdmaLength; ++i) {
        uint8_t byte = rawRead(hdmaSource + hdmaProgress);
        uint16_t dstAddr = hdmaDestination + hdmaProgress;
        if (currentVRAMBank == 1)
            vramBank1[dstAddr - 0x8000] = byte;
        else
            memory[dstAddr] = byte;
        hdmaProgress++;
    }
    if (hdmaProgress >= hdmaLength) {
        hdmaActive = false;
        memory[addressHDMA5] = 0xFF; // transfer complete
    }
}

void DMG_MMU::onHBlank() {
    if (hdmaActive && hdmaHBlankMode)
        runHDMAChunk(0x10); // 16 bytes per HBLank
}
