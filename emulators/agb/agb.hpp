/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_AGB_INCLUDES
#define VRITA_AGB_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <imgui.h>

#include "emulators/emulators.hpp"
#include "utilities/logger.hpp"

class AGB {
public:
    AGB(Logger& logger) : logger(logger) {}

    bool initialize(int x, int y, int width, int height);
    ImVec2 getWindowPosition();
    ImVec2 getWindowSize();

    // rendering
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool *windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);
    void clear();

    // DMG specifics
    bool initialize();
    std::string loadROM(const char* romFilePath);
    void stepCPU();

private:
    Logger& logger;

    // rendering
    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    static const uint32_t WIDTH = 240;
    static const uint32_t HEIGHT = 160;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;
    int windowScale = 1;
    int lastWindowScale = -1;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);
};

#endif