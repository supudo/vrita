#include "dmg.hpp"

#include <fstream>
#include <vector>
#include <cstdint>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

bool DMG::initialize() {
    managerMMU = std::make_shared<DMG_MMU>();
    managerMMU->clearResources();

    managerInterrupts = std::make_shared<DMG_INTERRUPT>(*managerMMU);
    managerTimer = std::make_shared<DMG_TIMER>(*managerInterrupts);
    managerCPU = std::make_shared<DMG_CPU>(logger, *managerMMU, *managerInterrupts, false, 0);
    managerPPU = std::make_shared<DMG_PPU>(*managerMMU);
    managerAPU = std::make_shared<DMG_APU>(*managerMMU);
    managerCartridge = std::make_shared<DMG_CARTRIDGE>(logger, *managerMMU);
    
    managerTimer->reset();

    managerMMU->setUnits(managerCartridge.get(), managerCPU.get(), managerTimer.get(), managerInterrupts.get(), managerPPU.get(), managerAPU.get());

    return true;
}

void DMG::stepAll() {
    if (ROMFileLoaded) {
        uint32_t cycles = 0;
        if (!managerInterrupts->checkForInterrupts()) {
            cycles = stepCPU();
            if (cycles == 0) return; // unsupported opcode
        }
        stepPPU(cycles);
        stepAPU(cycles);
    }
    //else
    //    managerTimer->tick();
}

std::string DMG::loadROM(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        logger.log("[DMG] WARNING: Failed to open ROM: %s", path);
        return "Failed to open ROM";
    }
    std::streamsize size = file.tellg();
    if (size <= 0 || size > 0x8000) {
        logger.log("[DMG] WARNING: Invalid ROM size: %i", (int)size, " bytes");
        file.close();
        return "Invalid ROM size";
    }
    resetROM();
    for (uint32_t i = 0; i < DMG::WIDTH * DMG::HEIGHT; i++)
        gFramebuffer[i] = 0xFF9BBC0F;
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(managerMMU->memory), size)) {
        logger.log("[DMG] WARNING: Failed to read ROM data!");
        file.close();
        return "Failed to read ROM data";
    }
    file.close();
    ROMFileLoaded = true;
    managerCartridge->loadROM(size);
    return "";
}

void DMG::resetROM() {
    ROMFileLoaded = false;
    managerCPU->clearResources();
    managerMMU->clearResources();
    managerPPU->clearResources();
    managerAPU->clearResources();
}

uint32_t DMG::stepCPU() {
    uint64_t before = managerCPU->cycles;
    if (managerCPU->halted)
        managerCPU->cycles += 4;
    else
        managerCPU->stepCPU(ROMFileLoaded);
    return (uint32_t)(managerCPU->cycles - before);
}

void DMG::stepMMU(uint32_t cycles) {
    managerMMU->tick(cycles);
}

void DMG::stepPPU(uint32_t cycles) {}

void DMG::stepAPU(uint32_t cycles) {}

#pragma region Rendering
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
    if (ROMFileLoaded) return;
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

void DMG::run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused) {
    float imgW = (float)(DMG::WIDTH * windowScale);
    float imgH = (float)(DMG::HEIGHT * windowScale);

    ImGuiStyle& style = ImGui::GetStyle();
    float decorH = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.0f + ImGui::GetFrameHeight() + style.ItemSpacing.y + 1.0f;
    float padX = style.WindowPadding.x * 2.0f;

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
        [] (ImGuiSizeCallbackData* data) {
        auto* c = (ConstraintData*)data->UserData;
        float contentW = data->DesiredSize.x - c->padX;
        data->DesiredSize.y = contentW / c->aspect + c->decorH;
    },
        &cd
    );

    ImGui::Begin("GameBoy (DMG)", windowOpened);
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        onFocused("dmg");

    if (ImGui::Button("Load ROM file", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        showFileBrowser("dmg");
    if (ImGui::Button("Eject ROM file", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        ROMFileLoaded = false;

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

    stepAll();

    ImGui::Image((ImTextureID)gTexture, ImVec2(dispW, dispH));

    ImGui::End();
}
#pragma endregion