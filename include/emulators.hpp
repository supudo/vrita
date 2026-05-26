#ifndef VRITA_EMULATORS_INCLUDES
#define VRITA_EMULATORS_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

class Emulators {
public:
    ~Emulators() = default;

    bool EMULATORS_SHOW_DMG = false;

    void init();
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run();
    void release(SDL_GPUDevice* device);
};

#endif