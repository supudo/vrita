/*

GameBoy Color (CGB)

*/

#ifndef VRITA_CGB_INCLUDES
#define VRITA_CGB_INCLUDES

#include <SDL2/SDL.h>
#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <functional>
#include <stdint.h>
#include <imgui.h>

#include "emulators/emulators.hpp"
#include "utilities/logger.hpp"

class CGB {
public:
    CGB(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool initialize(int x, int y, int width, int height);
    ImVec2 getWindowPosition();
    ImVec2 getWindowSize();

    // rendering
    bool createTexture();
    void generateTestPattern(float time);
    void uploadFramebufferToTexture();
    void run(bool *windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release();
    void clear();

    // CGB specifics
    bool initialize();
    std::string loadROM(const char* path);
    void stepCPU();

    bool ROMFileLoaded = false;
    void stopGame();
    void startGame();
    bool isGameRunning();

    void setVolume(uint8_t volume);
    uint8_t getVolume() const;
    void setMuted(bool muted);
    bool isMuted() const;

private:
    Logger& logger;
    Settings& settings;

    // rendering
    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    static const uint32_t WIDTH = 240;
    static const uint32_t HEIGHT = 160;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    GLuint gTexture = 0;
    int windowScale = 1;
    int lastWindowScale = -1;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    uint32_t renderingFrames = 0;
    double renderingFPS = 0.0;
    double renderingSpeed = 0.0;
    const double DMG_FPS = 59.7275;

    uint32_t droppedFrames = 0;
    uint32_t droppedFramesPerSecond = 0;

    std::chrono::steady_clock::time_point lastFPSTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastStepTime = std::chrono::steady_clock::now();
    double frameAccumulator = 0.0;
    static const uint32_t MAX_CATCHUP_FRAMES = 4;

    bool gameIsPaused = false;
    void toggleGameState();

    int paletteChoicesSelected = 0;
    double lastFrameStepMs = 0.0;
};

#endif