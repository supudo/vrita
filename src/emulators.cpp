#include "../include/emulators.hpp"

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "../include/dmg.hpp"

std::shared_ptr<DMG> emulatorDMG;

void Emulators::init() {
    emulatorDMG = std::make_shared<DMG>();
}

bool Emulators::createTexture(SDL_GPUDevice* device) {
    if (!emulatorDMG->createTexture(device)) {
        printf("[EMULATORS] Error: Cannot create DMG texture\n");
        return false;
    }
    return true;
}

void Emulators::generateTestPattern(float time) {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->generateTestPattern(time);
}

void Emulators::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->uploadFramebufferToTexture(device, commandBuffer);
}

void Emulators::run() {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->run();
}

void Emulators::release(SDL_GPUDevice* device) {
    emulatorDMG->release(device);
}