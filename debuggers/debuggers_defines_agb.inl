#ifndef VRITA_DEBUGGERS_DEFINES_AGB_INL_INCLUDES
#define VRITA_DEBUGGERS_DEFINES_AGB_INL_INCLUDES

#include <array>
#include <cstdint>
#include <map>
#include <stdint.h>
#include <vector>

inline std::array<MemoryRegion, 11> MemoryMap_AGB_Default = { {
    {"BIOS", "Internal BIOS ROM", {0x00000000, 0x00003FFF}, 0x888888, false},
    {"EWRAM", "External Work RAM", {0x02000000, 0x0203FFFF}, 0x0000FF, true},
    {"IWRAM", "Internal Work RAM", {0x03000000, 0x03007FFF}, 0x0000AA, true},
    {"I/O Registers", "Memory-mapped I/O", {0x04000000, 0x040003FE}, 0xFFFF00, true},
    {"Palette RAM", "BG + OBJ palettes", {0x05000000, 0x050003FF}, 0xFF00FF, true},
    {"VRAM", "Video RAM", {0x06000000, 0x06017FFF}, 0x00FF00, true},
    {"OAM", "Object Attribute Memory", {0x07000000, 0x070003FF}, 0xFF8800, true},
    {"ROM 0", "Cartridge ROM first 16MB", {0x08000000, 0x08FFFFFF}, 0xAAAAAA, false},
    {"ROM 1", "Cartridge ROM mirror", {0x09000000, 0x09FFFFFF}, 0x999999, false},
    {"ROM 2", "Cartridge ROM extended", {0x0A000000, 0x0BFFFFFF}, 0x888888, false},
    {"SRAM", "Save RAM", {0x0E000000, 0x0E00FFFF}, 0x00AAAA, true}
} };

inline MemoryTree MemoryMap_AGB_ByUnitTree = {
    // =========================================================
    // CPU / ARM7TDMI
    // =========================================================
    {"CPU", {
        {"BIOS", {
            {"BIOS", "ARM7 BIOS ROM", {0x00000000, 0x00003FFF}, 0xFFD700, false}
        }},
        {"WRAM", {
            {"On-Board WRAM", "256 KB on-board WRAM", {0x02000000, 0x0203FFFF}, 0x0000FF, true},
            {"On-Chip WRAM", "32 KB on-chip WRAM", {0x03000000, 0x03007FFF}, 0x0000CC, true}
        }},
        {"I/O Registers", {
            {"I/O Registers", "Hardware registers (0x4000000-0x40003FF)", {0x04000000, 0x040003FF}, 0xFFFF00, true}
        }},
        {"Palette RAM", {
            {"BG Palette", "Background palette", {0x05000000, 0x050003FF}, 0x00FF00, true},
            {"OBJ Palette", "Sprite palette", {0x05000400, 0x050007FF}, 0x00CC00, true}
        }},
        {"VRAM", {
            {"VRAM", "0x06000000-0x06017FFF, Video RAM", {0x06000000, 0x06017FFF}, 0x00AA00, true}
        }},
        {"OAM", {
            {"OAM", "Sprite attribute table", {0x07000000, 0x070003FF}, 0xFF00FF, true}
        }},
        {"ROM", {
            {"Game Pak ROM", "Cartridge ROM", {0x08000000, 0x09FFFFFF}, 0xAAAAAA, false}
        }},
        {"SRAM", {
            {"Game Pak SRAM", "External save RAM", {0x0E000000, 0x0E00FFFF}, 0x00AAAA, true}
        }}
    }},

    // =========================================================
    // DMA / Timers
    // =========================================================
    {"DMA/Timers", {
        {"DMA Channels", {
            {"DMA0-3", "DMA channels 0-3", {0x040000B0, 0x040000CF}, 0xFFAA00, true}
        }},
        {"Timers", {
            {"TIMER0-3", "Timers 0-3", {0x04000100, 0x0400010F}, 0xFFFF00, true}
        }},
        {"Interrupts", {
            {"IE", "Interrupt Enable", {0x04000200, 0x04000200}, 0xFF0000, true},
            {"IF", "Interrupt Flag", {0x04000202, 0x04000202}, 0xFF5500, true}
        }}
    }},

    // =========================================================
    // Sound (APU)
    // =========================================================
    {"APU", {
        {"Sound Registers", {
            {"Sound", "Sound Control / FIFO", {0x04000600, 0x040007FF}, 0xFF6666, true}
        }}
    }},

    // =========================================================
    // Serial / Key / Misc
    // =========================================================
    {"Serial/Misc", {
        {"Serial", {
            {"Serial IO", "Multiplayer / Link port", {0x04000120, 0x0400012F}, 0x00FFFF, true}
        }},
        {"Key Input", {
            {"KEYINPUT", "Button input", {0x04000130, 0x04000131}, 0xAAFF00, true},
            {"KEYCNT", "Key control", {0x04000132, 0x04000133}, 0xAA8800, true}
        }},
        {"Waitstate / IE", {
            {"WAITCNT", "Waitstate control", {0x04000204, 0x04000204}, 0xCCAA00, true}
        }}
    }},

    // =========================================================
    // BIOS / Reserved areas
    // =========================================================
    {"Reserved", {
        {"Unused", {
            {"Unused", "Reserved / unused", {0x0A000000, 0x0DFFFFFF}, 0x666666, false}
        }}
    }}
};

#endif