#include "../include/dmg.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

void DMG::release(SDL_GPUDevice* device) {
    SDL_ReleaseGPUTexture(device, gTexture);
}

bool DMG::createTexture(SDL_GPUDevice* device) {
    SDL_GPUTextureCreateInfo info = {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
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

    void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    memcpy(mapped, gFramebuffer, framebufferSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo source = {};
    source.transfer_buffer = transferBuffer;
    source.offset = 0;

    SDL_GPUTextureRegion destination = {};
    destination.texture = gTexture;
    destination.w = DMG::WIDTH;
    destination.h = DMG::HEIGHT;
    destination.d = 1;

    SDL_UploadToGPUTexture(copyPass, &source, &destination, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
}

void DMG::run(bool* windowOpened) {
    float imgW = (float)(DMG::WIDTH  * windowScale);
    float imgH = (float)(DMG::HEIGHT * windowScale);

    ImGuiStyle& style = ImGui::GetStyle();
    float decorH = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.0f + ImGui::GetFrameHeight() + style.ItemSpacing.y + 1.0f;
    float padX   = style.WindowPadding.x * 2.0f;

    if (windowScale != lastWindowScale && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImGui::SetNextWindowSize(ImVec2(imgW + padX, imgH + decorH), ImGuiCond_Always);
        lastWindowScale = windowScale;
    }

    struct ConstraintData { float aspect; float decorH; float padX; };
    static ConstraintData cd;
    cd = { (float)DMG::WIDTH / (float)DMG::HEIGHT, decorH, padX };

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(padX + DMG::WIDTH, decorH + DMG::HEIGHT),
        ImVec2(FLT_MAX, FLT_MAX),
        [](ImGuiSizeCallbackData* data) {
            auto* c = (ConstraintData*)data->UserData;
            float contentW = data->DesiredSize.x - c->padX;
            data->DesiredSize.y = contentW / c->aspect + c->decorH;
        },
        &cd
    );

    ImGui::Begin("GameBoy (DMG)", windowOpened);

    ImGui::SliderInt("Scale", &windowScale, 1, 20);
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)DMG::WIDTH / (float)DMG::HEIGHT;
    float dispW = avail.x;
    float dispH = dispW / aspect;
    if (dispH > avail.y) {
        dispH = avail.y;
        dispW = dispH * aspect;
    }
    float offX = (avail.x - dispW) * 0.5f;
    if (offX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);

    ImGui::Image((ImTextureID)gTexture, ImVec2(dispW, dispH));

    ImGui::End();
}
