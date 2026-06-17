#ifndef VRITA_DEBUGGERS_DEFINES_CGB_INL_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_CGB_INL_INCLUDES

#include <array>
#include <cstdint>
#include <map>
#include <stdint.h>
#include <vector>

#include "debuggers_defines.hpp"

inline std::array<MemoryRegion, 14> MemoryMap_CGB_Default = { {
    {"Boot ROM", "Only when Boot ROM is enabled", {0x0000, 0x00FF}, 0xFAAAAA, false},
    {"ROM Bank 0", "Fixed cartridge ROM bank", {0x0000, 0x3FFF}, 0xAAAAAA, false},
    {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
    {"VRAM Bank 0", "Video RAM bank 0", {0x8000, 0x9FFF}, 0x00FF00, true},
    {"VRAM Bank 1", "Video RAM bank 1 (CGB only)", {0x8000, 0x9FFF}, 0x00CC00, true},
    {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
    {"WRAM Bank 0", "Work RAM bank 0", {0xC000, 0xCFFF}, 0x0000FF, true},
    {"WRAM Bank 1", "Work RAM bank 1 (CGB only)", {0xD000, 0xDFFF}, 0x0000CC, true},
    {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x0000AA, true},
    {"OAM", "Sprite attributes", {0xFE00, 0xFE9F}, 0xFF00FF, true},
    {"Unusable", "Prohibited - reads 0xFF, writes ignored", {0xFEA0, 0xFEFF}, 0xAAAAAA, false},
    {"I/O Registers", "I/O registers", {0xFF00, 0xFF7F}, 0xFFFF00, true},
    {"CGB Palette Registers", "BG/OBJ palettes (CGB only)", {0xFF68, 0xFF6B}, 0xAA00FF, true},
    {"HRAM + IE", "High RAM + IE register", {0xFF80, 0xFFFF}, 0xFF8800, true}
} };

inline MemoryTree MemoryMap_CGBByUnitTree = {
    {"MMU", {
        {"ROM", {
            {"Boot ROM", "DMG/CGB bootstrap ROM", {0x0000, 0x00FF}, 0xFFD700, false},
            {"ROM Bank 0", "Fixed cartridge ROM", {0x0000, 0x3FFF}, 0xAAAAAA, false},
            {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
        }},
        {"RAM", {
            {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
            {"WRAM Bank 0", "Work RAM bank 0", {0xC000, 0xCFFF}, 0x0000FF, true},
            {"WRAM Bank N", "Work RAM banks 1-7 (CGB)", {0xD000, 0xDFFF}, 0x0000CC, true},
            {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x000088, true},
            {"HRAM", "High RAM", {0xFF80, 0xFFFE}, 0xFF8800, true},
        }},
        {"Reserved", {
            {"Unusable", "Prohibited area", {0xFEA0, 0xFEFF}, 0x666666, false}
        }}
    }},
    {"PPU", {
        {"VRAM", {
            {"Tile Data", "Tile patterns", {0x8000, 0x97FF}, 0x00DD00, true},
            {"BG Map 0", "Background map 0", {0x9800, 0x9BFF}, 0x00AA00, true},
            {"BG Map 1", "Background map 1", {0x9C00, 0x9FFF}, 0x008800, true},
        }},
        {"VRAM Banks (CGB)", {
            {"VRAM Bank 0", "Base VRAM bank", {0x8000, 0x9FFF}, 0x00FF00, true},
            {"VRAM Bank 1", "Extra VRAM bank", {0x8000, 0x9FFF}, 0x00CC00, true},
        }},
        {"OAM", {
            {"OAM", "Sprite attribute table", {0xFE00, 0xFE9F}, 0xFF00FF, true}
        }},
        {"Registers", {
            {"LCD Registers", "PPU control/status", {0xFF40, 0xFF4B}, 0xCC00CC, true},
            {"Palette Registers", "CGB BG/OBJ palettes", {0xFF68, 0xFF6B}, 0xAA00FF, true},
        }}
    }},
    {"APU", {
        {"Registers", {
            {"APU", "Sound registers", {0xFF10, 0xFF3F}, 0xFF6666, true}
        }}
    }},
    {"Timer", {
        {"Registers", {
            {"Timer", "DIV/TIMA/TMA/TAC", {0xFF04, 0xFF07}, 0xFFFF00, true}
        }}
    }},
    {"Serial", {
        {"Registers", {
            {"Serial", "SB/SC", {0xFF01, 0xFF02}, 0x00FFFF, true}
        }}
    }},
    {"DMA", {
        {"Registers", {
            {"DMA", "OAM DMA transfer", {0xFF46, 0xFF46}, 0xFFAA00, true}
        }},
        {"CGB", {
            {"HDMA", "HDMA1-5 transfer system", {0xFF51, 0xFF55}, 0xFF7700, true}
        }}
    }},
    {"CPU", {
        {"Input", {
            {"Joypad", "P1/JOYP", {0xFF00, 0xFF00}, 0xAAFF00, true}
        }},
        {"Interrupts", {
            {"IF", "Interrupt Flag", {0xFF0F, 0xFF0F}, 0xFF0000, true},
            {"IE", "Interrupt Enable", {0xFFFF, 0xFFFF}, 0xCC0000, true}
        }},
        {"Control", {
            {"Boot ROM Disable", "FF50 register", {0xFF50, 0xFF50}, 0xAAAA00, true}
        }},
        {"CGB", {
            {"Speed Switch (KEY1)", "CPU speed control", {0xFF4D, 0xFF4D}, 0xFF5500, true},
            {"Infrared (RP)", "CGB IR port", {0xFF56, 0xFF56}, 0xFF8800, true}
        }}
    }}
};

#endif