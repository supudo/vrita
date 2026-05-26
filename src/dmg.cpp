#include "../include/dmg.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

// ============================================================
// Release
// ============================================================

void DMG::release(SDL_GPUDevice* device) {
    SDL_ReleaseGPUTexture(device, gTexture);
}

// ============================================================
// Create DMG texture
// ============================================================

bool DMG::createTexture(SDL_GPUDevice* device) {
    SDL_GPUTextureCreateInfo info={};
    info.type=SDL_GPU_TEXTURETYPE_2D;
    info.format=SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = DMG::WIDTH;
    info.height = DMG::HEIGHT;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    gTexture = SDL_CreateGPUTexture(device, &info);
    if (!gTexture) {
        printf("[DMG] Failed to create DMG texture: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

// ============================================================
// Fake emulator rendering
// Replace with your actual PPU output
// ============================================================

void DMG::generateTestPattern(float time) {
    for (uint32_t y = 0; y < DMG::HEIGHT; y++) {
        for (uint32_t x = 0; x < DMG::WIDTH; x++) {
            uint8_t r = (uint8_t)((x + (int)(time * 50.0f)) & 255);
            uint8_t g = (uint8_t)((y * 2) & 255);
            uint8_t b = (uint8_t)(128);
            gFramebuffer[y * DMG::WIDTH + x] = (255 << 24) | (b << 16) | (g << 8) | (r);
        }
    }
}

// ============================================================
// Upload framebuffer to GPU texture
// ============================================================

void DMG::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    uint32_t framebufferSize = DMG::WIDTH * DMG::HEIGHT * sizeof(uint32_t);

    SDL_GPUTransferBufferCreateInfo transferInfo = {};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = framebufferSize;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

    if (!transferBuffer) {
        printf("[DMG] Failed to create transfer buffer\n");
        return;
    }

    void* mapped=SDL_MapGPUTransferBuffer(device, transferBuffer, false);

    memcpy(mapped, gFramebuffer, framebufferSize);

    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCopyPass* copyPass=SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo source={};
    source.transfer_buffer=transferBuffer;
    source.offset=0;

    SDL_GPUTextureRegion destination={};
    destination.texture = gTexture;
    destination.w = DMG::WIDTH;
    destination.h = DMG::HEIGHT;
    destination.d = 1;

    SDL_UploadToGPUTexture(copyPass, &source, &destination, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
}

void DMG::run() {
    ImGui::Begin("GameBoy (DMG)");

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)DMG::WIDTH / (float)DMG::HEIGHT;
    float width = avail.x;
    float height = width / aspect;
    if (height > avail.y) {
        height = avail.y;
        width = height * aspect;
    }

    float scale = floorf(fminf(width / (float)DMG::WIDTH, height / (float)DMG::HEIGHT));

    if (scale < 1.0f)
        scale = 1.0f;

    width = DMG::WIDTH * scale;
    height = DMG::HEIGHT * scale;

    float cursorX = (avail.x - width) * 0.5f;
    if (cursorX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + cursorX);

    ImGui::Image((ImTextureID)gTexture, ImVec2(width, height));
    ImGui::End();
}
