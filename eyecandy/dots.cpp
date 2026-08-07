#include "dots.hpp"

#include <algorithm>
#include <imgui.h>
#include "imgui/imgui_impl_sdl2.h"

void Dots::release() {
    if (gTexture) {
        glDeleteTextures(1, &gTexture);
        gTexture = 0;
    }
}

static GLuint Dots_CreateTexture(uint32_t width, uint32_t height) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

bool Dots::createTexture() {
    width = 160;
    height = 144;
    framebuffer.resize(width * height);
    gTexture = Dots_CreateTexture(width, height);
    return gTexture != 0;
}

void Dots::generateTestPattern(float fwidth, float fheight, float time) {
    uint32_t w = (uint32_t)std::max(1.0f, fwidth);
    uint32_t h = (uint32_t)std::max(1.0f, fheight);

    if (w != width || h != height) {
        width = w;
        height = h;
        framebuffer.resize((size_t)width * height);

        if (gTexture) {
            glDeleteTextures(1, &gTexture);
            gTexture = 0;
        }

        gTexture = Dots_CreateTexture(width, height);
    }

    for (uint32_t i = 0, c = 0; i < height; i++)
        for (uint32_t j = 0; j < width; j++, c++)
            framebuffer[c] = (uint32_t)(i * i + j * j + (uint32_t)time) | 0xff000000;
}

void Dots::uploadFramebufferToTexture() {
    if (!gTexture || framebuffer.empty())
        return;

    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Dots::run() {
    ImGui::Begin("Dots (EyeCandy)");
    ImVec2 avail = ImGui::GetContentRegionAvail();
    generateTestPattern(avail.x, avail.y, (float)SDL_GetTicks());
    if (gTexture)
        ImGui::Image((ImTextureID)(intptr_t)gTexture, avail);
    ImGui::End();
}
