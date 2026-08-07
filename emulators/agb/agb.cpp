#include "agb.hpp"

#include <imgui.h>
#include "imgui/imgui_impl_sdl2.h"

bool AGB::initialize(int x, int y, int width, int height) {
    windowPositionX = x;
    windowPositionY = y;
    windowWidth = width;
    windowHeight = height;
    return true;
}

ImVec2 AGB::getWindowPosition() {
    return lastWindowPosition;
}

ImVec2 AGB::getWindowSize() {
    return lastWindowSize;
}

void AGB::release() {
    if (gTexture) {
        glDeleteTextures(1, &gTexture);
        gTexture = 0;
    }
}

bool AGB::createTexture() {
    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, AGB::WIDTH, AGB::HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!gTexture) {
        logger.log("[AGB] Failed to create AGB texture");
        return false;
    }
    return true;
}

void AGB::generateTestPattern(float time) {
    for (uint32_t y = 0; y < AGB::HEIGHT; y++) {
        for (uint32_t x = 0; x < AGB::WIDTH; x++) {
            uint8_t r = (uint8_t)((x + (int)(time * 50.0f)) & 255);
            uint8_t g = (uint8_t)((y * 2) & 255);
            uint8_t b = (uint8_t)(128);
            gFramebuffer[y * AGB::WIDTH + x] = (255 << 24) | (b << 16) | (g << 8) | (r);
        }
    }
}

void AGB::uploadFramebufferToTexture() {
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, AGB::WIDTH, AGB::HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, gFramebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AGB::run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused) {
    float imgW = (float)(AGB::WIDTH * windowScale);
    float imgH = (float)(AGB::HEIGHT * windowScale);

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
    cd = { (float)AGB::WIDTH / (float)AGB::HEIGHT, decorH, padX };

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(padX + AGB::WIDTH, decorH + AGB::HEIGHT),
        ImVec2(FLT_MAX, FLT_MAX),
        [] (ImGuiSizeCallbackData* data) {
            auto* c = (ConstraintData*)data->UserData;
            float contentW = data->DesiredSize.x - c->padX;
            data->DesiredSize.y = contentW / c->aspect + c->decorH;
        },
        &cd
    );

    ImGui::Begin("GameBoy Advance (AGB)", windowOpened);

    lastWindowPosition = ImGui::GetWindowPos();
    lastWindowSize = ImGui::GetWindowSize();

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        onFocused("agb");

    ImGui::SliderInt("Scale", &windowScale, 1, 20);

    if (ImGui::Button("Load ROM file", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        showFileBrowser("agb");

    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)AGB::WIDTH / (float)AGB::HEIGHT;
    float dispW = avail.x;
    float dispH = dispW / aspect;
    if (dispH > avail.y) {
        dispH = avail.y;
        dispW = dispH * aspect;
    }
    float offX = (avail.x - dispW) * 0.5f;
    if (offX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);

    ImGui::Image((ImTextureID)(intptr_t)gTexture, ImVec2(dispW, dispH));

    ImGui::End();
}

// ===============
// AGB
// ===============

bool AGB::initialize() {
    return true;
}

std::string AGB::loadROM(const char* romFilePath) {
    return "";
}

void AGB::stepCPU() {
}

void AGB::clear() {
}