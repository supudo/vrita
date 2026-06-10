#include "palette_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"

bool PaletteViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Palette Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Palette Viewer", "position_y", 44);
    //windowWidth = settings.GetInt("Debuggers - Palette Viewer", "width", 488);
    //windowHeight = settings.GetInt("Debuggers - Palette Viewer", "height", 357);
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

void PaletteViewer::setMemory(const char* emulatorType, uint8_t bgp, uint8_t obp0, uint8_t obp1) {
    if (strcmp(emulatorType, "dmg") == 0)
        this->emulatorType = 1;
    else if (strcmp(emulatorType, "agb") == 0)
        this->emulatorType = 2;
    if (autoRefresh) {
        paletteBGP = bgp;
        paletteOBP0 = obp0;
        paletteOBP1 = obp1;
    }
}

void PaletteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
    if (!ImGui::Begin("Debuggers - Palette Viewer", windowOpened, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    ImGui::BeginChild("##palettes");

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

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("##colorPalettesTable", 2, tableFlags)) {
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

    ImGui::EndChild();

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
        case 2:
            return DMG_Palette_CGB[colorValue];
        case 3:
            return DMG_Palette_MGB[colorValue];
        case 4:
            return DMG_Palette_MGL[colorValue];
        default:
            return DMG_Palette_DMG[colorValue];
    }
}

void PaletteViewer::renderColorButtons(const char* label, uint8_t paletteValue) {
    ImGui::PushID(label);
    uint8_t colorValue0 = paletteValue & 0x03;
    uint8_t colorValue1 = (paletteValue >> 2) & 0x03;
    uint8_t colorValue2 = (paletteValue >> 4) & 0x03;
    uint8_t colorValue3 = (paletteValue >> 6) & 0x03;

    PaletteColor bgp_color0, bgp_color1, bgp_color2, bgp_color3;
    if (paletteChoicesSelected > 0) {
        bgp_color0 = getColorPalette(colorValue0);
        bgp_color1 = getColorPalette(colorValue1);
        bgp_color2 = getColorPalette(colorValue2);
        bgp_color3 = getColorPalette(colorValue3);
    }
    else {
        float v0 = colorValue0 / 3.0f, v1 = colorValue1 / 3.0f;
        float v2 = colorValue2 / 3.0f, v3 = colorValue3 / 3.0f;
        bgp_color0 = { v0, v0, v0 };
        bgp_color1 = { v1, v1, v1 };
        bgp_color2 = { v2, v2, v2 };
        bgp_color3 = { v3, v3, v3 };
    }

    renderButtonWithBorder("##0", ImVec2(80, 80), bgp_color0, { 1.0f, 1.0f, 1.0f }, 2.0f);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##1", ImVec2(80, 80), bgp_color1, { 1.0f, 1.0f, 1.0f }, 2.0f);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##2", ImVec2(80, 80), bgp_color2, { 1.0f, 1.0f, 1.0f }, 2.0f);
    ImGui::SameLine(0.0f, 10.0f);
    renderButtonWithBorder("##3", ImVec2(80, 80), bgp_color3, { 1.0f, 1.0f, 1.0f }, 2.0f);
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