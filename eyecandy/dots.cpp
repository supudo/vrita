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
    mDevice = device;
    mWidth = 160;
    mHeight = 144;
    mFramebuffer.resize(mWidth * mHeight);

    SDL_GPUTextureCreateInfo info = {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = mWidth;
    info.height = mHeight;
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

    if (w != mWidth || h != mHeight) {
        mWidth = w;
        mHeight = h;
        mFramebuffer.resize(mWidth * mHeight);

        if (gTexture) {
            SDL_ReleaseGPUTexture(mDevice, gTexture);
            gTexture = nullptr;
        }

        SDL_GPUTextureCreateInfo info = {};
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.width = mWidth;
        info.height = mHeight;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
        gTexture = SDL_CreateGPUTexture(mDevice, &info);
    }

    for (uint32_t i = 0, c = 0; i < mHeight; i++)
        for (uint32_t j = 0; j < mWidth; j++, c++)
            mFramebuffer[c] = (uint32_t)(i * i + j * j + (uint32_t)time) | 0xff000000;
}

void Dots::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    if (!gTexture || mFramebuffer.empty())
        return;

    const uint32_t size = mWidth * mHeight * sizeof(uint32_t);

    SDL_GPUTransferBufferCreateInfo tbInfo = {};
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.size = size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    if (!transfer)
        return;

    void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
    std::memcpy(mapped, mFramebuffer.data(), size);
    SDL_UnmapGPUTransferBuffer(device, transfer);

    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo src = {};
    src.transfer_buffer = transfer;
    src.offset = 0;

    SDL_GPUTextureRegion dst = {};
    dst.texture = gTexture;
    dst.w = mWidth;
    dst.h = mHeight;
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
