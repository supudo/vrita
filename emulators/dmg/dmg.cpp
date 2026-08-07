#include "dmg.hpp"

#include <algorithm>
#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>

#include <imgui.h>
#include "imgui/imgui_impl_sdl2.h"

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
    managerCPU = std::make_shared<DMG_CPU>(logger, *managerMMU, *managerInterrupts);
    managerPPU = std::make_shared<DMG_PPU>(logger, *managerMMU, *managerInterrupts);
    managerPPU->setFramebuffer(gFramebuffer);
    managerAPU = std::make_shared<DMG_APU>(logger, *managerMMU);
    managerCartridge = std::make_shared<DMG_CARTRIDGE>(logger, *managerMMU);
    managerJoypad = std::make_shared<DMG_JOYPAD>(logger, *managerMMU, *managerInterrupts);
    
    managerTimer->reset();

    managerMMU->setUnits(logger, *managerCartridge, *managerCPU, *managerTimer, *managerInterrupts, *managerPPU, *managerAPU, *managerJoypad);
    managerInterrupts->setCPURegisters(managerCPU->Registers);

    managerAPU->setUserVolume((uint8_t)settings.GetInt("Emulators - DMG", "volume", 100));
    managerAPU->setMuted(settings.GetBool("Emulators - DMG", "muted", false));

    paletteChoicesSelected = settings.GetInt("Emulators - DMG", "dmg_palette", 0);
    managerPPU->setPalette(paletteChoicesSelected);

    initAudio();

    return true;
}

bool DMG::initAudio() {
    SDL_AudioSpec audioSpec{};
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = 2;
    audioSpec.freq = 44100;
    audioSpec.samples = 2048;
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);
    if (!audioDevice) {
        logger.log("[DMG-APU] Cannot create audio device!");
        return false;
    }
    managerAPU->initAudioDevice(audioDevice);
    SDL_PauseAudioDevice(audioDevice, 0);
    return true;
}

ImVec2 DMG::getWindowPosition() {
    return lastWindowPosition;
}

ImVec2 DMG::getWindowSize() {
    return lastWindowSize;
}

void DMG::stepAll() {
#ifdef TRACY_ENABLE
    ZoneScopedN("DMG::stepAll");
#endif
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
    initAudio();
    for (uint32_t i = 0; i < DMG::WIDTH * DMG::HEIGHT; i++)
        gFramebuffer[i] = DMG_PackForFramebuffer(DMG_PALETTE_DEFAULT[0]);
    std::streamsize size = file.tellg();
    std::streamsize memNeeded = std::max(size, (std::streamsize)0x10000);
    logger.log("[DMG] Loading ROM: %s", path);
    logger.log("[DMG] ROM size: %lld bytes (0x%llX), buffer: %lld bytes", (long long)size, (long long)size, (long long)memNeeded);
    managerMMU->memory.resize((size_t)memNeeded, 0);
    managerMMU->memorySize = (uint32_t)managerMMU->memory.size();
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
    if (audioDevice) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
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

void DMG::setVolume(uint8_t volume) {
    if (volume > 100)
        volume = 100;
    managerAPU->setUserVolume(volume);
    settings.Set("Emulators - DMG", "volume", (int)volume);
    settings.Save();
}

uint8_t DMG::getVolume() const {
    return managerAPU->getUserVolume();
}

void DMG::setMuted(bool muted) {
    managerAPU->setMuted(muted);
    settings.Set("Emulators - DMG", "muted", muted);
    settings.Save();
}

bool DMG::isMuted() const {
    return managerAPU->isMuted();
}

void DMG::handleKey(uint32_t type, uint32_t key) {
    if (ROMFileLoaded)
        managerJoypad->handleKey(type, key);
}

void DMG::release() {
    if (gTexture) {
        glDeleteTextures(1, &gTexture);
        gTexture = 0;
    }
}

bool DMG::createTexture() {
    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DMG::WIDTH, DMG::HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!gTexture) {
        logger.log("[DMG] Failed to create DMG texture");
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

void DMG::uploadFramebufferToTexture() {
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DMG::WIDTH, DMG::HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, gFramebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);
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

    if (ImGui::Button(ICON_FA_MOBILE_SCREEN_BUTTON, ImVec2(40, 32)))
        renderJoypad = !renderJoypad;
    ImGui::SetItemTooltip("Toggle D-Pad");

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
    if (ImGui::Combo("##palettedmg", &paletteChoicesSelected, paletteChoices, IM_ARRAYSIZE(paletteChoices))) {
        settings.Set("Emulators - DMG", "dmg_palette", paletteChoicesSelected);
        settings.Save();
        managerPPU->setPalette(paletteChoicesSelected);
    }
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
        {
#ifdef TRACY_ENABLE
            ZoneScopedN("DMG::EmulateFrame");
#endif
            uint64_t frameStart = managerMMU->totalCycles;
            while ((managerMMU->totalCycles - frameStart) < managerTimer->CYCLES_PER_FRAME)
                stepAll();
        }
        renderingFrames++;
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - lastTime).count();
        if (elapsed >= 1.0) {
            renderingFPS = renderingFrames / elapsed;
            renderingFrames = 0;
            lastTime = now;
            renderingSpeed = (renderingFPS / DMG_FPS) * 100.0;
#ifdef TRACY_ENABLE
            TracyPlot("FPS", renderingFPS);
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

    if (renderJoypad) {
        ImGui::Separator();
        renderJoypadUI();
    }

    ImGui::Separator();

    ImGui::Text("START = <Enter>, SELECT = <space>");
    ImGui::Text("D-Pad = <arrow keys>, A = <A>, B = <B>");

    ImGui::Separator();

    ImGui::Text("FPS: %.2f, Speed: %.2f%%", renderingFPS, renderingSpeed);

    lastBelowImageH = ImGui::GetCursorPosY() - cursorYAfterImage;
    lastDecorH = ImGui::GetWindowSize().y - dispH;

    ImGui::End();
}

void DMG::renderJoypadUI() {
    float availW = ImGui::GetContentRegionAvail().x;
    float scale = availW > 0.0f ? availW / JOYPAD_UI_WIDTH : 1.0f;
    scale = std::clamp(scale, 0.4f, 2.5f);

    ImVec2 size(JOYPAD_UI_WIDTH * scale, JOYPAD_UI_HEIGHT * scale);

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

    auto V = [&](float x, float y) { return ImVec2(x, y) * scale; };

    draw->AddRectFilled(origin, origin + size, bodyColor, 18.0f * scale);

    // d-pad

    ImVec2 dCenter = origin + V(90, 95);
    float iconFontSize = ImGui::GetFontSize() * scale;

    auto DrawPadPiece = [&](const char* id, ImVec2 pos, ImVec2 sz, const char* icon, uint8_t joypadButton) {
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton(id, sz);
        bool down = ImGui::IsItemActive();
        if (ROMFileLoaded)
            managerJoypad->setButton(joypadButton, down);
        draw->AddRectFilled(pos, pos + sz, down ? buttonPressed : buttonColor, 5.0f * scale);
        ImVec2 iconSize = ImGui::GetFont()->CalcTextSizeA(iconFontSize, FLT_MAX, 0.0f, icon);
        draw->AddText(ImGui::GetFont(), iconFontSize, pos + (sz - iconSize) * 0.5f, IM_COL32_WHITE, icon);
    };

    float padGap = 2.0f * scale;
    DrawPadPiece("UP", dCenter + V(-15, -60), V(30, 45) - ImVec2(0, padGap), ICON_FA_ARROW_UP, DMG_JOYPAD::JOYPAD_UP);
    DrawPadPiece("DOWN", dCenter + V(-15, 15) + ImVec2(0, padGap), V(30, 45) - ImVec2(0, padGap), ICON_FA_ARROW_DOWN, DMG_JOYPAD::JOYPAD_DOWN);
    DrawPadPiece("LEFT", dCenter + V(-60, -15), V(45, 30) - ImVec2(padGap, 0), ICON_FA_ARROW_LEFT, DMG_JOYPAD::JOYPAD_LEFT);
    DrawPadPiece("RIGHT", dCenter + V(15, -15) + ImVec2(padGap, 0), V(45, 30) - ImVec2(padGap, 0), ICON_FA_ARROW_RIGHT, DMG_JOYPAD::JOYPAD_RIGHT);
    draw->AddRectFilled(dCenter + V(-15, -15), dCenter + V(15, 15), buttonColor, 4.0f * scale);

    // button B

    float letterFontSize = ImGui::GetFontSize() * 1.8f * scale;
    float faceRadius = 28.0f * scale;

    ImVec2 bPos = origin + V(300, 115);
    ImGui::SetCursorScreenPos(bPos - ImVec2(faceRadius, faceRadius));
    ImGui::InvisibleButton("B", ImVec2(faceRadius * 2.0f, faceRadius * 2.0f));
    bool bHeld = ImGui::IsItemActive();
    if (ROMFileLoaded)
        managerJoypad->setButton(DMG_JOYPAD::JOYPAD_B, bHeld);
    draw->AddCircleFilled(bPos, faceRadius, bHeld ? purplePressed : purple, 40);

    ImVec2 bLetterSize = ImGui::GetFont()->CalcTextSizeA(letterFontSize, FLT_MAX, 0.0f, ICON_FA_B);
    draw->AddText(ImGui::GetFont(), letterFontSize, bPos - bLetterSize * 0.5f, IM_COL32_WHITE, ICON_FA_B);

    // button A

    ImVec2 aPos = origin + V(370, 80);

    ImGui::SetCursorScreenPos(aPos - ImVec2(faceRadius, faceRadius));
    ImGui::InvisibleButton("A", ImVec2(faceRadius * 2.0f, faceRadius * 2.0f));
    bool aHeld = ImGui::IsItemActive();
    if (ROMFileLoaded)
        managerJoypad->setButton(DMG_JOYPAD::JOYPAD_A, aHeld);
    draw->AddCircleFilled(aPos, faceRadius, aHeld ? purplePressed : purple, 40);
    ImVec2 aLetterSize = ImGui::GetFont()->CalcTextSizeA(letterFontSize, FLT_MAX, 0.0f, ICON_FA_A);
    draw->AddText(ImGui::GetFont(), letterFontSize, aPos - aLetterSize * 0.5f, IM_COL32_WHITE, ICON_FA_A);

    // buttons SELECT and START

    float pillFontSize = ImGui::GetFontSize() * scale;

    auto DrawPill = [&](const char* id, ImVec2 center, const char* label, uint8_t joypadButton) {
        ImVec2 pillSize = V(56, 32);
        ImVec2 p = center - pillSize * 0.5f;
        ImGui::SetCursorScreenPos(p);
        ImGui::InvisibleButton(id, pillSize);
        bool held = ImGui::IsItemActive();
        if (ROMFileLoaded)
            managerJoypad->setButton(joypadButton, held);
        draw->AddRectFilled(p, p + pillSize, held ? buttonPressed : buttonColor, 8.0f * scale);
        ImVec2 labelSize = ImGui::GetFont()->CalcTextSizeA(pillFontSize, FLT_MAX, 0.0f, label);
        ImVec2 labelPos = p + ImVec2((pillSize.x - labelSize.x) * 0.5f, pillSize.y + 6.0f * scale);
        draw->AddText(ImGui::GetFont(), pillFontSize, labelPos, textColor, label);
    };
    DrawPill("SELECT", origin + V(170, 190), "SELECT", DMG_JOYPAD::JOYPAD_SELECT);
    DrawPill("START", origin + V(260, 190), "START", DMG_JOYPAD::JOYPAD_START);

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
