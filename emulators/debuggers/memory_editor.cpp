#include "memory_editor.hpp"

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
    viewPerspective = settings.GetInt("Debuggers - Memory Editor", "view_perspective", 0);
    selectedMemoryRegion = nullptr;
    return true;
}

void MemoryEditor::release() {
    selectedMemoryRegion = nullptr;
    settings.Set("Debuggers - Memory Editor", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Memory Editor", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Memory Editor", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Memory Editor", "height", (int)lastWindowSize.y);
    settings.Save();
}

void MemoryEditor::setMemory(const char* emulatorType, uint8_t* data, uint32_t size) {
    if (data != memoryData || size != memorySize) {
        selectedMemoryRegion = nullptr;
        if (data && size > 0) {
            shadowMemory.assign(data, data + size);
            changeTimer.assign(size, 0.0f);
        }
        else {
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
                memoryRegions = nullptr;
                memoryRegionCount = 0;
                break;
            default:
                memoryRegions = nullptr;
                memoryRegionCount = 0;
                selectedMemoryRegion = nullptr;
                break;
        }
        this->emulatorType = 1;
    }
    else if (emulatorType == "agb") {
        memoryRegions = MemoryMap_AGB_Default.data();
        memoryRegionCount = MemoryMap_AGB_Default.size();
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

    if (emulatorType == 1 && followRegister > 0 && registerReadFunction) {
        static const char* registerNames[] = { "", "BC", "DE", "HL", "SP", "PC" };
        followAddress = (int)registerReadFunction(registerNames[followRegister]);
    }
    else
        followAddress = -1;

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
    static const char* viewPerspectives[] = { "Default", "By unit" };
    if (ImGui::Combo("##viewPerspectiveCombo", &viewPerspective, viewPerspectives, IM_ARRAYSIZE(viewPerspectives))) {
        settings.Set("Debuggers - Memory Editor", "view_perspective", (int)viewPerspective);
        settings.Save();
    }
    ImGui::Separator();
    ImGui::Separator();
    if (emulatorType == 1) {
        static const char* registerFollowOptions[] = { "None", "BC", "DE", "HL", "SP", "PC" };
        ImGui::Text("Follow register:");
        ImGui::SameLine();
        if (ImGui::Combo("##followRegister", &followRegister, registerFollowOptions, IM_ARRAYSIZE(registerFollowOptions))) {
            settings.Set("Debuggers - Memory Editor", "follow_register", followRegister);
            settings.Save();
        }
    }
    ImGui::Separator();
    switch (viewPerspective) {
        case 0:
            memoryRegions = MemoryMap_DMG_Default.data();
            memoryRegionCount = MemoryMap_DMG_Default.size();
            renderViewPerspectiveDefault();
            break;
        case 1:
            memoryRegions = nullptr;
            memoryRegionCount = 0;
            if (emulatorType == 1)
                renderViewPerspectiveAdvanced(MemoryMap_DMG_ByUnitTree);
            break;
    }

    ImGui::End();
}

void MemoryEditor::setRegsiterCallback(std::function<uint16_t(const char*)> getRegsiter) {
    registerReadFunction = getRegsiter;
}

void MemoryEditor::renderViewPerspectiveDefault() {
    if (ImGui::BeginTabBar("MemoryEditor", ImGuiTabBarFlags_None)) {
        for (size_t r = 0; r < memoryRegionCount; r++) {
            int tab_addr = (followAddress >= 0) ? followAddress : scrollToAddress;
            bool is_target = tab_addr >= 0
                && (uint32_t)tab_addr >= memoryRegions[r].range.start
                && (uint32_t)tab_addr <= memoryRegions[r].range.end;
            ImGuiTabItemFlags flags = is_target ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            if (ImGui::BeginTabItem(memoryRegions[r].region, nullptr, flags)) {
                ImGui::SetItemTooltip(memoryRegions[r].notes);
                renderMemoryRegion(memoryRegions[r]);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void MemoryEditor::renderViewPerspectiveAdvanced(const MemoryTree& tree) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float total_width = ImGui::GetContentRegionAvail().x;

    if (panelsWidthLeft < 100.0f)
        panelsWidthLeft = 100.0f;
    if (panelsWidthLeft > total_width - 100.0f)
        panelsWidthLeft = total_width - 100.0f;

    float right_width = total_width - panelsWidthLeft - panelsSplitterWidth;

    // left
    ImGui::BeginChild("leftPanel", ImVec2(panelsWidthLeft, 0), true);
    ImGui::Text("Sections");
    ImGui::Separator();
    renderViewPerspectiveTree(tree);
    ImGui::EndChild();

    ImGui::SameLine(0.0f, 0.0f);

    // splitter
    ImGui::InvisibleButton("panelsSplitter", ImVec2(panelsSplitterWidth, -1));
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        panelsWidthLeft += ImGui::GetIO().MouseDelta.x;

    // splitter view
    ImVec2 p0 = ImGui::GetItemRectMin();
    ImVec2 p1 = ImGui::GetItemRectMax();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(p0, p1, IM_COL32(80, 80, 80, 255));

    ImGui::SameLine(0.0f, 0.0f);

    // right
    ImGui::BeginChild("rightPanel", ImVec2(right_width, 0), true);
    if (selectedMemoryRegion != nullptr)
        renderMemoryRegion(*selectedMemoryRegion);
    else
        ImGui::Text("Select a section from left.");
    ImGui::EndChild();
}

void MemoryEditor::renderViewPerspectiveTree(const MemoryTree& tree) {
    int tab_addr = (followAddress >= 0) ? followAddress : scrollToAddress;
    for (const auto& [unitName, categories] : tree) {
        bool unit_has_target = false;
        if (tab_addr >= 0) {
            for (const auto& [catName, regions] : categories)
                for (const auto& region : regions)
                    if ((uint32_t)tab_addr >= region.range.start && (uint32_t)tab_addr <= region.range.end) {
                        unit_has_target = true;
                        break;
                    }
        }
        if (unit_has_target)
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        bool openUnit = ImGui::TreeNode(unitName);
        if (openUnit) {
            for (const auto& [categoryName, regions] : categories) {
                bool cat_has_target = false;
                if (tab_addr >= 0) {
                    for (const auto& region : regions) {
                        if ((uint32_t)tab_addr >= region.range.start && (uint32_t)tab_addr <= region.range.end) {
                            cat_has_target = true;
                            selectedMemoryRegion = &region;
                            break;
                        }
                    }
                }
                if (regions.size() == 1) {
                    if (cat_has_target)
                        selectedMemoryRegion = &regions.front();
                    renderViewPerspectiveTreeRegion(regions.front());
                }
                else {
                    if (cat_has_target)
                        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    bool openCat = ImGui::TreeNode(categoryName);
                    if (openCat) {
                        for (const auto& region : regions)
                            renderViewPerspectiveTreeRegion(region);
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::TreePop();
        }
    }
}

void MemoryEditor::renderViewPerspectiveTreeRegion(const MemoryRegion& region) {
    ImGui::PushID(region.region);
    bool isSelected = (selectedMemoryRegion == &region);
    ImVec4 color(((region.color >> 16) & 0xFF) / 255.0f, ((region.color >> 8) & 0xFF) / 255.0f, (region.color & 0xFF) / 255.0f, 1.0f);
    ImGui::ColorButton("##color", color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(10, 10));
    ImGui::SameLine();
    char label[128];
    snprintf(label, sizeof(label), "%s [0x%04X - 0x%04X]", region.region, region.range.start, region.range.end);
    if (region.editable)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
    if (ImGui::Selectable(label, isSelected))
        selectedMemoryRegion = &region;
    if (region.editable)
        ImGui::PopStyleColor();
    ImGui::SetItemTooltip(region.notes);
    ImGui::PopID();
}

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
    if (followAddress >= 0)
        gotoAddr = (uint32_t)followAddress;
    ImGui::SetNextItemWidth(120);
    ImGui::InputScalar("##memoryeditorgoto", ImGuiDataType_U32, &gotoAddr, nullptr, nullptr, "%04X", ImGuiInputTextFlags_AutoSelectAll);
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

        int scrollTargetRow = -1;
        if (followAddress >= 0 && (uint32_t)followAddress >= regionStart && (uint32_t)followAddress <= regionEnd)
            scrollTargetRow = (int)(((uint32_t)followAddress - regionStart) / 16);
        if (scrollToAddress >= 0 && (uint32_t)scrollToAddress >= regionStart && (uint32_t)scrollToAddress <= regionEnd) {
            scrollTargetRow = (scrollToAddress - (int)regionStart) / 16;
            scrollToAddress = -1;
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)((regionSize + 15) / 16));
        if (scrollTargetRow >= 0)
            clipper.IncludeItemsByIndex(scrollTargetRow, scrollTargetRow + 1);
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
                if (row == scrollTargetRow)
                    ImGui::SetScrollHereY(0.0f);

                // Bytes
                char ascii[17] = {};
                for (int col = 0; col < 16; col++) {
                    ImGui::TableSetColumnIndex(col + 1);
                    if (addr + col < memorySize) {
                        bool is_followed = (followAddress >= 0 && (int)(addr + col) == followAddress);
                        bool is_selected = ((int)(addr + col) == activeAddress);
                        float ct = (addr + col < changeTimer.size()) ? changeTimer[addr + col] : 0.0f;
                        
                        if (is_followed)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(200, 40, 40, 255));
                        else if (is_selected)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(80, 80, 180, 120));
                        else if (ct > 0.0f)
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(180, 60, 60, (int)(ct * 160)));
                        
                        uint8_t b = memoryData[addr + col];
                        if (is_followed)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.78f, 0.16f, 0.16f, 1.0f));
                        if (region.editable) {
                            if (is_followed)
                                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.78f, 0.16f, 0.16f, 1.0f));
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
                            if (is_followed)
                                ImGui::PopStyleColor();
                        }
                        else
                            ImGui::TextColored(is_followed ? ImVec4(1, 1, 1, 1) : color, "%02X", b);
                        if (is_followed)
                            ImGui::PopStyleColor();
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
                        uint32_t current_addr = addr + col;
                        if (current_addr >= memorySize)
                            break;
                        uint8_t value = memoryData[current_addr];
                        char c[2];
                        c[0] = (value >= 32 && value <= 126) ? static_cast<char>(value) : '.';
                        c[1] = '\0';
                        bool is_followed = (followAddress >= 0 && (int)current_addr == followAddress);
                        bool is_selected = ((int)current_addr == activeAddress);
                        float ct = (current_addr < changeTimer.size()) ? changeTimer[current_addr] : 0.0f;
                        if (is_followed)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.78f, 0.16f, 0.16f, 1.0f));
                        else if (is_selected)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.31f, 0.31f, 0.71f, 0.8f));
                        else if (ct > 0.0f)
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.7f, 0.24f, 0.24f, ct * 0.8f));
                        bool has_highlight = is_followed || is_selected || ct > 0.0f;
                        ImGui::PushID(current_addr);
                        ImGui::SetNextItemWidth(12);
                        if (ImGui::InputText("##char", c, sizeof(c), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AutoSelectAll)) {
                            memoryWrite(current_addr, static_cast<uint8_t>(c[0]));
                            memoryData[current_addr] = static_cast<uint8_t>(c[0]);
                        }
                        if (ImGui::IsItemFocused())
                            activeAddress = (int)current_addr;
                        ImGui::PopID();
                        if (has_highlight)
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

