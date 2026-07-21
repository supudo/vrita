#include "sprite_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"
#include "debuggers_defines_dmg.hpp"
#include "debuggers_defines_cgb.hpp"
#include "debuggers_defines_agb.hpp"

bool SpriteViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Sprite Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Sprite Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Sprite Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Sprite Viewer", "height", 300);
    return true;
}

void SpriteViewer::release() {
    settings.Set("Debuggers - Sprite Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Sprite Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Sprite Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Sprite Viewer", "height", (int)lastWindowSize.y);
    settings.Save();
}

void SpriteViewer::setMemory(const char* emulatorType, uint8_t* data) {
    memoryData = data;
    uint8_t et = -1;
    if (strcmp(emulatorType, "dmg") == 0)
        et = 1;
    else if (strcmp(emulatorType, "agb") == 0)
        et = 2;
    else
        et = 0;
    bool changed = et != this->emulatorType;
    this->emulatorType = et;
    if (changed)
        initializeData(et);
}

void SpriteViewer::initializeData(uint8_t emulatorType) {
}

void SpriteViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!ImGui::Begin("Debuggers - Sprite Viewer", windowOpened)) {
        ImGui::End();
        return;
    }

    if (!memoryData) {
        ImGui::Text("No file loaded. Memory is empty.");
        ImGui::End();
        return;
    }

    if (autoRefresh)
        initializeData(emulatorType);

    ImGui::Checkbox("Show grid", &showGrid);

    ImGui::Checkbox("Auto refresh", &autoRefresh);
    ImGui::SameLine();
    if (autoRefresh)
        ImGui::BeginDisabled();
    if (ImGui::Button("Refresh"))
        initializeData(emulatorType);
    if (autoRefresh)
        ImGui::EndDisabled();

    ImGui::Separator();

    ImGui::Text("Tiles size");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat("##tileSize", &zoomPerPixel, 1.0f, 16.0f);

    ImGui::Separator();

    float availHeight = ImGui::GetContentRegionAvail().y;
    float tileMapHeight = availHeight - lastInfoHeight;
    if (tileMapHeight < 50.0f)
        tileMapHeight = 50.0f;
    renderSprites(tileMapHeight);

    ImGui::Separator();

    renderInfo();

    ImGui::End();
}

void SpriteViewer::renderSprites(float height) {
    ImGui::BeginChild("TileMap", ImVec2(0, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    float tileSizeZoom = 8.0f * zoomPerPixel;
    float gridGap = showGrid ? 1.0f : 0.0f;
    float tileStep = tileSizeZoom + gridGap;
    int tilesPerRow = DMG_SpritesX;

    for (uint32_t t = 0; t < DMG_SpritesX * DMG_SpritesY; t++) {
        int tx = t % tilesPerRow;
        int ty = t / tilesPerRow;
        ImVec2 pos(start.x + tx * tileStep, start.y + ty * tileStep);

        if (showGrid)
            draw_list->AddRect(pos, ImVec2(pos.x + tileSizeZoom, pos.y + tileSizeZoom), IM_COL32(60, 60, 60, 255));
    }

    ImVec2 viewportEnd(start.x + DMG_SpritesX * tileStep, start.y + DMG_SpritesY * tileStep);
    draw_list->AddRect(start, viewportEnd, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);

    int totalRows = (DMG_SpritesX * DMG_SpritesY + tilesPerRow - 1) / tilesPerRow;
    ImGui::Dummy(ImVec2(tilesPerRow * tileStep, totalRows * tileStep));

    ImGui::EndChild();
}

void SpriteViewer::renderInfo() {
    const float infoPaddingY = 8.0f;
    float infoStartY = ImGui::GetCursorPosY();

    ImGui::Dummy(ImVec2(0, infoPaddingY));

    ImGuiTableFlags table_flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("Settings", 2, table_flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

        float rowHeight = ImGui::GetFrameHeight();

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Transparency");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Index");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Y / X");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Tile index");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Attributes");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Source Y");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Source X");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Source tile index");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Source attributes");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("...");

        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0, infoPaddingY));

    lastInfoHeight = ImGui::GetCursorPosY() - infoStartY;
}

void SpriteViewer::textRightAligned(const char* text) {
    float textWidth = ImGui::CalcTextSize(text).x;
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > textWidth)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - textWidth);
    ImGui::Text("%s", text);
}