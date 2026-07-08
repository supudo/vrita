#include "dmg.hpp"

#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include "utilities/iconfonts/IconsFontAwesome7.h"

bool DMG::initialize(int x, int y, int width, int height) {
    gameIsPaused = false;
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
    managerJoypad = std::make_shared<DMG_JOYPAD>(logger, *managerMMU, *managerInterrupts);
    
    managerTimer->reset();

    managerMMU->setUnits(logger, *managerCartridge, *managerCPU, *managerTimer, *managerInterrupts, *managerPPU, *managerAPU, *managerJoypad);
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
    managerJoypad->clearResources();
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
}

void DMG::stopGame() {
    gameIsPaused = true;
}

void DMG::startGame() {
    gameIsPaused = false;
}

bool DMG::isGameRunning() {
    return !gameIsPaused;
}

void DMG::logCPUCalls(bool isOn) {
    managerCPU->logCalls = isOn;
}

void DMG::handleKey(uint32_t type, uint32_t key) {
    if (ROMFileLoaded)
        managerJoypad->handleKey(type, key);
}

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
    static float lastDecorH = 150.0f;
    static float lastBelowImageH = 300.0f;
    float decorH = lastDecorH;
    float belowImageH = lastBelowImageH;
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

    if (ImGui::Button(ICON_FA_BOX_ARCHIVE, ImVec2(40, 32)))
        ImGui::OpenPopup("recentFiles");
    ImGui::SetItemTooltip("Recent Files");
    ImGui::SameLine();

    if (ImGui::BeginPopupContextItem("recentFiles")) {
        for (const auto& [key, value] : settings.GetSection("DMG - Recent Files")) {
            if (ImGui::Selectable(value.c_str()))
                loadROM(key.c_str());
            ImGui::SetItemTooltip(key.c_str());
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button(ICON_FA_ARROWS_DOWN_TO_LINE, ImVec2(40, 32)))
        showFileBrowser("dmg");
    ImGui::SetItemTooltip("Load ROM file");
    ImGui::SameLine();

    if (!ROMFileLoaded)
        ImGui::BeginDisabled();

    if (ImGui::Button(ICON_FA_EJECT, ImVec2(40, 32)))
        ROMFileLoaded = false;
    ImGui::SetItemTooltip("Eject ROM file");
    ImGui::SameLine();

    if (ImGui::Button(gameIsPaused ? ICON_FA_PLAY : ICON_FA_PAUSE, ImVec2(40, 32)))
        toggleGameState();
    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_MOBILE_SCREEN_BUTTON, ImVec2(40, 32)))
        renderJoypad = !renderJoypad;
    ImGui::SetItemTooltip("Toggle D-Pad");

    if (!ROMFileLoaded)
        ImGui::EndDisabled();
    
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)DMG::WIDTH / (float)DMG::HEIGHT;
    float dispW = avail.x;
    float dispH = dispW / aspect;
    if (dispH > avail.y - belowImageH) {
        dispH = avail.y - belowImageH;
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

    float cursorYAfterImage = ImGui::GetCursorPosY();

    if (renderJoypad) {
        ImGui::Separator();
        renderJoypadUI();
    }

    ImGui::Separator();

    ImGui::Text("START = <Enter>, SELECT = <space>");
    ImGui::Text("D-Pad = <arrow keys>");
    ImGui::Text("A = <A>, B = <B>");

    ImGui::Separator();

    ImGui::Text("FPS: %.2f, Speed: %.2f%%", renderingFPS, renderingSpeed);

    lastBelowImageH = ImGui::GetCursorPosY() - cursorYAfterImage;
    lastDecorH = ImGui::GetWindowSize().y - dispH;

    ImGui::End();
}

void DMG::renderJoypadUI() {
    ImVec2 size(JOYPAD_UI_WIDTH, JOYPAD_UI_HEIGHT);

    float offX = (ImGui::GetContentRegionAvail().x - size.x) * 0.5f;
    if (offX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);

    ImGui::BeginChild("##joypad", size, ImGuiChildFlags_None);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    ImU32 bodyColor = IM_COL32(197, 199, 183, 255);
    ImU32 buttonColor = IM_COL32(45, 45, 45, 255);
    ImU32 buttonPressed = IM_COL32(20, 20, 20, 255);
    ImU32 purple = IM_COL32(93, 53, 95, 255);
    ImU32 purplePressed = IM_COL32(65, 35, 70, 255);
    ImU32 textColor = IM_COL32(25, 25, 25, 255);

    draw->AddRectFilled(origin, origin + size, bodyColor, 18.0f);

    // d-pad

    ImVec2 dCenter = origin + ImVec2(90, 95);

    auto DrawPadPiece = [&](const char* id, ImVec2 pos, ImVec2 sz, const char* icon) {
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton(id, sz);
        bool down = ImGui::IsItemActive();
        draw->AddRectFilled(pos, pos + sz, down ? buttonPressed : buttonColor, 5);
        ImVec2 iconSize = ImGui::CalcTextSize(icon);
        draw->AddText(pos + (sz - iconSize) * 0.5f, IM_COL32_WHITE, icon);
    };

    constexpr float PAD_GAP = 2.0f;
    DrawPadPiece("UP", dCenter + ImVec2(-15, -60), ImVec2(30, 45 - PAD_GAP), ICON_FA_ARROW_UP);
    DrawPadPiece("DOWN", dCenter + ImVec2(-15, 15 + PAD_GAP), ImVec2(30, 45 - PAD_GAP), ICON_FA_ARROW_DOWN);
    DrawPadPiece("LEFT", dCenter + ImVec2(-60, -15), ImVec2(45 - PAD_GAP, 30), ICON_FA_ARROW_LEFT);
    DrawPadPiece("RIGHT", dCenter + ImVec2(15 + PAD_GAP, -15), ImVec2(45 - PAD_GAP, 30), ICON_FA_ARROW_RIGHT);
    draw->AddRectFilled(dCenter + ImVec2(-15, -15), dCenter + ImVec2(15, 15), buttonColor, 4);

    // button B

    ImVec2 bPos = origin + ImVec2(300, 115);
    ImGui::SetCursorScreenPos(bPos - ImVec2(28, 28));
    ImGui::InvisibleButton("B", ImVec2(56, 56));
    bool bHeld = ImGui::IsItemActive();
    draw->AddCircleFilled(bPos, 28, bHeld ? purplePressed : purple, 40);

    float letterFontSize = ImGui::GetFontSize() * 1.8f;
    ImVec2 bLetterSize = ImGui::GetFont()->CalcTextSizeA(letterFontSize, FLT_MAX, 0.0f, ICON_FA_B);
    draw->AddText(ImGui::GetFont(), letterFontSize, bPos - bLetterSize * 0.5f, IM_COL32_WHITE, ICON_FA_B);

    // button A

    ImVec2 aPos = origin + ImVec2(370, 80);

    ImGui::SetCursorScreenPos(aPos - ImVec2(28, 28));
    ImGui::InvisibleButton("A", ImVec2(56, 56));

    bool aHeld = ImGui::IsItemActive();

    draw->AddCircleFilled(aPos, 28, aHeld ? purplePressed : purple, 40);
    ImVec2 aLetterSize = ImGui::GetFont()->CalcTextSizeA(letterFontSize, FLT_MAX, 0.0f, ICON_FA_A);
    draw->AddText(ImGui::GetFont(), letterFontSize, aPos - aLetterSize * 0.5f, IM_COL32_WHITE, ICON_FA_A);

    // buttons SELECT and START

    auto DrawPill = [&](const char* id, ImVec2 center, const char* label) {
        ImVec2 p = center - ImVec2(28, 16);
        ImGui::SetCursorScreenPos(p);
        ImGui::InvisibleButton(id, ImVec2(56, 32));
        bool held = ImGui::IsItemActive();
        draw->AddRectFilled(p, p + ImVec2(56, 32), held ? buttonPressed : buttonColor, 8);
        float labelX = (56.0f - ImGui::CalcTextSize(label).x) * 0.5f;
        draw->AddText(p + ImVec2(labelX, 38), textColor, label);
    };
    DrawPill("SELECT", origin + ImVec2(170, 190), "SELECT");
    DrawPill("START", origin + ImVec2(260, 190), "START");

    // speaker lines

    //ImVec2 dirRaw(18.0f, 6.0f);
    //float dirLen = sqrtf(dirRaw.x * dirRaw.x + dirRaw.y * dirRaw.y);
    //ImVec2 dirUnit(dirRaw.x / dirLen, dirRaw.y / dirLen);
    //ImVec2 perpUnit(-dirUnit.y, dirUnit.x);
    //constexpr float LINE_SPACING = 10.0f;
    //constexpr float EXTEND = 40.0f;
    //for (int i = 0; i < 6; i++) {
    //    ImVec2 base = origin + ImVec2(355, 180) + perpUnit * (i * LINE_SPACING);
    //    ImVec2 lineStart = base - dirUnit * EXTEND;
    //    ImVec2 lineEnd = base + dirRaw + dirUnit * EXTEND;
    //    draw->AddLine(lineStart, lineEnd, IM_COL32(120, 120, 120, 255), 2.0f);
    //}

    ImGui::EndChild();
}
