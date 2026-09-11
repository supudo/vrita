#ifndef VRITA_PALETTEVIEWER_INCLUDES
#define VRITA_PALETTEVIEWER_INCLUDES

#include <array>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <functional>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "debuggers_defines.hpp"

class Settings;

class PaletteViewer {
public:
    PaletteViewer(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void release();
    void setCallbacks(std::function<const uint8_t* (bool)> getPaletteRAM);
    void setMemory(const char* emuType, uint8_t bgp, uint8_t obp0, uint8_t obp1, bool isCGB);
    void render(bool* windowOpened);

    PaletteColor getColorPalette(uint8_t colorValue);

    int paletteChoicesSelected = 0;

private:
    Logger& logger;
    Settings& settings;

    bool autoRefresh = true;
    float zoomPerPixel = 1.0f;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 488;
    int windowHeight = 357;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint8_t emulatorType = 0;
    uint8_t paletteBGP = 0;
    uint8_t paletteOBP0 = 0;
    uint8_t paletteOBP1 = 0;

    PaletteColor unpackPaletteColor(uint32_t packed);

    void renderCenteredCellContent(const char* lbl, float rowHeight = 80.0f);
    void renderColorButtons(const char* label, uint8_t paletteValue);
    bool renderButtonWithBorder(const char* label, const ImVec2& size, PaletteColor background_color, PaletteColor border_color = { 1.0f, 1.0f, 1.0f }, float border_thickness = 2.0f);

    inline static std::string rgbToHex(int r, int g, int b) { char buffer[8]; std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b); return buffer; }

    bool isCGBLoaded = false;
    std::function<const uint8_t* (bool isOBJ)> funcGetPaletteRAM;
    PaletteColor resolveCGBColor(uint8_t paletteNum, bool isOBJ, uint8_t colorId) const;
    void renderCGBPaletteButtons(const char* label, uint8_t paletteNum, bool isOBJ);
};

#endif