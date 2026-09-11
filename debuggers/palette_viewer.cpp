#include "palette_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <imgui.h>
#include "third_party/imgui/imgui_impl_sdl2.h"

#include "utilities/settings.hpp"
#include "debuggers_defines_dmg.hpp"
#include "debuggers_defines_cgb.hpp"
#include "debuggers_defines_agb.hpp"
#include "emulators/dmg/palette_presets.hpp"

bool PaletteViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Palette Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Palette Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Palette Viewer", "width", 488);
    windowHeight = settings.GetInt("Debuggers - Palette Viewer", "height", 357);
    paletteChoicesSelected = settings.GetInt("Debuggers - Palette Viewer", "dmg_chosen_palette", 0);
    return true;
}

void PaletteViewer::release() {
    settings.Set("Debuggers - Palette Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Palette Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Palette Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Palette Viewer", "height", (int)lastWindowSize.y);
    settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteChoicesSelected);
    settings.Save();
}

void PaletteViewer::setCallbacks(std::function<const uint8_t* (bool)> getPaletteRAM) {
    funcGetPaletteRAM = getPaletteRAM;
}

void PaletteViewer::setMemory(const char* emuType, uint8_t bgp, uint8_t obp0, uint8_t obp1, bool isCGB) {
    isCGBLoaded = isCGB;
    if (strcmp(emuType, "dmg") == 0)
        emulatorType = 1;
    else if (strcmp(emuType, "agb") == 0)
        emulatorType = 2;
    if (autoRefresh) {
        paletteBGP = bgp;
        paletteOBP0 = obp0;
        paletteOBP1 = obp1;
    }
}

void PaletteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Debuggers - Palette Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (isCGBLoaded ? !funcGetPaletteRAM : paletteBGP == 0) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    if (!isCGBLoaded) {
        ImGui::Text("Choose palette transformer:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
        if (ImGui::Combo("##paletteChoicesCombo", &paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
            settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteChoicesSelected);
            settings.Save();
        }

        ImGui::Text("Auto refresh:");
        ImGui::SameLine();
        ImGui::Checkbox("##autoRefresh", &autoRefresh);

        ImGui::Separator();
    }

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (isCGBLoaded) {
        if (ImGui::BeginTable("##colorPalettesTableCGB", 2, tableFlags)) {
            ImGui::TableSetupColumn("Palette", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
            ImGui::TableSetupColumn("Colors");
            ImGui::TableHeadersRow();

            char label[16];
            for (uint8_t i = 0; i < 8; i++) {
                std::snprintf(label, sizeof(label), "BG %i", i);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                renderCenteredCellContent(label);
                ImGui::TableSetColumnIndex(1);
                renderCGBPaletteButtons(label, i, false);
            }
            for (uint8_t i = 0; i < 8; i++) {
                std::snprintf(label, sizeof(label), "OBJ %i", i);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                renderCenteredCellContent(label);
                ImGui::TableSetColumnIndex(1);
                renderCGBPaletteButtons(label, i, true);
            }

            ImGui::EndTable();
        }
    }
    else if (ImGui::BeginTable("##colorPalettesTable", 2, tableFlags)) {
        ImGui::TableSetupColumn("Palette", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 100.0f);
        ImGui::TableSetupColumn("Colors");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        renderCenteredCellContent("BGP");
        ImGui::TableSetColumnIndex(1);
        renderColorButtons("##paletteBGP", paletteBGP);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        renderCenteredCellContent("OBP 0");
        ImGui::TableSetColumnIndex(1);
        renderColorButtons("##paletteOBP0", paletteOBP0);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        renderCenteredCellContent("OBP 1");
        ImGui::TableSetColumnIndex(1);
        renderColorButtons("##paletteOBP1", paletteOBP1);

        ImGui::EndTable();
    }

    ImGui::End();
}

void PaletteViewer::renderCenteredCellContent(const char* lbl) {
    ImVec2 textSize = ImGui::CalcTextSize(lbl);
    float cellWidth = ImGui::GetContentRegionAvail().x;
    float x = (cellWidth - textSize.x) * 0.5f;
    float y = (80.0f - textSize.y) * 0.5f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
    ImGui::Text("%s", lbl);
}

PaletteColor PaletteViewer::getColorPalette(uint8_t colorValue) {
    switch (paletteChoicesSelected) {
        case 0:
            return unpackPaletteColor(DMG_PALETTE_DEFAULT[colorValue]);
        case 2:
            return unpackPaletteColor(DMG_PALETTE_CGB[colorValue]);
        case 3:
            return unpackPaletteColor(DMG_PALETTE_MGB[colorValue]);
        case 4:
            return unpackPaletteColor(DMG_PALETTE_MGL[colorValue]);
        default:
            return unpackPaletteColor(DMG_PALETTE_DMG[colorValue]);
    }
}

PaletteColor PaletteViewer::unpackPaletteColor(uint32_t packed) {
    return {
        ((packed >> 16) & 0xFF) / 255.0f,
        ((packed >> 8) & 0xFF) / 255.0f,
        (packed & 0xFF) / 255.0f
    };
}

void PaletteViewer::renderColorButtons(const char* label, uint8_t paletteValue) {
    ImGui::PushID(label);
    uint8_t colorValue0 = paletteValue & 0x03;
    uint8_t colorValue1 = (paletteValue >> 2) & 0x03;
    uint8_t colorValue2 = (paletteValue >> 4) & 0x03;
    uint8_t colorValue3 = (paletteValue >> 6) & 0x03;

    PaletteColor bgp_color0 = getColorPalette(colorValue0);
    PaletteColor bgp_color1 = getColorPalette(colorValue1);
    PaletteColor bgp_color2 = getColorPalette(colorValue2);
    PaletteColor bgp_color3 = getColorPalette(colorValue3);

    renderButtonWithBorder("##0", ImVec2(80, 80), bgp_color0);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##1", ImVec2(80, 80), bgp_color1);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##2", ImVec2(80, 80), bgp_color2);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##3", ImVec2(80, 80), bgp_color3);
    ImGui::PopID();
}

void PaletteViewer::renderCGBPaletteButtons(const char* label, uint8_t paletteNum, bool isOBJ) {
    ImGui::PushID(label);

    PaletteColor color0 = resolveCGBColor(paletteNum, isOBJ, 0);
    PaletteColor color1 = resolveCGBColor(paletteNum, isOBJ, 1);
    PaletteColor color2 = resolveCGBColor(paletteNum, isOBJ, 2);
    PaletteColor color3 = resolveCGBColor(paletteNum, isOBJ, 3);

    renderButtonWithBorder("##0", ImVec2(80, 80), color0);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##1", ImVec2(80, 80), color1);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##2", ImVec2(80, 80), color2);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##3", ImVec2(80, 80), color3);
    ImGui::PopID();
}

bool PaletteViewer::renderButtonWithBorder(const char* label, const ImVec2& size, PaletteColor background_color, PaletteColor border_color, float border_thickness) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 end_pos = ImVec2(pos.x + size.x, pos.y + size.y);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(background_color.r, background_color.g, background_color.b, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(background_color.r, background_color.g, background_color.b, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(background_color.r, background_color.g, background_color.b, 1.0));

    bool clicked = ImGui::Button(label, size);
    int r = static_cast<int>(std::round(background_color.r * 255));
    int g = static_cast<int>(std::round(background_color.g * 255));
    int b = static_cast<int>(std::round(background_color.b * 255));
    std::string hex = rgbToHex(r, g, b);
    ImGui::SetItemTooltip("RGB (%i, %i, %i)\nHEX %s", r, g, b, hex.c_str());
    draw_list->AddRect(pos, end_pos, ImColor(ImVec4(border_color.r, border_color.g, border_color.b, 1.0f)), 0.0f, 0, border_thickness);

    ImGui::PopStyleColor(3);

    return clicked;
}

PaletteColor PaletteViewer::resolveCGBColor(uint8_t paletteNum, bool isOBJ, uint8_t colorId) const {
    if (!funcGetPaletteRAM)
        return { 1.0f, 0.0f, 1.0f }; // some color
    const uint8_t* pal = funcGetPaletteRAM(isOBJ);
    uint16_t idx = paletteNum * 8 + colorId * 2;
    uint16_t rgb555 = pal[idx] | (pal[idx + 1] << 8);
    uint8_t r5 = rgb555 & 0x1F;
    uint8_t g5 = (rgb555 >> 5) & 0x1F;
    uint8_t b5 = (rgb555 >> 10) & 0x1F;
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g5 << 3) | (g5 >> 2);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    return { r8 / 255.0f, g8 / 255.0f, b8 / 255.0f };
}