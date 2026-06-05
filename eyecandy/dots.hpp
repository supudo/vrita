#ifndef VRITA_DOTS_INCLUDES
#define VRITA_DOTS_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <vector>

class Dots {
public:
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float fwidth, float fheight, float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run();
    void release(SDL_GPUDevice* device);

private:
    SDL_GPUDevice* device = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint32_t> framebuffer;
    SDL_GPUTexture* gTexture = nullptr;
};

#endif
