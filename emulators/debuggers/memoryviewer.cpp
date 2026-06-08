#include "memoryviewer.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool MemoryViewer::init(Settings settings) {
    windowPositionX = settings.GetInt("Debuggers - Memory Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Memory Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Memory Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Memory Viewer", "height", 300);

    MemoryMap_DMG = { {
        {"ROM Bank 0", "Fixed cartridge ROM bank", {0x0000, 0x3FFF}, 0xAAAAAA, false},
        {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
        {"VRAM", "Video RAM", {0x8000, 0x9FFF}, 0x00FF00, true},
        {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
        {"WRAM", "Work RAM", {0xC000, 0xDFFF}, 0x0000FF, true},
        {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x0000AA, true},
        {"OAM", "Sprite attributes", {0xFE00, 0xFE9F}, 0xFF00FF, true},
        {"I/O Registers", "I/O registers", {0xFF00, 0xFF7F}, 0xFFFF00, true},
        {"HRAM + IE", "High RAM + IE register", {0xFF80, 0xFFFF}, 0xFF8800, true}
    } };

    MemoryMap_AGB = { {
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

    return true;
}

void MemoryViewer::release(Settings& settings) {
    settings.Set("Debuggers - Memory Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Memory Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Memory Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Memory Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void MemoryViewer::setMemory(const char* emulatorType, const uint8_t* data, uint32_t size) {
    memoryData = data;
    memorySize = size;
    if (emulatorType == "dmg") {
        memoryRegions = MemoryMap_DMG.data();
        memoryRegionCount = MemoryMap_DMG.size();
    }
    else if (emulatorType == "agb") {
        memoryRegions = MemoryMap_AGB.data();
        memoryRegionCount = MemoryMap_AGB.size();
    }
    else {
        memoryRegions = nullptr;
        memoryRegionCount = 0;
    }
}

const MemoryRegion* MemoryViewer::getRegion(uint32_t addr) const {
    for (size_t i = 0; i < memoryRegionCount; i++) {
        const auto& region = memoryRegions[i];
        if (addr >= region.range.start &&
            addr <= region.range.end) {
            return &region;
        }
    }
    return nullptr;
}

void MemoryViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Memory Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!memoryData) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    static uint32_t gotoAddr = 0;
    ImGui::SetNextItemWidth(120);
    ImGui::InputScalar("##memoryviewergoto", ImGuiDataType_U32, &gotoAddr, nullptr, nullptr, "%04X");
    ImGui::SameLine();
    if (ImGui::Button("Jump"))
        scrollToAddrress = (int)(gotoAddr & ~0xF);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##memoryviewer", 18, tableFlags, ImVec2(0, 0))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        for (int i = 0; i < 16; i++) {
            char label[3];
            snprintf(label, sizeof(label), "%X", i);
            ImGui::TableSetupColumn(label);
        }
        ImGui::TableSetupColumn("Dump");
        ImGui::TableHeadersRow();

        if (scrollToAddrress >= 0) {
            int targetRow = scrollToAddrress / 16;
            ImGui::SetScrollY((float)targetRow * ImGui::GetTextLineHeightWithSpacing());
            scrollToAddrress = -1;
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)(memorySize / 16));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                uint32_t addr = row * 16;
                if (addr >= memorySize) break;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const MemoryRegion* region = getRegion(addr);
                if (region)
                    ImGui::Text("%s:%04X", region->region, addr);
                else
                    ImGui::Text("%08X", addr);
                if (region && ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(region->notes);
                    ImGui::EndTooltip();
                }

                char ascii[17] = {};
                for (int col = 0; col < 16; col++) {
                    ImGui::TableSetColumnIndex(col + 1);
                    if (addr + col < memorySize) {
                        uint8_t b = memoryData[addr + col];

                        ImVec4 color = ImVec4(1, 1, 1, 1);
                        for (size_t r = 0; r < memoryRegionCount; r++) {
                            if (addr + col >= memoryRegions[r].range.start && addr + col <= memoryRegions[r].range.end) {
                                uint32_t c = memoryRegions[r].color;
                                color = ImVec4(
                                    ((c >> 16) & 0xFF) / 255.0f,
                                    ((c >> 8) & 0xFF) / 255.0f,
                                    (c & 0xFF) / 255.0f,
                                    1.0f
                                );
                                break;
                            }
                        }

                        ImGui::TextColored(color, "%02X", b);
                        ascii[col] = (b >= 32 && b < 127) ? (char)b : '.';
                    }
                }

                ImGui::TableSetColumnIndex(17);
                ImGui::TextUnformatted(ascii);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}