#ifndef VRITA_PALETTEVIEWER_INCLUDES
#define VRITA_PALETTEVIEWER_INCLUDES

#include <array>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "palette_viewer_dmg.hpp"

class Settings;

class PaletteViewer {
public:
    PaletteViewer(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void release();
    void setMemory(const char* emulatorType, uint8_t bgp, uint8_t obp0, uint8_t obp1);
    void render(bool* windowOpened);

private:
    Logger& logger;
    Settings& settings;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint8_t emulatorType = 0;
    uint8_t paletteBGP = 0;
    uint8_t paletteOBP0 = 0;
    uint8_t paletteOBP1 = 0;

    int paletteChoicesSelected = 0;

    void renderCenteredCellContent(const char* lbl);
    void renderColorButtons(const char* label, uint8_t paletteValue);
    bool renderButtonWithBorder(const char* label, const ImVec2& size, PaletteColor background_color, PaletteColor border_color, float border_thickness = 2.0f);
    PaletteColor getColorPalette(uint8_t colorValue);

    inline static std::string rgbToHex(int r, int g, int b) { char buffer[8]; std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b); return buffer; }
};

#endif