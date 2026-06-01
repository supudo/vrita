#ifndef VRITA_EMULATORS_INCLUDES
#define VRITA_EMULATORS_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <functional>
#include <string>

class Emulators {
public:
    ~Emulators() = default;

    bool EMULATORS_SHOW_DMG = false;
    bool EMULATORS_SHOW_AGB = false;

    void init();
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void stepFrame();
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(const std::function<void(const char*)>& loadRom, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);
    std::string loadROM(const char* romFilePath);
};

#endif