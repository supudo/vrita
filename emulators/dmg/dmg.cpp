#include "dmg.hpp"

#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

bool DMG::initialize(int x, int y, int width, int height) {
    gameIsPaused = false;
    gameStateLabel = "Pause game";
    logCallsLabel = "Log CPU calls (OFF)";
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;

    windowPositionX = x;
    windowPositionY = y;
    windowWidth = width;
    windowHeight = height;

    managerMMU = std::make_shared<DMG_MMU>();
    managerMMU->clearResources();

    managerInterrupts = std::make_shared<DMG_INTERRUPT>(*managerMMU);
    managerTimer = std::make_shared<DMG_TIMER>(logger, *managerInterrupts);
    managerCPU = std::make_shared<DMG_CPU>(logger, *managerMMU, *managerInterrupts, false, 0);
    managerPPU = std::make_shared<DMG_PPU>(logger, *managerMMU, *managerInterrupts);
    managerPPU->setFramebuffer(gFramebuffer);
    managerAPU = std::make_shared<DMG_APU>(*managerMMU);
    managerCartridge = std::make_shared<DMG_CARTRIDGE>(logger, *managerMMU);
    
    managerTimer->reset();

    managerMMU->setUnits(logger, *managerCartridge, *managerCPU, *managerTimer, *managerInterrupts, *managerPPU, *managerAPU);
    managerInterrupts->setCPURegisters(managerCPU->Registers);

    return true;
}

ImVec2 DMG::getWindowPosition() {
    return lastWindowPosition;
}

ImVec2 DMG::getWindowSize() {
    return lastWindowSize;
}

void DMG::stepAll() {
    if (ROMFileLoaded) {
        uint64_t before = managerMMU->totalCycles;
        if (!managerInterrupts->checkForInterrupts())
            stepCPU();
        uint32_t elapsed = (uint32_t)(managerMMU->totalCycles - before);
        stepPPU(elapsed);
        stepAPU(elapsed);
    }
}

std::string DMG::loadROM(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        logger.log("[DMG-CPU] WARNING: Failed to open ROM: %s", path);
        return "Failed to open ROM";
    }
    clear();
    for (uint32_t i = 0; i < DMG::WIDTH * DMG::HEIGHT; i++)
        gFramebuffer[i] = 0xFF9BBC0F;
    std::streamsize size = file.tellg();
    std::streamsize memNeeded = std::max(size, (std::streamsize)0x10000);
    logger.log("[DMG] Loading ROM: %s", path);
    logger.log("[DMG] ROM size: %lld bytes (0x%llX), buffer: %lld bytes", (long long)size, (long long)size, (long long)memNeeded);
    managerMMU->memory.resize((size_t)memNeeded, 0);
    logger.log("[DMG] Memory buffer resized to %zu bytes", managerMMU->memory.size());
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(managerMMU->memory.data()), size)) {
        logger.log("[DMG-CPU] WARNING: Failed to read ROM data!");
        file.close();
        return "Failed to read ROM data";
    }
    file.close();
    logger.log("[DMG] ROM read into memory. Type byte @ 0x147: 0x%02X", managerMMU->memory[0x147]);
    managerCartridge->loadROM(size);
    managerMMU->resetRegisters();
    logger.log("[DMG] Hardware registers restored. ROM loaded.");
    ROMFileLoaded = true;
    gameIsPaused = false;
    gameStateLabel = "Pause game";
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;
    return "";
}

void DMG::clear() {
    ROMFileLoaded = false;
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;
    managerCPU->clearResources();
    managerMMU->clearResources();
    managerPPU->clearResources();
    managerAPU->clearResources();
    managerTimer->reset();
    managerCartridge->clearResources();
}

void DMG::stepCPU() {
    if (managerMMU->isHalted) {
        managerMMU->tick(4);
        return;
    }
    managerCPU->step(ROMFileLoaded);
}

void DMG::stepMMU(uint32_t cycles) {
    managerMMU->tick(cycles);
}

void DMG::stepPPU(uint32_t cycles) {
    managerPPU->step(ROMFileLoaded, cycles);
}

void DMG::stepAPU(uint32_t cycles) {
    managerAPU->step(ROMFileLoaded, cycles);
}

void DMG::toggleGameState() {
    gameIsPaused = !gameIsPaused;
    gameStateLabel = gameIsPaused ? "Run Game" : "Pause Game";
}

void DMG::stopGame() {
    gameIsPaused = true;
    gameStateLabel = gameIsPaused ? "Run Game" : "Pause Game";
}

void DMG::startGame() {
    gameIsPaused = false;
    gameStateLabel = gameIsPaused ? "Run Game" : "Pause Game";
}

bool DMG::isGameRunning() {
    return !gameIsPaused;
}

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
        logger.log("[DMG] Failed to create DMG texture: %s", SDL_GetError());
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
        logger.log("[DMG] Failed to create transfer buffer");
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

    if (windowScale != lastWindowScale) {
        if (lastWindowScale == -1)
            ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_Once);
        else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            ImGui::SetNextWindowSize(ImVec2(imgW + padX, imgH + decorH), ImGuiCond_Always);
        lastWindowScale = windowScale;
    }

    struct ConstraintData { float aspect; float decorH; float padX; };
    static ConstraintData cd;
    cd = { (float)DMG::WIDTH / (float)DMG::HEIGHT, decorH, padX };

    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_Once);

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

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        onFocused("dmg");
    if (ImGui::Button("Recent Files"))
        ImGui::OpenPopup("recentFiles");
    ImGui::SameLine();

    if (ImGui::BeginPopupContextItem("recentFiles")) {
        for (const auto& [key, value] : settings.GetSection("DMG - Recent Files")) {
            if (ImGui::Selectable(value.c_str()))
                loadROM(key.c_str());
            ImGui::SetItemTooltip(key.c_str());
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Load ROM file"))
        showFileBrowser("dmg");
    ImGui::SameLine();
    if (ImGui::Button("Eject ROM file"))
        ROMFileLoaded = false;
    
    ImGui::Separator();

    if (ImGui::Button(gameStateLabel.c_str(), ImVec2(60, 0)))
        toggleGameState();
    ImGui::SameLine();

    ImGui::Separator();

    ImGui::SameLine();
    ImGui::Text("Game Speed");
    ImGui::SameLine();
    ImGui::Button("0.5x");
    ImGui::SameLine();
    ImGui::Button("1x");
    ImGui::SameLine();
    ImGui::Button("2x");

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    if (ImGui::Button(logCallsLabel.c_str())) {
        managerCPU->logCalls = !managerCPU->logCalls;
        logCallsLabel = managerCPU->logCalls ? "Log CPU calls (ON)" : "Log CPU calls (OFF)";
    }

    ImGui::Separator();

    ImGui::Text("FPS: %.2f, Speed: %.2f%%", renderingFPS, renderingSpeed);
    
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

    static auto lastTime = std::chrono::steady_clock::now();
    if (ROMFileLoaded && !gameIsPaused) {
        static const uint64_t CYCLES_PER_FRAME = 70224; // 154 lines * 456 dots @ 4.194304 MHz / 59.7275 fps
        uint64_t frameStart = managerMMU->totalCycles;
        while ((managerMMU->totalCycles - frameStart) < CYCLES_PER_FRAME)
            stepAll();
        renderingFrames++;
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - lastTime).count();
        if (elapsed >= 1.0) {
            renderingFPS = renderingFrames / elapsed;
            renderingFrames = 0;
            lastTime = now;
            renderingSpeed = (renderingFPS / DMG_FPS) * 100.0;
        }
    }

    ImGui::Image((ImTextureID)gTexture, ImVec2(dispW, dispH));

    ImGui::End();
}
#pragma endregion