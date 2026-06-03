#include "emulators.hpp"

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "dmg/dmg.hpp"
#include "agb/agb.hpp"

std::shared_ptr<DMG> emulatorDMG;
std::shared_ptr<AGB> emulatorAGB;

void Emulators::init() {
    emulatorDMG = std::make_shared<DMG>(logger);
    emulatorDMG->initialize();
    emulatorAGB = std::make_shared<AGB>(logger);
    emulatorAGB->initialize();
}

bool Emulators::createTexture(SDL_GPUDevice* device) {
    if (!emulatorDMG->createTexture(device)) {
        logger.log("[EMULATORS] Error: Cannot create DMG texture");
        return false;
    }
    if (!emulatorAGB->createTexture(device)) {
        logger.log("[EMULATORS] Error: Cannot create AGB texture");
        return false;
    }
    return true;
}

void Emulators::generateTestPattern(float time) {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->generateTestPattern(time);
    if (EMULATORS_SHOW_AGB)
        emulatorAGB->generateTestPattern(time);
}

void Emulators::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->uploadFramebufferToTexture(device, commandBuffer);
    if (EMULATORS_SHOW_AGB)
        emulatorAGB->uploadFramebufferToTexture(device, commandBuffer);
}

void Emulators::run(const std::function<void(const char*)>& loadRom, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused) {
    if (EMULATORS_SHOW_DMG)
        emulatorDMG->run(&EMULATORS_SHOW_DMG, showFileBrowser, onFocused);
    if (EMULATORS_SHOW_AGB)
        emulatorAGB->run(&EMULATORS_SHOW_AGB, showFileBrowser, onFocused);
}

void Emulators::release(SDL_GPUDevice* device) {
    emulatorDMG->release(device);
    emulatorAGB->release(device);
}

std::string Emulators::loadROM(const char* romFilePath) {
    if (EMULATORS_SHOW_DMG)
        return emulatorDMG->loadROM(romFilePath);
    if (EMULATORS_SHOW_AGB)
        return emulatorAGB->loadROM(romFilePath);
    return "";
}