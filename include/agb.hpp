/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_AGB_INCLUDES
#define VRITA_AGB_INCLUDES

#include "emulators.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <functional>
#include <iostream>
#include <stdint.h>

class AGB {
public:
    // rendering
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool *windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);

    // DMG specifics
    bool initialize();
    std::string loadROM(const char* romFilePath);
    void stepCPU();

private:
    // rendering
    static const uint32_t WIDTH = 240;
    static const uint32_t HEIGHT = 160;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;

    int windowScale = 1;
    int lastWindowScale = -1;
};

#endif