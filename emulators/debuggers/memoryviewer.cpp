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

void MemoryViewer::setMemory(const char* emulatorType, uint8_t* data, uint32_t size) {
    memoryData = data;
    memorySize = size;
    if (emulatorType == "dmg") {
        memoryRegions = MemoryMap_DMG.data();
        memoryRegionCount = MemoryMap_DMG.size();
        this->emulatorType = 1;
    }
    else if (emulatorType == "agb") {
        memoryRegions = MemoryMap_AGB.data();
        memoryRegionCount = MemoryMap_AGB.size();
        this->emulatorType = 2;
    }
    else {
        memoryRegions = nullptr;
        memoryRegionCount = 0;
        this->emulatorType = 0;
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

void MemoryViewer::setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8) {
    memoryRead = read8;
    memoryWrite = write8;
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

    if (ImGui::BeginTabBar("MemoryViewer", ImGuiTabBarFlags_None)) {
        for (size_t r = 0; r < memoryRegionCount; r++) {
            if (ImGui::BeginTabItem(memoryRegions[r].region)) {
                ImGui::SetItemTooltip(memoryRegions[r].notes);
                renderMemoryRegion(memoryRegions[r]);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void MemoryViewer::renderMemoryRegion(MemoryRegion region) {
    uint32_t regionStart = region.range.start;
    uint32_t regionEnd = region.range.end;
    if (regionStart >= memorySize)
        return;
    regionEnd = std::min(regionEnd, memorySize - 1);
    uint32_t regionSize = regionEnd - regionStart + 1;
    uint32_t c = region.color;
    ImVec4 color(((c >> 16) & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, (c & 0xFF) / 255.0f, 1.0f);

    static uint32_t gotoAddr = 0;
    ImGui::SetNextItemWidth(120);
    ImGui::InputScalar("##memoryviewergoto", ImGuiDataType_U32, &gotoAddr, nullptr, nullptr, "%04X");
    ImGui::SameLine();
    if (ImGui::Button("Jump")) {
        if (gotoAddr >= regionStart && gotoAddr <= regionEnd)
            scrollToAddrress = (int)(gotoAddr & ~0xF);
    }

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
            int targetRow = (scrollToAddrress - regionStart) / 16;
            ImGui::SetScrollY((float)targetRow * ImGui::GetTextLineHeightWithSpacing());
            scrollToAddrress = -1;
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)((regionSize + 15) / 16));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                uint32_t addr = regionStart + row * 16;
                if (addr >= memorySize) break;

                ImGui::TableNextRow();

                // Address
                ImGui::TableSetColumnIndex(0);
                if (this->emulatorType == 1)
                    ImGui::Text("%04X", addr);
                else
                    ImGui::Text("%08X", addr);

                // Bytes
                char ascii[17] = {};
                for (int col = 0; col < 16; col++) {
                    ImGui::TableSetColumnIndex(col + 1);
                    if (addr + col < memorySize) {
                        bool isSelected = ((int)(addr + col) == activeAddr);
                        if (isSelected)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(80, 80, 180, 120));
                        uint8_t b = memoryData[addr + col];
                        if (region.editable) {
                            ImGui::PushID(addr + col);
                            uint8_t value = b;
                            ImGui::SetNextItemWidth(28);
                            if (ImGui::InputScalar("##byte", ImGuiDataType_U8, &value, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll)) {
                                memoryWrite(addr + col, value);
                                memoryData[addr + col] = value;
                            }
                            if (ImGui::IsItemFocused())
                                activeAddr = (int)(addr + col);
                            ImGui::PopID();
                        }
                        else
                            ImGui::TextColored(color, "%02X", b);
                        uint8_t final_b = memoryData[addr + col];
                        ascii[col] = (final_b >= 32 && final_b < 127) ? (char)final_b : '.';
                    }
                }

                // Dump
                ImGui::TableSetColumnIndex(17);
                if (!region.editable)
                    ImGui::TextUnformatted(ascii);
                else
                    for (int col = 0; col < 16; col++) {
                        uint32_t currentAddr = addr + col;
                        if (currentAddr >= memorySize)
                            return;
                        uint8_t value = memoryData[currentAddr];
                        char c[2];
                        c[0] = (value >= 32 && value <= 126) ? static_cast<char>(value) : '.';
                        c[1] = '\0';
                        bool isSelected = ((int)currentAddr == activeAddr);
                        if (isSelected)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.31f, 0.31f, 0.71f, 0.8f));
                        ImGui::PushID(currentAddr);
                        ImGui::SetNextItemWidth(12);
                        if (ImGui::InputText("##char", c, sizeof(c), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll)) {
                            memoryWrite(currentAddr, static_cast<uint8_t>(c[0]));
                            memoryData[currentAddr] = static_cast<uint8_t>(c[0]);
                        }
                        if (ImGui::IsItemFocused())
                            activeAddr = (int)currentAddr;
                        ImGui::PopID();
                        if (isSelected)
                            ImGui::PopStyleColor();
                        if (col != 15)
                            ImGui::SameLine(0.0f, 0.0f);
                    }
            }
        }
        ImGui::EndTable();
    }
}