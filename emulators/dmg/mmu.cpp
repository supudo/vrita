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
}

void DMG_MMU::clearMemory() {
    memorySize = 0x10000;
    memory.assign(memorySize, 0);
    resetRegisters();
}

void DMG_MMU::clearResources() {
    totalCycles = 0;
    firstRAMWrite = true;
    clearMemory();
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
        uint16_t source = (uint16_t)value << 8;
        for (uint16_t i = 0; i < 0xA0; i++)
            memory[0xFE00 + i] = read8(source + i, false);
        return;
    }
    memory[address] = value;
}

void DMG_MMU::tick(uint32_t cycles) {
    totalCycles += cycles;
    managerTimer->tick(cycles);
}