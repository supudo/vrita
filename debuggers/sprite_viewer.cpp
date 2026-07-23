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
    zoomPerPixel = settings.GetInt("Debuggers - Sprite Viewer", "zoom_per_pixel", 3.0);
    return true;
}

void SpriteViewer::release() {
    settings.Set("Debuggers - Sprite Viewer", "position_x", (int)lastWindowPosition.x);
    settings.Set("Debuggers - Sprite Viewer", "position_y", (int)lastWindowPosition.y);
    settings.Set("Debuggers - Sprite Viewer", "width", (int)lastWindowSize.x);
    settings.Set("Debuggers - Sprite Viewer", "height", (int)lastWindowSize.y);
    settings.Set("Debuggers - Sprite Viewer", "zoom_per_pixel", zoomPerPixel);
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

void SpriteViewer::setCallbacks(std::function<uint8_t(uint16_t)> read8, std::function<void(uint16_t, uint8_t)> write8) {
    funcMemoryRead = read8;
    funcMemoryWrite = write8;
}

void SpriteViewer::initializeData(uint8_t emulatorType) {
    if (emulatorType == 1) {
        isSprite8x16 = funcMemoryRead(DMG_Address_LCDC) & 0x04;

        tiles.clear();
        tiles.reserve(DMG_TilesCount);
        const uint8_t* vramTiles = memoryData + DMG_Address_TileStart;
        for (uint32_t i = 0; i < DMG_TilesCount; i++) {
            const uint8_t* vramAddress = vramTiles + i * 16;
            uint16_t address = static_cast<uint16_t>(vramAddress - memoryData);
            TileItem tile(i, address);
            decodeTile(vramAddress, tile);
            tiles.push_back(tile);
        }

        const uint8_t* oam = memoryData + DMG_Address_SpritesStart;
        spriteItems.clear();
        spriteItems.reserve(DMG_SpriteCount);
        for (uint32_t i = 0; i < DMG_SpriteCount; ++i) {
            const uint8_t* entry = oam + i * 4;
            uint16_t address = static_cast<uint16_t>(entry - memoryData);
            const uint8_t tileIndex = entry[2];
            const uint8_t firstTile = isSprite8x16 ? (tileIndex & 0xFE) : tileIndex;
            SpriteItem spriteTile(i, entry[0], entry[1], tileIndex, entry[3], static_cast<uint8_t>(i), address, &tiles[firstTile], isSprite8x16 ? &tiles[firstTile + 1] : nullptr);
            spriteItems.push_back(spriteTile);
        }
    }
}

void SpriteViewer::decodeTile(const uint8_t* tileData, TileItem& tile) {
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
    ImGui::BeginChild("Sprites", ImVec2(0, height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 start = ImGui::GetCursorScreenPos();

    float spriteSizeZoom = 8.0f * zoomPerPixel;
    float gridGap = showGrid ? 1.0f : 0.0f;
    float spriteStep = spriteSizeZoom + gridGap;

    for (uint32_t t = 0; t < DMG_SpritesX * DMG_SpritesY; t++) {
        int tx = t % DMG_SpritesX;
        int ty = t / DMG_SpritesX;
        ImVec2 pos(start.x + tx * spriteStep, start.y + ty * spriteStep);

        //const TileItem* tile = spriteItems[t].TileTop;
        //if (tile) {
        //    for (int y = 0; y < 8; y++) {
        //        for (int x = 0; x < 8; x++) {
        //            PaletteColor color = paletteViewer.getColorPalette(tile->Pixels[x][y]);
        //            ImU32 col = IM_COL32((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), 255);
        //            ImVec2 p0(pos.x + x * zoomPerPixel, pos.y + y * zoomPerPixel);
        //            ImVec2 p1(p0.x + zoomPerPixel, p0.y + zoomPerPixel);
        //            draw_list->AddRectFilled(p0, p1, col);
        //        }
        //    }
        //}

        const TileItem* tile = spriteItems[t].TileTop;
        if (tile) {
            for (uint16_t i = 0; i < spriteItems.size(); i++) {
                ImVec2 pos(start.x + tx * spriteStep, start.y + ty * spriteStep);
                drawTileUnit(draw_list, spriteItems[t], pos, zoomPerPixel);
            }
        }

        if (showGrid)
            draw_list->AddRect(pos, ImVec2(pos.x + spriteSizeZoom, pos.y + spriteSizeZoom), IM_COL32(60, 60, 60, 255));
    }

    int totalRows = (DMG_SpritesX * DMG_SpritesY + DMG_SpritesX - 1) / DMG_SpritesX;
    ImGui::Dummy(ImVec2(DMG_SpritesX * spriteStep, totalRows * spriteStep));

    ImGui::EndChild();
}

void SpriteViewer::drawTileUnit(ImDrawList* draw_list, const SpriteItem& sprite, ImVec2 pos, float pixelSize) {
    if (sprite.TileTop)
        drawTile(draw_list, *sprite.TileTop, pos, pixelSize, false);
    if (isSprite8x16)
        drawTile(draw_list, *sprite.TileBottom, ImVec2(pos.x, pos.y + pixelSize * 8.0f), pixelSize, false);
    if (showGrid) {
        float w = pixelSize * 8.0f;
        float h = pixelSize * 8.0f * (isSprite8x16 ? 2.0f : 1.0f);
        draw_list->AddRect(pos, ImVec2(pos.x + w, pos.y + h), IM_COL32(60, 60, 60, 255));
    }
}

void SpriteViewer::drawTile(ImDrawList* draw_list, const TileItem& tile, ImVec2 pos, float pixelSize, bool drawBorder) {
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