/*

GameBoy Advance (AGB)

*/

#ifndef VRITA_AGB_INCLUDES
#define VRITA_AGB_INCLUDES

#include "emulators.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

class AGB {
public:
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool *windowOpened);
    void release(SDL_GPUDevice* device);

private:
    static const uint32_t WIDTH = 240;
    static const uint32_t HEIGHT = 160;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;

    int windowScale = 1;
    int lastWindowScale = -1;
};

#endif