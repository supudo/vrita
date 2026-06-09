#ifndef VRITA_MEMORYEDITOR_DMG_INCLUDES
#define VRITA_MEMORYEDITOR_DMG_INCLUDES

#include <array>
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <map>
#include <stdint.h>
#include <vector>

#include "memory_editor_data.hpp"

inline std::array<MemoryRegion, 11> MemoryMap_DMG_Default = { {
    {"Boot ROM", "Only when Boot ROM is enabled", {0x0000, 0x00FF}, 0xFAAAAA, false},
    {"ROM Bank 0", "Fixed cartridge ROM bank", {0x0000, 0x3FFF}, 0xAAAAAA, false},
    {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
    {"VRAM", "Video RAM", {0x8000, 0x9FFF}, 0x00FF00, true},
    {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
    {"WRAM", "Work RAM", {0xC000, 0xDFFF}, 0x0000FF, true},
    {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x0000AA, true},
    {"OAM", "Sprite attributes", {0xFE00, 0xFE9F}, 0xFF00FF, true},
    {"Unusable", "Prohibited - reads 0xFF, writes ignored", {0xFEA0, 0xFEFF}, 0xAAAAAA, false},
    {"I/O Registers", "I/O registers", {0xFF00, 0xFF7F}, 0xFFFF00, true},
    {"HRAM + IE", "High RAM + IE register", {0xFF80, 0xFFFF}, 0xFF8800, true}
} };

inline MemoryTree MemoryMap_DMG_ByUnitTree = {
    {"MMU", {
        {"ROM", {
            {"Boot ROM", "DMG bootstrap ROM", {0x0000, 0x00FF}, 0xFFD700, false},
            {"ROM Bank 0", "Fixed cartridge ROM", {0x0000, 0x3FFF}, 0xAAAAAA, false},
            {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
        }},
        {"RAM", {
            {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
            {"WRAM Bank 0", "Work RAM", {0xC000, 0xCFFF}, 0x0000FF, true},
            {"WRAM Bank 1", "Work RAM (mirror on DMG)", {0xD000, 0xDFFF}, 0x0000CC, true},
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
        {"OAM", {
            {"OAM", "Sprite attribute table", {0xFE00, 0xFE9F}, 0xFF00FF, true}
        }},
        {"Registers", {
            {"LCD Registers", "PPU control/status registers", {0xFF40, 0xFF4B}, 0xCC00CC, true}
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
        }}
    }}
};

#endif