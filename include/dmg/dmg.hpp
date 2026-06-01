/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_INCLUDES
#define VRITA_DMG_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

#include "../emulators.hpp"
#include "../logger.hpp"

#include "mmu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "apu.hpp"

class DMG {
public:
    bool initialize(Logger& logger);

    // rendering
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);

    // DMG specifics
    std::string loadROM(const char* romFilePath);

private:
    Logger* logger = nullptr;

    void stepCPU();
    void stepPPU();
    void stepAPU();

    // rendering
    static const uint32_t WIDTH = 160;
    static const uint32_t HEIGHT = 144;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;
    int windowScale = 1;
    int lastWindowScale = -1;

    //cpu
    bool ime = false; // interrupt master enable

    DMG_CPU *managerCPU = nullptr;
    DMG_APU *managerAPU = nullptr;
    DMG_MMU *managerMMU = nullptr;
    DMG_PPU *managerPPU = nullptr;
};

#endif