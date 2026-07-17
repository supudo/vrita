#include "tilemap_viewer.hpp"

#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/settings.hpp"
#include "debuggers_defines_dmg.hpp"
#include "debuggers_defines_cgb.hpp"
#include "debuggers_defines_agb.hpp"

bool TilemapViewer::init() {
    windowPositionX = settings.GetInt("Debuggers - Tilemap Viewer", "position_x", 44);
    windowPositionY = settings.GetInt("Debuggers - Tilemap Viewer", "position_y", 44);
    windowWidth = settings.GetInt("Debuggers - Tilemap Viewer", "width", 300);
    windowHeight = settings.GetInt("Debuggers - Tilemap Viewer", "height", 300);
    tileMapAddress = settings.GetInt("Debuggers - Tilemap Viewer", "tile_map_address", 0);
    tileDataAddress = settings.GetInt("Debuggers - Tilemap Viewer", "tile_data_address", 0);
    return true;
}

void TilemapViewer::release() {
    settings.Set("Debuggers - Tilemap Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Tilemap Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Tilemap Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Tilemap Viewer", "height", (int)lastWindowSize.y);
    settings.Set("Debuggers - Tile Viewer", "tile_map_address", tileMapAddress);
    settings.Set("Debuggers - Tile Viewer", "tile_data_address", tileDataAddress);
    settings.Save();
}

void TilemapViewer::setMemory(const char* emulatorType, uint8_t* data) {
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

void TilemapViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1) {
        tiles.clear();
        tiles.reserve(DMG_TilesCount);
        const uint8_t* vramTiles = memoryData + DMG_TileAddressStart;
        for (uint32_t i = 0; i < DMG_TilesCount; i++) {
            const uint8_t* vramAddress = vramTiles + i * 16;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);
            TileItem tile(i, address);
            decodeTile(vramAddress, tile);
            tiles.push_back(tile);
        }

        mapTiles1.clear();
        mapTiles1.reserve(DMG_TilemapCount);
        const uint8_t* vramTilemap1 = memoryData + DMG_TileMap1Start;
        for (uint32_t i = 0; i < DMG_TilemapCount; i++) {
            const uint8_t* vramAddress = vramTilemap1 + i;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);
            uint8_t tileIndex = *vramAddress;
            TilemapItem mapTile(i, tileIndex, address, &tiles[tileIndex]);
            mapTiles1.push_back(mapTile);
        }

        mapTiles2.clear();
        mapTiles2.reserve(DMG_TilemapCount);
        const uint8_t* vramTilemap2 = memoryData + DMG_TileMap2Start;
        for (uint32_t i = 0; i < DMG_TilemapCount; i++) {
            const uint8_t* vramAddress = vramTilemap2 + i;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);
            uint8_t tileIndex = *vramAddress;
            TilemapItem mapTile(i, tileIndex, address, &tiles[tileIndex]);
            mapTiles2.push_back(mapTile);
        }
    }
}

void TilemapViewer::decodeTile(const uint8_t* tileData, TileItem& tile) {
    if (emulatorType == 1) {
        for (uint8_t y = 0; y < 8; y++) {
            uint8_t low = tileData[y * 2];
            uint8_t high = tileData[y * 2 + 1];
            for (uint8_t x = 0; x < 8; x++) {
                uint8_t bit = 7 - x;
                uint8_t lo = (low >> bit) & 1;
                uint8_t hi = (high >> bit) & 1;
                tile.Pixels[x][y] = (hi << 1) | lo; // from 0 to 3
            }
        }
    }
}

void TilemapViewer::render(bool* windowOpened) {
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_FirstUseEver);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (!ImGui::Begin("Debuggers - Tilemap Viewer", windowOpened)) {
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

    renderTileMap(tileMapHeight, tileMapAddress == 2 ? mapTiles2 : mapTiles1);
    renderTileMapInfo();

    ImGui::End();
}

void TilemapViewer::renderTileMap(float height, ImVector<TilemapItem> mapTiles) {
    ImGui::BeginChild("TileMap", ImVec2(0, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    float tileSizeZoom = 8.0f * zoomPerPixel;
    float gridGap = showGrid ? 1.0f : 0.0f;
    float tileStep = tileSizeZoom + gridGap;
    int tilesPerRow = 32;

    for (uint32_t t = 0; t < DMG_TilemapX * DMG_TilemapY; t++) {
        int tx = t % tilesPerRow;
        int ty = t / tilesPerRow;
        ImVec2 pos(start.x + tx * tileStep, start.y + ty * tileStep);

        const TileItem* tile = mapTiles[t].Tile;
        if (tile) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    PaletteColor color = paletteViewer.getColorPalette(tile->Pixels[x][y]);
                    ImU32 col = IM_COL32((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), 255);
                    ImVec2 p0(pos.x + x * zoomPerPixel, pos.y + y * zoomPerPixel);
                    ImVec2 p1(p0.x + zoomPerPixel, p0.y + zoomPerPixel);
                    draw_list->AddRectFilled(p0, p1, col);
                }
            }
        }

        ImVec2 tileEnd(pos.x + tileSizeZoom, pos.y + tileSizeZoom);
        if (ImGui::IsMouseHoveringRect(pos, tileEnd))
            hoveredTilemapItem = mapTiles[t];

        if (showGrid)
            draw_list->AddRect(pos, ImVec2(pos.x + tileSizeZoom, pos.y + tileSizeZoom), IM_COL32(60, 60, 60, 255));
    }

    ImVec2 viewportEnd(start.x + DMG_ViewportX * tileStep, start.y + DMG_ViewportY * tileStep);
    draw_list->AddRect(start, viewportEnd, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);

    int totalRows = (DMG_TilemapX * DMG_TilemapY + tilesPerRow - 1) / tilesPerRow;
    ImGui::Dummy(ImVec2(tilesPerRow * tileStep, totalRows * tileStep));

    ImGui::EndChild();
}

void renderTilemapTiles(ImVector<TilemapItem> mapTiles) {
}

void TilemapViewer::drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            PaletteColor color = paletteViewer.getColorPalette(tile.Pixels[x][y]);
            ImU32 col = IM_COL32((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), 255);
            ImVec2 p0(pos.x + x * pixelSize, pos.y + y * pixelSize);
            ImVec2 p1(p0.x + pixelSize, p0.y + pixelSize);
            draw_list->AddRectFilled(p0, p1, col);
        }
    }
    if (showGrid && drawBorder) {
        float s = pixelSize * 8.0f;
        draw_list->AddRect(pos, ImVec2(pos.x + s, pos.y + s), IM_COL32(60, 60, 60, 255));
    }
}

void TilemapViewer::renderTileMapInfo() {
    const float infoPaddingY = 8.0f;
    float infoStartY = ImGui::GetCursorPosY();

    ImGui::Separator();

    ImGui::Dummy(ImVec2(0, infoPaddingY));

    ImGuiTableFlags table_flags = ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("Settings", 2, table_flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

        float rowHeight = ImGui::GetFrameHeight();

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Tile map");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(120);
        static const char* windowTileMapAddresses[] = { "Auto", "$9800", "$9C00" };
        if (ImGui::Combo("##windowTileMapAddresses", &tileMapAddress, windowTileMapAddresses, IM_ARRAYSIZE(windowTileMapAddresses))) {
            settings.Set("Debuggers - Tile Viewer", "tile_map_address", tileMapAddress);
            settings.Save();
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Tile data");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(120);
        static const char* tileDataAddresses[] = { "Auto", "$8800", "$8000" };
        if (ImGui::Combo("##tileDataAddresses", &tileDataAddress, tileDataAddresses, IM_ARRAYSIZE(tileDataAddresses))) {
            settings.Set("Debuggers - Tile Viewer", "tile_data_address", tileDataAddress);
            settings.Save();
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Palette transformer");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(120);
        static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
        if (ImGui::Combo("##paletteChoicesCombo", &paletteViewer.paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
            settings.Set("Debuggers - Palette Viewer", "dmg_chosen_palette", paletteViewer.paletteChoicesSelected);
            settings.Save();
        }

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Display mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("...");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Tile map index");
        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        if (hoveredTilemapItem.Tile)
            ImGui::Text("%i (%02X) @ VRAM 00:%04X", hoveredTilemapItem.TileIndex, hoveredTilemapItem.TileIndex, hoveredTilemapItem.TileAddress);
        else
            ImGui::Text("");

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        textRightAligned("Tile index");
        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();
        if (hoveredTilemapItem.Tile)
            ImGui::Text("%i (%02X) @ VRAM 00:%04X", hoveredTilemapItem.Tile->TileItemID, hoveredTilemapItem.Tile->TileItemID, hoveredTilemapItem.Tile->TileAddress);
        else
            ImGui::Text("");

        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0, infoPaddingY));

    lastInfoHeight = ImGui::GetCursorPosY() - infoStartY;
}

void TilemapViewer::textRightAligned(const char* text) {
    float textWidth = ImGui::CalcTextSize(text).x;
    float avail = ImGui::GetContentRegionAvail().x;
    if (avail > textWidth)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - textWidth);
    ImGui::Text("%s", text);
}