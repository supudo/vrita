#ifndef VRITA_DEBUGGERS_DEFINES_DMG_INL_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_DMG_INL_INCLUDES

#include <array>
#include <cstdint>
#include <map>
#include <stdint.h>
#include <string_view>
#include <vector>

#include "debuggers_defines.hpp"

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

inline std::array<PaletteColor, 4> DMG_Palette_DMG = { {
    { 0.88f, 0.97f, 0.82f },
    { 0.55f, 0.75f, 0.42f },
    { 0.22f, 0.42f, 0.18f },
    { 0.06f, 0.15f, 0.06f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_CGB = { {
    { 0.61f, 0.74f, 0.06f },
    { 0.55f, 0.67f, 0.06f },
    { 0.19f, 0.38f, 0.19f },
    { 0.06f, 0.22f, 0.06f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_MGB = { {
    { 0.77f, 0.81f, 0.63f },
    { 0.55f, 0.58f, 0.43f },
    { 0.30f, 0.33f, 0.24f },
    { 0.12f, 0.12f, 0.12f }
} };

inline std::array<PaletteColor, 4> DMG_Palette_MGL = { {
    { 0.11f, 0.87f, 0.81f },
    { 0.10f, 0.78f, 0.70f },
    { 0.09f, 0.65f, 0.59f },
    { 0.04f, 0.48f, 0.43f }
} };

constexpr std::array<std::string_view, 256> DMG_MBC_Types = [] {
    std::array<std::string_view, 256> a{};
    a[0x00] = "ROM ONLY";
    a[0x01] = "MBC1";
    a[0x02] = "MBC1 + RAM";
    a[0x03] = "MBC1 + RAM + BATTERY";
    a[0x05] = "MBC2";
    a[0x06] = "MBC2 + BATTERY";
    a[0x08] = "ROM + RAM";
    a[0x09] = "ROM + RAM + BATTERY";
    a[0x0B] = "MMM01";
    a[0x0C] = "MMM01 + RAM";
    a[0x0D] = "MMM01 + RAM + BATTERY";
    a[0x0F] = "MBC3 + TIMER + BATTERY";
    a[0x10] = "MBC3 + TIMER + RAM + BATTERY";
    a[0x11] = "MBC3";
    a[0x12] = "MBC3 + RAM";
    a[0x13] = "MBC3 + RAM + BATTERY";
    a[0x19] = "MBC5";
    a[0x1A] = "MBC5 + RAM";
    a[0x1B] = "MBC5 + RAM + BATTERY";
    a[0x1C] = "MBC5 + RUMBLE";
    a[0x1D] = "MBC5 + RUMBLE + RAM";
    a[0x1E] = "MBC5 + RUMBLE + RAM + BATTERY";
    a[0x20] = "MBC6";
    a[0x22] = "MBC7 + SENSOR + RUMBLE + RAM + BATTERY";
    a[0xFC] = "POCKET CAMERA";
    a[0xFD] = "BANDAI TAMA5";
    a[0xFE] = "HuC3";
    a[0xFF] = "HuC1 + RAM + BATTERY";
    return a;
}();

using namespace std::string_view_literals;

constexpr std::array<std::pair<std::string_view, std::string_view>, 64> DMG_NewLicenseeCodes { {
    { "00"sv, "None"sv },
    { "01"sv, "Nintendo Research & Development 1"sv },
    { "08"sv, "Capcom"sv },
    { "13"sv, "EA(Electronic Arts)"sv },
    { "18"sv, "Hudson Soft"sv },
    { "19"sv, "B-AI"sv },
    { "20"sv, "KSS"sv },
    { "22"sv, "Planning Office WADA"sv },
    { "24"sv, "PCM Complete"sv },
    { "25"sv, "San-X"sv },
    { "28"sv, "Kemco"sv },
    { "29"sv, "SETA Corporation"sv },
    { "30"sv, "Viacom"sv },
    { "31"sv, "Nintendo"sv },
    { "32"sv, "Bandai"sv },
    { "33"sv, "Ocean Software / Acclaim Entertainment"sv },
    { "34"sv, "Konami"sv },
    { "35"sv, "HectorSoft"sv },
    { "37"sv, "Taito"sv },
    { "38"sv, "Hudson Soft"sv },
    { "39"sv, "Banpresto"sv },
    { "41"sv, "Ubi Soft1"sv },
    { "42"sv, "Atlus"sv },
    { "44"sv, "Malibu Interactive"sv },
    { "46"sv, "Angel"sv },
    { "47"sv, "Bullet-Proof Software"sv },
    { "49"sv, "Irem"sv },
    { "50"sv, "Absolute"sv },
    { "51"sv, "Acclaim Entertainment"sv },
    { "52"sv, "Activision"sv },
    { "53"sv, "Sammy USA Corporation"sv },
    { "54"sv, "Konami"sv },
    { "55"sv, "Hi Tech Expressions"sv },
    { "56"sv, "LJN"sv },
    { "57"sv, "Matchbox"sv },
    { "58"sv, "Mattel"sv },
    { "59"sv, "Milton Bradley Company"sv },
    { "60"sv, "Titus Interactive"sv },
    { "61"sv, "Virgin Games Ltd."sv },
    { "64"sv, "Lucasfilm Games"sv },
    { "67"sv, "Ocean Software"sv },
    { "69"sv, "EA(Electronic Arts)"sv },
    { "70"sv, "Infogrames"sv },
    { "71"sv, "Interplay Entertainment"sv },
    { "72"sv, "Broderbund"sv },
    { "73"sv, "Sculptured Software"sv },
    { "75"sv, "The Sales Curve Limited"sv },
    { "78"sv, "THQ"sv },
    { "79"sv, "Accolade"sv },
    { "80"sv, "Misawa Entertainment"sv },
    { "83"sv, "LOZC G."sv },
    { "86"sv, "Tokuma Shoten"sv },
    { "87"sv, "Tsukuda Original"sv },
    { "91"sv, "Chunsoft Co."sv },
    { "92"sv, "Video System"sv },
    { "93"sv, "Ocean Software / Acclaim Entertainment"sv },
    { "95"sv, "Varie"sv },
    { "96"sv, "Yonezawa10 / S'Pal"sv },
    { "97"sv, "Kaneko"sv },
    { "99"sv, "Pack-In-Video"sv },
    { "9H"sv, "Bottom Up"sv },
    { "A4"sv, "Konami (Yu-Gi- Oh!)"sv },
    { "BL"sv, "MTO"sv },
    { "DK"sv, "Kodansha"sv },
} };

constexpr std::string_view DMG_GetNewLicneseeCode(std::string_view code) {
    for (const auto& [key, value] : DMG_NewLicenseeCodes) {
        if (key == code)
            return value;
    }
    return "Unknown";
}

#endif