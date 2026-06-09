#include "memoryeditor.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool MemoryEditor::init() {
    windowPositionX = settings.GetInt("Debuggers - Memory Editor", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Memory Editor", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Memory Editor", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Memory Editor", "height", 300);
    viewPerspective = settings.GetInt("Debuggers - Memory Editor", "viewperspective", 0);

    MemoryMap_DMG_Default = { {
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

    MemoryMap_DMG_Debug = { {
        {"Boot ROM", "DMG bootstrap ROM", {0x0000, 0x00FF}, 0xFFD700, false},
        {"ROM Bank 0", "Fixed cartridge ROM bank", {0x0000, 0x3FFF}, 0xAAAAAA, false},
        {"ROM Bank N", "Switchable ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
        {"VRAM", "Video RAM", {0x8000, 0x9FFF}, 0x00FF00, true},
        {"Tile Data 0", "Tile data 0 (0x8000-0x87FF)", {0x8000, 0x87FF}, 0x00CC00, true},
        {"Tile Data 1", "Tile data 1 (0x8800-0x97FF)", {0x8800, 0x97FF}, 0x00CC55, true},
        {"BG Map 0", "Background tile map 0", {0x9800, 0x9BFF}, 0x00AA00, true},
        {"BG Map 1", "Background tile map 1", {0x9C00, 0x9FFF}, 0x00AA55, true},
        {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
        {"WRAM Bank 0", "Work RAM bank 0", {0xC000, 0xCFFF}, 0x0000FF, true},
        {"WRAM Bank 1", "Work RAM bank 1", {0xD000, 0xDFFF}, 0x0000CC, true},
        {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x0000AA, true},
        {"OAM", "Sprite attributes", {0xFE00, 0xFE9F}, 0xFF00FF, true},
        {"Unusable", "Prohibited - reads 0xFF, writes ignored", {0xFEA0, 0xFEFF}, 0xAAAAAA, false},
        {"I/O Registers", "Hardware registers", {0xFF00, 0xFF7F}, 0xFFFF00, true},
        {"Joypad", "Input registers", {0xFF00, 0xFF00}, 0xFFFFAA, true},
        {"Serial", "Serial transfer", {0xFF01, 0xFF02}, 0xFFFFBB, true},
        {"Timer", "DIV/TIMA/TMA/TAC", {0xFF04, 0xFF07}, 0xFFFFCC, true},
        {"Interrupt Flag", "IF register", {0xFF0F, 0xFF0F}, 0xFFFFDD, true},
        {"Audio", "APU registers", {0xFF10, 0xFF3F}, 0xFF99FF, true},
        {"LCD/PPU", "PPU control registers", {0xFF40, 0xFF4B}, 0xFF55FF, true},
        {"DMA", "DMA transfer register", {0xFF46, 0xFF46}, 0xFF33FF, true},
        {"Boot ROM disable", "FF50 register", {0xFF50, 0xFF50}, 0xFF11FF, true},
        {"HRAM", "High RAM", {0xFF80, 0xFFFE}, 0xFF8800, true},
        {"IE Register", "Interrupt Enable", {0xFFFF, 0xFFFF}, 0xFF0000, true}
    } };

    MemoryMap_DMG_ByUnit = { {
        // MMU / Cartridge
        {"Boot ROM", "DMG bootstrap ROM", {0x0000, 0x00FF}, 0xFFD700, false},
        {"ROM Bank 0", "Fixed cartridge ROM bank", {0x0000, 0x3FFF}, 0xAAAAAA, false},
        {"ROM Bank N", "Switchable cartridge ROM bank", {0x4000, 0x7FFF}, 0x888888, false},
        {"External RAM", "Cartridge RAM", {0xA000, 0xBFFF}, 0x00AAAA, true},
        {"WRAM Bank 0", "Work RAM", {0xC000, 0xCFFF}, 0x0000FF, true},
        {"WRAM Bank 1", "Work RAM", {0xD000, 0xDFFF}, 0x0000CC, true},
        {"Echo RAM", "Mirror of WRAM", {0xE000, 0xFDFF}, 0x000088, true},
        {"HRAM", "High RAM", {0xFF80, 0xFFFE}, 0xFF8800, true},

        // PPU
        {"VRAM", "Video RAM", {0x8000, 0x9FFF}, 0x00FF00, true},
        {"Tile Data", "Tile patterns", {0x8000, 0x97FF}, 0x00DD00, true},
        {"BG Map 0", "Background tile map 0", {0x9800, 0x9BFF}, 0x00AA00, true},
        {"BG Map 1", "Background tile map 1", {0x9C00, 0x9FFF}, 0x008800, true},
        {"OAM", "Sprite attribute table", {0xFE00, 0xFE9F}, 0xFF00FF, true},
        {"LCD Registers", "PPU control/status registers", {0xFF40, 0xFF4B}, 0xCC00CC, true},

        // APU
        {"APU Registers", "Audio registers", {0xFF10, 0xFF3F}, 0xFF6666, true},

        // Timer
        {"Timer Registers", "DIV/TIMA/TMA/TAC", {0xFF04, 0xFF07}, 0xFFFF00, true},

        // Serial
        {"Serial Registers", "SB/SC", {0xFF01, 0xFF02}, 0x00FFFF, true},

        // Input
        {"Joypad Register", "P1/JOYP", {0xFF00, 0xFF00}, 0xAAFF00, true},

        // DMA
        {"DMA Register", "OAM DMA", {0xFF46, 0xFF46}, 0xFFAA00, true},

        // Interrupts
        {"IF Register", "Interrupt Flag", {0xFF0F, 0xFF0F}, 0xFF0000, true},
        {"IE Register", "Interrupt Enable", {0xFFFF, 0xFFFF}, 0xCC0000, true},

        // Boot Control
        {"Boot ROM Disable", "Disables bootstrap ROM", {0xFF50, 0xFF50}, 0xAAAA00, true},

        // Reserved
        {"Unusable", "Prohibited area", {0xFEA0, 0xFEFF}, 0x666666, false}
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

void MemoryEditor::release() {
    settings.Set("Debuggers - Memory Editor", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Memory Editor", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Memory Editor", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Memory Editor", "height", (int)lastWindowSize.y);
    settings.Save();
}

void MemoryEditor::setMemory(const char* emulatorType, uint8_t* data, uint32_t size) {
    if (data != memoryData || size != memorySize) {
        if (data && size > 0) {
            shadowMemory.assign(data, data + size);
            changeTimer.assign(size, 0.0f);
        } else {
            shadowMemory.clear();
            changeTimer.clear();
        }
    }
    memoryData = data;
    memorySize = size;
    if (emulatorType == "dmg") {
        switch (viewPerspective) {
            case 0:
                memoryRegions = MemoryMap_DMG_Default.data();
                memoryRegionCount = MemoryMap_DMG_Default.size();
                break;
            case 1:
                memoryRegions = MemoryMap_DMG_Debug.data();
                memoryRegionCount = MemoryMap_DMG_Debug.size();
                break;
            case 2:
                memoryRegions = MemoryMap_DMG_ByUnit.data();
                memoryRegionCount = MemoryMap_DMG_ByUnit.size();
                break;
        }
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

const MemoryRegion* MemoryEditor::getRegion(uint32_t addr) const {
    for (size_t i = 0; i < memoryRegionCount; i++) {
        const auto& region = memoryRegions[i];
        if (addr >= region.range.start &&
            addr <= region.range.end) {
            return &region;
        }
    }
    return nullptr;
}

void MemoryEditor::setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8) {
    memoryRead = read8;
    memoryWrite = write8;
}

void MemoryEditor::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debuggers - Memory Editor", windowOpened)) {
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

    float deltaTime = ImGui::GetIO().DeltaTime;
    for (uint32_t i = 0; i < memorySize; i++) {
        if (memoryData[i] != shadowMemory[i]) {
            changeTimer[i] = 1.0f;
            shadowMemory[i] = memoryData[i];
        }
        if (changeTimer[i] > 0.0f)
            changeTimer[i] = std::max(0.0f, changeTimer[i] - deltaTime);
    }

    ImGui::Text("View perspective:");
    ImGui::SameLine();
    const char* viewPerspectives[] = { "Default", "Debug", "By unit" };
    if (ImGui::Combo("##viewPerspectiveCombo", &viewPerspective, viewPerspectives, IM_ARRAYSIZE(viewPerspectives))) {
        settings.Set("Debuggers - Memory Editor", "viewperspective", (int)viewPerspective);
        settings.Save();
    }
    ImGui::Separator();
    switch (viewPerspective) {
        case 0:
            memoryRegions = MemoryMap_DMG_Default.data();
            memoryRegionCount = MemoryMap_DMG_Default.size();
            renderViewPerspectiveDefault();
            break;
        case 1:
            memoryRegions = MemoryMap_DMG_Debug.data();
            memoryRegionCount = MemoryMap_DMG_Debug.size();
            renderViewPerspectiveDebug();
            break;
        case 2:
            memoryRegions = MemoryMap_DMG_ByUnit.data();
            memoryRegionCount = MemoryMap_DMG_ByUnit.size();
            renderViewPerspectiveByUnit();
            break;
    }

    ImGui::End();
}

void MemoryEditor::renderViewPerspectiveDefault() {
    if (ImGui::BeginTabBar("MemoryEditor", ImGuiTabBarFlags_None)) {
        for (size_t r = 0; r < memoryRegionCount; r++) {
            if (ImGui::BeginTabItem(memoryRegions[r].region)) {
                ImGui::SetItemTooltip(memoryRegions[r].notes);
                renderMemoryRegion(memoryRegions[r]);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void MemoryEditor::renderViewPerspectiveDebug() {
    if (ImGui::BeginTabBar("MemoryEditor", ImGuiTabBarFlags_None)) {
        for (size_t r = 0; r < memoryRegionCount; r++) {
            if (ImGui::BeginTabItem(memoryRegions[r].region)) {
                ImGui::SetItemTooltip(memoryRegions[r].notes);
                renderMemoryRegion(memoryRegions[r]);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void MemoryEditor::renderViewPerspectiveByUnit() {}

void MemoryEditor::renderMemoryRegion(MemoryRegion region) {
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
    ImGui::InputScalar("##memoryeditorgoto", ImGuiDataType_U32, &gotoAddr, nullptr, nullptr, "%04X");
    ImGui::SameLine();
    if (ImGui::Button("Jump")) {
        if (gotoAddr >= regionStart && gotoAddr <= regionEnd)
            scrollToAddress = (int)(gotoAddr & ~0xF);
    }

    float previewHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetTextLineHeightWithSpacing() * 2.0f + ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
    float tableHeight = ImGui::GetContentRegionAvail().y - previewHeight;
    tableHeight = std::max(tableHeight, ImGui::GetFrameHeightWithSpacing() * 3.0f);

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##memoryeditor", 18, tableFlags, ImVec2(0, tableHeight))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        for (int i = 0; i < 16; i++) {
            char label[3];
            snprintf(label, sizeof(label), "%X", i);
            ImGui::TableSetupColumn(label);
        }
        ImGui::TableSetupColumn("Dump");
        ImGui::TableHeadersRow();

        if (scrollToAddress >= 0) {
            int targetRow = (scrollToAddress - regionStart) / 16;
            ImGui::SetScrollY((float)targetRow * ImGui::GetTextLineHeightWithSpacing());
            scrollToAddress = -1;
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
                        bool isSelected = ((int)(addr + col) == activeAddress);
                        float ct = (addr + col < changeTimer.size()) ? changeTimer[addr + col] : 0.0f;
                        if (isSelected)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(80, 80, 180, 120));
                        else if (ct > 0.0f)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(180, 60, 60, (int)(ct * 160)));
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
                                activeAddress = (int)(addr + col);
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
                            break;
                        uint8_t value = memoryData[currentAddr];
                        char c[2];
                        c[0] = (value >= 32 && value <= 126) ? static_cast<char>(value) : '.';
                        c[1] = '\0';
                        bool isSelected = ((int)currentAddr == activeAddress);
                        float ct = (currentAddr < changeTimer.size()) ? changeTimer[currentAddr] : 0.0f;
                        bool hasHighlight = isSelected || ct > 0.0f;
                        if (isSelected)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.31f, 0.31f, 0.71f, 0.8f));
                        else if (ct > 0.0f)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.7f, 0.24f, 0.24f, ct * 0.8f));
                        ImGui::PushID(currentAddr);
                        ImGui::SetNextItemWidth(12);
                        if (ImGui::InputText("##char", c, sizeof(c), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll)) {
                            memoryWrite(currentAddr, static_cast<uint8_t>(c[0]));
                            memoryData[currentAddr] = static_cast<uint8_t>(c[0]);
                        }
                        if (ImGui::IsItemFocused())
                            activeAddress = (int)currentAddr;
                        ImGui::PopID();
                        if (hasHighlight)
                            ImGui::PopStyleColor();
                        if (col != 15)
                            ImGui::SameLine(0.0f, 0.0f);
                    }
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    ImGui::Text("Preview as:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##combo_previewType", selectedDataType, ImGuiComboFlags_HeightLargest)) {
        for (int n = 0; n < IM_ARRAYSIZE(previewDataTypes); n++) {
            bool isSelected = (selectedDataType == previewDataTypes[n]);
            if (ImGui::Selectable(previewDataTypes[n], isSelected))
                selectedDataType = previewDataTypes[n];
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    char bufDec[128] = "", bufHex[128] = "", bufBin[128] = "";
    getPreviewData(activeAddress, bufDec, 'd');
    getPreviewData(activeAddress, bufHex, 'x');
    getPreviewData(activeAddress, bufBin, 'b');
    ImGui::Text("Dec"); ImGui::SameLine(); ImGui::TextUnformatted(bufDec);
    ImGui::Text("Hex"); ImGui::SameLine(); ImGui::TextUnformatted(bufHex);
    ImGui::Text("Bin"); ImGui::SameLine(); ImGui::TextUnformatted(bufBin);
}

void MemoryEditor::getPreviewData(int address, char* out_buf, char format) {
    out_buf[0] = '\0';
    if (address < 0 || !memoryData || (uint32_t)address >= memorySize)
        return;

    auto safe = [&](uint32_t sz) { return (uint32_t)address + sz <= memorySize; };

    auto writeBin = [&](uint64_t v, int bits) {
        int pos = 0;
        for (int i = bits - 1; i >= 0; i--) {
            if (i < bits - 1 && i % 8 == 7) out_buf[pos++] = ' ';
            out_buf[pos++] = (v >> i) & 1 ? '1' : '0';
        }
        out_buf[pos] = '\0';
    };

    uint8_t v8 = 0;
    uint16_t v16 = 0;
    uint32_t v32 = 0;

    if (strcmp(selectedDataType, "Uint8") == 0 && safe(1)) {
        v8 = memoryData[address];
        if (format == 'd') snprintf(out_buf, 128, "%u", (unsigned)v8);
        else if (format == 'x') snprintf(out_buf, 128, "0x%02X", (unsigned)v8);
        else writeBin(v8, 8);
    } 
    else if (strcmp(selectedDataType, "Uint16") == 0 && safe(2)) {
        memcpy(&v16, &memoryData[address], 2);
        if (format == 'd') snprintf(out_buf, 128, "%u", (unsigned)v16);
        else if (format == 'x') snprintf(out_buf, 128, "0x%04X", (unsigned)v16);
        else writeBin(v16, 16);
    } 
    else if (strcmp(selectedDataType, "Uint32") == 0 && safe(4)) {
        memcpy(&v32, &memoryData[address], 4);
        if (format == 'd') snprintf(out_buf, 128, "%u", v32);
        else if (format == 'x') snprintf(out_buf, 128, "0x%08X", v32);
        else writeBin(v32, 32);
    } 
    else
        snprintf(out_buf, 128, "N/A");
}