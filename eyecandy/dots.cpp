#include "dots.hpp"

#include <algorithm>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

void Dots::release(SDL_GPUDevice* device) {
    if (gTexture) {
        SDL_ReleaseGPUTexture(device, gTexture);
        gTexture = nullptr;
    }
}

bool Dots::createTexture(SDL_GPUDevice* device) {
    this->device = device;
    width = 160;
    height = 144;
    framebuffer.resize(width * height);

    SDL_GPUTextureCreateInfo info = {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    gTexture = SDL_CreateGPUTexture(device, &info);
    return gTexture != nullptr;
}

void Dots::generateTestPattern(float width, float height, float time) {
    uint32_t w = (uint32_t)std::max(1.0f, width);
    uint32_t h = (uint32_t)std::max(1.0f, height);

    if (w != width || h != height) {
        width = w;
        height = h;
        framebuffer.resize(width * height);

        if (gTexture) {
            SDL_ReleaseGPUTexture(device, gTexture);
            gTexture = nullptr;
        }

        SDL_GPUTextureCreateInfo info = {};
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.width = width;
        info.height = height;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
        gTexture = SDL_CreateGPUTexture(device, &info);
    }

    for (uint32_t i = 0, c = 0; i < height; i++)
        for (uint32_t j = 0; j < width; j++, c++)
            framebuffer[c] = (uint32_t)(i * i + j * j + (uint32_t)time) | 0xff000000;
}

void Dots::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    if (!gTexture || framebuffer.empty())
        return;

    const uint32_t size = width * height * sizeof(uint32_t);

    SDL_GPUTransferBufferCreateInfo tbInfo = {};
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.size = size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    if (!transfer)
        return;

    void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
    std::memcpy(mapped, framebuffer.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer);

    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo src = {};
    src.transfer_buffer = transfer;
    src.offset = 0;

    SDL_GPUTextureRegion dst = {};
    dst.texture = gTexture;
    dst.w = width;
    dst.h = height;
    dst.d = 1;

    SDL_UploadToGPUTexture(copy, &src, &dst, false);
    SDL_EndGPUCopyPass(copy);
    SDL_ReleaseGPUTransferBuffer(device, transfer);
}

void Dots::run() {
    ImGui::Begin("Dots (EyeCandy)");
    ImVec2 avail = ImGui::GetContentRegionAvail();
    generateTestPattern(avail.x, avail.y, (float)SDL_GetTicks());
    if (gTexture)
        ImGui::Image(gTexture, avail);
    ImGui::End();
}
