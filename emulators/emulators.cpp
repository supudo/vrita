#include "emulators.hpp"

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "dmg/dmg.hpp"
#include "agb/agb.hpp"

#include "utilities/settings.hpp"
#include "debuggers/memoryviewer.hpp"

void Emulators::init(Settings& settings) {
    emulatorDMG = std::make_shared<DMG>(logger, settings);
    int dmgWindowPositionX = settings.GetInt("Emulators - DMG", "position_x", 44);
    int dmgWindowPositionY = settings.GetInt("Emulators - DMG", "position_y", 44);
    int dmgWindowSizeWidth = settings.GetInt("Emulators - DMG", "width", 300);
    int dmgWindowSizeHeight = settings.GetInt("Emulators - DMG", "height", 300);
    emulatorDMG->initialize(dmgWindowPositionX, dmgWindowPositionY, dmgWindowSizeWidth, dmgWindowSizeHeight);

    emulatorAGB = std::make_shared<AGB>(logger);
    int agbWindowPositionX = settings.GetInt("Emulators - AGB", "position_x", 44);
    int agbWindowPositionY = settings.GetInt("Emulators - AGB", "position_y", 44);
    int agbWindowSizeWidth = settings.GetInt("Emulators - AGB", "width", 300);
    int agbWindowSizeHeight = settings.GetInt("Emulators - AGB", "height", 300);
    emulatorAGB->initialize(agbWindowPositionX, agbWindowPositionY, agbWindowSizeWidth, agbWindowSizeHeight);

    EMULATORS_SHOW_DMG = settings.GetBool("Emulators", "show_dmg", false);
    EMULATORS_SHOW_AGB = settings.GetBool("Emulators", "show_agb", false);

    debuggerMemoryViewer = std::make_shared<MemoryViewer>(logger);
    debuggerMemoryViewer->init(settings);
    debuggersMemoryViewerVisible = settings.GetBool("Debuggers - Memory Viewer", "visible", false);
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
    if (EMULATORS_SHOW_DMG) {
        EMULATORS_SHOW_AGB = false;
        emulatorAGB->clear();
        emulatorDMG->run(&EMULATORS_SHOW_DMG, showFileBrowser, onFocused);
    }
    if (EMULATORS_SHOW_AGB) {
        EMULATORS_SHOW_DMG = false;
        emulatorDMG->clear();
        emulatorAGB->run(&EMULATORS_SHOW_AGB, showFileBrowser, onFocused);
    }

    if (EMULATORS_SHOW_DMG && emulatorDMG->managerMMU && emulatorDMG->ROMFileLoaded)
        debuggerMemoryViewer->setMemory(emulatorDMG->managerMMU->memory, DMG_MMU::MEMORY_SIZE);
    else
        debuggerMemoryViewer->setMemory(nullptr, 0);

    if (debuggersMemoryViewerVisible)
        debuggerMemoryViewer->render(&debuggersMemoryViewerVisible);
}

void Emulators::release(SDL_GPUDevice* device, Settings& settings) {
    settings.Set("Emulators", "show_dmg", EMULATORS_SHOW_DMG);
    settings.Set("Emulators", "show_agb", EMULATORS_SHOW_AGB);

    ImVec2 dmg_position = emulatorDMG->getWindowPosition();
    settings.Set("Emulators - DMG", "position_x", (int)dmg_position.x);
    settings.Set("Emulators - DMG", "position_y", (int)dmg_position.y);
    ImVec2 dmg_size = emulatorDMG->getWindowSize();
    settings.Set("Emulators - DMG", "width", (int)dmg_size.x);
    settings.Set("Emulators - DMG", "height", (int)dmg_size.y);

    ImVec2 agb_position = emulatorAGB->getWindowPosition();
    settings.Set("Emulators - AGB", "position_x", (int)agb_position.x);
    settings.Set("Emulators - AGB", "position_y", (int)agb_position.y);
    ImVec2 agb_size = emulatorAGB->getWindowSize();
    settings.Set("Emulators - AGB", "width", (int)agb_size.x);
    settings.Set("Emulators - AGB", "height", (int)agb_size.y);

    settings.Set("Debuggers - Memory Viewer", "visible", debuggersMemoryViewerVisible);

    settings.Save();

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