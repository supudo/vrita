/*

GameBoy

*/

#ifndef VRITA_DMG_INCLUDES
#define VRITA_DMG_INCLUDES

#include "emulators.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

class DMG {
public:
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run();
    void release(SDL_GPUDevice* device);

private:
    static const uint32_t WIDTH = 160;
    static const uint32_t HEIGHT = 144;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;

    void runFrame();
};

#endif