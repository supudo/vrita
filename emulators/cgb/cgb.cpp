#include "cgb.hpp"

#include <iostream>

#include <imgui.h>
#include "third_party/imgui/imgui_impl_sdl2.h"
#include "emulators/dmg/palette_presets.hpp"

#include "utilities/iconfonts/IconsFontAwesome7.h"

bool CGB::initialize(int x, int y, int width, int height) {
    gameIsPaused = false;
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;
    droppedFrames = 0;
    droppedFramesPerSecond = 0;
    lastFPSTime = std::chrono::steady_clock::now();
    lastStepTime = std::chrono::steady_clock::now();
    frameAccumulator = 0.0;

    windowPositionX = x;
    windowPositionY = y;
    windowWidth = width;
    windowHeight = height;
    return true;
}

ImVec2 CGB::getWindowPosition() {
    return lastWindowPosition;
}

ImVec2 CGB::getWindowSize() {
    return lastWindowSize;
}

void CGB::release() {
    if (gTexture) {
        glDeleteTextures(1, &gTexture);
        gTexture = 0;
    }
}

bool CGB::createTexture() {
    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CGB::WIDTH, CGB::HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!gTexture) {
        logger.log("[CGB] Failed to create CGB texture");
        return false;
    }
    return true;
}

void CGB::generateTestPattern(float time) {
    if (ROMFileLoaded) return;
    for (uint32_t y = 0; y < CGB::HEIGHT; y++) {
        for (uint32_t x = 0; x < CGB::WIDTH; x++) {
            uint8_t r = (uint8_t)((x + (int)(time * 50.0f)) & 255);
            uint8_t g = (uint8_t)((y * 2) & 255);
            uint8_t b = (uint8_t)(128);
            gFramebuffer[y * CGB::WIDTH + x] = (255 << 24) | (b << 16) | (g << 8) | (r);
        }
    }
}

void CGB::uploadFramebufferToTexture() {
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CGB::WIDTH, CGB::HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, gFramebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CGB::run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused) {
    float imgW = (float)(CGB::WIDTH * windowScale);
    float imgH = (float)(CGB::HEIGHT * windowScale);

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
    cd = { (float)CGB::WIDTH / (float)CGB::HEIGHT, decorH, padX };

    ImGui::SetNextWindowPos(ImVec2((float)windowPositionX, (float)windowPositionY), ImGuiCond_Once);

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(padX + CGB::WIDTH, decorH + CGB::HEIGHT),
        ImVec2(FLT_MAX, FLT_MAX),
        [] (ImGuiSizeCallbackData* data) {
            auto* c = (ConstraintData*)data->UserData;
            float contentW = data->DesiredSize.x - c->padX;
            data->DesiredSize.y = contentW / c->aspect + c->decorH;
        },
        &cd
    );

    ImGui::Begin("GameBoy Color (CGB)", windowOpened);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        onFocused("cgb");

    if (ImGui::Button(ICON_FA_BOX_ARCHIVE, ImVec2(40, 32)))
        ImGui::OpenPopup("recentFiles");
    ImGui::SetItemTooltip("Recent Files");
    ImGui::SameLine();

    if (ImGui::BeginPopupContextItem("recentFiles")) {
        for (const auto& [key, value] : settings.GetSection("CGB - Recent Files")) {
            if (ImGui::Selectable(value.c_str()))
                loadROM(key.c_str());
            ImGui::SetItemTooltip(key.c_str());
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button(ICON_FA_ARROWS_DOWN_TO_LINE, ImVec2(40, 32)))
        showFileBrowser("cgb");
    ImGui::SetItemTooltip("Load ROM file");
    ImGui::SameLine();

    bool localRomFileLoaded = ROMFileLoaded;
    if (!localRomFileLoaded)
        ImGui::BeginDisabled();

    if (ImGui::Button(ICON_FA_EJECT, ImVec2(40, 32)))
        ROMFileLoaded = false;
    ImGui::SetItemTooltip("Eject ROM file");
    ImGui::SameLine();

    if (ImGui::Button(gameIsPaused ? ICON_FA_PLAY : ICON_FA_PAUSE, ImVec2(40, 32)))
        toggleGameState();
    ImGui::SameLine();

    if (ImGui::Button(isMuted() ? ICON_FA_VOLUME_XMARK : ICON_FA_VOLUME_HIGH, ImVec2(40, 32)))
        ImGui::OpenPopup("volumePopup");
    if (ImGui::BeginPopup("volumePopup")) {
        if (ImGui::Button(isMuted() ? "Unmute" : "Mute"))
            setMuted(!isMuted());
        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
        int sliderVolume = (int)getVolume();
        if (ImGui::VSliderInt("##volume", ImVec2(40, 160), &sliderVolume, 0, 100, "%d%%"))
            setVolume((uint8_t)sliderVolume);
        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }

    if (!localRomFileLoaded)
        ImGui::EndDisabled();

    ImGui::Separator();

    if (!ROMFileLoaded)
        ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(240);
    static const char* paletteChoices[] = { "Default", "DMG", "CGB", "MGB", "MGL" };
    if (ImGui::Combo("##palettecgb", &paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
        settings.Set("Emulators - CGB", "cgb_palette", paletteChoicesSelected);
        settings.Save();
    }
    if (!ROMFileLoaded)
        ImGui::EndDisabled();

    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)CGB::WIDTH / (float)CGB::HEIGHT;
    float dispW = avail.x;
    float dispH = dispW / aspect;
    if (dispH > avail.y - belowImageH) {
        dispH = avail.y - belowImageH;
        dispW = dispH * aspect;
    }
    float offX = (avail.x - dispW) * 0.5f;
    if (offX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);

    if (ROMFileLoaded && !gameIsPaused) {
        const double frameDuration = 1.0 / DMG_FPS;

        auto stepNow = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(stepNow - lastStepTime).count();
        lastStepTime = stepNow;

        double maxAccumulator = frameDuration * MAX_CATCHUP_FRAMES;
        frameAccumulator += dt;
        if (frameAccumulator > maxAccumulator) {
            droppedFrames += (uint32_t)((frameAccumulator - maxAccumulator) / frameDuration);
            frameAccumulator = maxAccumulator;
        }

        auto cpuStart = std::chrono::steady_clock::now();
        uint32_t framesStepped = 0;
        while (frameAccumulator >= frameDuration && framesStepped < MAX_CATCHUP_FRAMES) {
            {
#ifdef TRACY_ENABLE
                ZoneScopedN("CGB::EmulateFrame");
#endif
                // TODO
                //uint64_t frameStart = managerMMU->totalCycles;
                //while ((managerMMU->totalCycles - frameStart) < managerTimer->CYCLES_PER_FRAME)
                //    stepAll();
            }
            frameAccumulator -= frameDuration;
            renderingFrames++;
            framesStepped++;
        }
        auto cpuEnd = std::chrono::steady_clock::now();
        if (framesStepped > 0)
            lastFrameStepMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count() / framesStepped;

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - lastFPSTime).count();
        if (elapsed >= 1.0) {
            renderingFPS = renderingFrames / elapsed;
            renderingFrames = 0;
            lastFPSTime = now;
            renderingSpeed = (renderingFPS / DMG_FPS) * 100.0;
            droppedFramesPerSecond = droppedFrames;
            droppedFrames = 0;
#ifdef TRACY_ENABLE
            TracyPlot("FPS", renderingFPS);
            TracyPlot("Dropped Frames", (int64_t)droppedFramesPerSecond);
#endif
        }
    }

    ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest, nullptr);
    ImGui::Image((ImTextureID)(intptr_t)gTexture, ImVec2(dispW, dispH));
    ImGui::GetWindowDrawList()->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerLinear, nullptr);

    if (!ROMFileLoaded) {
        ImVec2 imgMin = ImGui::GetItemRectMin();
        ImVec2 imgMax = ImGui::GetItemRectMax();
        const char* overlayText = "Please, load or drag a ROM file";
        ImVec2 textSize = ImGui::CalcTextSize(overlayText);
        ImVec2 textPos(
            imgMin.x + (imgMax.x - imgMin.x - textSize.x) * 0.5f,
            imgMin.y + (imgMax.y - imgMin.y - textSize.y) * 0.5f
        );
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(imgMin, imgMax, IM_COL32(0, 0, 0, 120));
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), overlayText);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), overlayText);
    }

    float cursorYAfterImage = ImGui::GetCursorPosY();

    ImGui::Separator();

    ImGui::Text("START = <Enter>, SELECT = <space>");
    ImGui::Text("D-Pad = <arrow keys>, A = <A>, B = <B>");

    ImGui::Separator();

    ImGui::Text("FPS: %6.2f, Speed: %6.2f%%, Dropped: %3u/s", renderingFPS, renderingSpeed, droppedFramesPerSecond);

    lastBelowImageH = ImGui::GetCursorPosY() - cursorYAfterImage;
    lastDecorH = ImGui::GetWindowSize().y - dispH;

    ImGui::End();
}

bool CGB::initialize() {
    return true;
}

std::string CGB::loadROM(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        logger.log("[CGB] WARNING: Failed to open ROM: %s", path);
        return "Failed to open ROM";
    }
    clear();
    for (uint32_t i = 0; i < CGB::WIDTH * CGB::HEIGHT; i++)
        gFramebuffer[i] = DMG_PackForFramebuffer(DMG_PALETTE_DEFAULT[0]);
    std::streamsize size = file.tellg();
    std::streamsize memNeeded = std::max(size, (std::streamsize)0x10000);
    logger.log("[CGB] Loading ROM: %s", path);
    logger.log("[CGB] ROM size: %lld bytes (0x%llX), buffer: %lld bytes", (long long)size, (long long)size, (long long)memNeeded);
    ROMFileLoaded = true;
    gameIsPaused = false;
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;
    droppedFrames = 0;
    droppedFramesPerSecond = 0;
    lastFPSTime = std::chrono::steady_clock::now();
    lastStepTime = std::chrono::steady_clock::now();
    frameAccumulator = 0.0;
    return "";
}

void CGB::stepCPU() {
}

void CGB::clear() {
    ROMFileLoaded = false;
    renderingFrames = 0;
    renderingFPS = 0.0;
    renderingSpeed = 0.0;
    droppedFrames = 0;
    droppedFramesPerSecond = 0;
}

void CGB::toggleGameState() {
    gameIsPaused = !gameIsPaused;
    if (!gameIsPaused) {
        lastStepTime = std::chrono::steady_clock::now();
        frameAccumulator = 0.0;
    }
}

void CGB::stopGame() {
    gameIsPaused = true;
}

void CGB::startGame() {
    gameIsPaused = false;
    lastStepTime = std::chrono::steady_clock::now();
    frameAccumulator = 0.0;
}

bool CGB::isGameRunning() {
    return false;
}

void CGB::setVolume(uint8_t volume) {
    if (volume > 100)
        volume = 100;
    settings.Set("Emulators - CGB", "volume", (int)volume);
    settings.Save();
}

uint8_t CGB::getVolume() const {
    return 100;
}

void CGB::setMuted(bool muted) {
    settings.Set("Emulators - CGB", "muted", muted);
    settings.Save();
}

bool CGB::isMuted() const {
    return true;
}