/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_INCLUDES
#define VRITA_DMG_INCLUDES

#include <stdint.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <imgui.h>

#include "emulators/emulators.hpp"
#include "utilities/logger.hpp"

#include "apu.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "interrupt.hpp"
#include "mmu.hpp"
#include "ppu.hpp"
#include "timer.hpp"

class DMG {
public:
    DMG(Logger& logger) : logger(logger) {}

    bool initialize(int x, int y, int width, int height);
    ImVec2 getWindowPosition();
    ImVec2 getWindowSize();

    // rendering
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);
    void clear();

    // DMG specifics
    std::string loadROM(const char* romFilePath);

    std::shared_ptr<DMG_TIMER> managerTimer;
    std::shared_ptr<DMG_INTERRUPT> managerInterrupts;
    std::shared_ptr<DMG_CARTRIDGE> managerCartridge;
    std::shared_ptr<DMG_CPU> managerCPU;
    std::shared_ptr<DMG_APU> managerAPU;
    std::shared_ptr<DMG_MMU> managerMMU;
    std::shared_ptr<DMG_PPU> managerPPU;

    bool ROMFileLoaded = false;

private:
    Logger& logger;
    void stepAll();

    void toggleGameState();
    std::string gameStateLabel = "Pause game";
    bool gameIsPaused = false;
    std::string logCallsLabel = "Log CPU calls (OFF)";

    void stepCPU();
    void stepMMU(uint32_t cycles);
    void stepPPU(uint32_t cycles);
    void stepAPU(uint32_t cycles);

    // rendering
    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    static const uint32_t WIDTH = 160;
    static const uint32_t HEIGHT = 144;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;
    int windowScale = 1;
    int lastWindowScale = -1;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);
};

#endif