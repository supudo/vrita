#include "emulators.hpp"

#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "dmg/dmg.hpp"
#include "agb/agb.hpp"

#include "utilities/settings.hpp"
#include "debuggers/memory_editor.hpp"

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

    debuggerMemoryEditor = std::make_shared<MemoryEditor>(logger, settings);
    debuggerMemoryEditor->init();
    debuggersMemoryEditorVisible = settings.GetBool("Debuggers - Memory Editor", "visible", false);

    debuggerPaletteViewer = std::make_shared<PaletteViewer>(logger, settings);
    debuggerPaletteViewer->init();
    debuggerPaletteViewerVisible = settings.GetBool("Debuggers - Palette Viewer", "visible", false);

    debuggerTileViewer = std::make_shared<TileViewer>(logger, settings, *debuggerPaletteViewer);
    debuggerTileViewer->init();
    debuggerTileViewerVisible = settings.GetBool("Debuggers - Tile Viewer", "visible", false);

    debuggerTilemapViewer = std::make_shared<TilemapViewer>(logger, settings, *debuggerPaletteViewer);
    debuggerTilemapViewer->init();
    debuggerTilemapViewerVisible = settings.GetBool("Debuggers - Tilemap Viewer", "visible", false);

    debuggerSpriteViewer = std::make_shared<SpriteViewer>(logger, settings, *debuggerPaletteViewer);
    debuggerSpriteViewer->init();
    debuggerSpriteViewerVisible = settings.GetBool("Debuggers - Sprite Viewer", "visible", false);

    debuggerDebugger = std::make_shared<Debugger>(logger, settings);
    debuggerDebugger->init();
    debuggerDebuggerVisible = settings.GetBool("Debuggers - Debugger", "visible", false);
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

    if (EMULATORS_SHOW_DMG && emulatorDMG->managerMMU && emulatorDMG->managerCPU && emulatorDMG->ROMFileLoaded) {
        debuggerMemoryEditor->setMemory("dmg", emulatorDMG->managerMMU->memory.data(), emulatorDMG->managerMMU->memorySize);
        debuggerMemoryEditor->setCallbacks(
            [&] (uint32_t addr) {
                return emulatorDMG->managerMMU->read8(static_cast<uint16_t>(addr), true);
            },
            [&] (uint32_t addr, uint8_t value) {
                emulatorDMG->managerMMU->write16(static_cast<uint16_t>(addr), value, true);
            }
        );
        debuggerMemoryEditor->setRegsiterCallback(
            [&] (const char* name) -> uint16_t {
            auto& r = emulatorDMG->managerCPU->Registers;
                if (strcmp(name, "BC") == 0) return r.BC;
                if (strcmp(name, "DE") == 0) return r.DE;
                if (strcmp(name, "HL") == 0) return r.HL;
                if (strcmp(name, "SP") == 0) return r.SP;
                if (strcmp(name, "PC") == 0) return r.PC;
                return 0;
            }
        );
        debuggerPaletteViewer->setMemory("dmg", emulatorDMG->managerMMU->memory[0xFF47], emulatorDMG->managerMMU->memory[0xFF48], emulatorDMG->managerMMU->memory[0xFF49]);
        debuggerTileViewer->setMemory("dmg", emulatorDMG->managerMMU->memory.data());
        debuggerTilemapViewer->setMemory("dmg", emulatorDMG->managerMMU->memory.data());
        debuggerSpriteViewer->setCallbacks(
            [&] (uint32_t addr) {
                return emulatorDMG->managerMMU->read8(static_cast<uint16_t>(addr), true);
            },
            [&] (uint32_t addr, uint8_t value) {
                emulatorDMG->managerMMU->write16(static_cast<uint16_t>(addr), value, true);
            }
        );
        debuggerSpriteViewer->setMemory("dmg", emulatorDMG->managerMMU->memory.data());
        debuggerDebugger->setCallbacks(
            [&] (uint32_t addr) {
                return emulatorDMG->managerMMU->read8(static_cast<uint16_t>(addr), true);
            },
            [&] (uint32_t addr, uint8_t value) {
                emulatorDMG->managerMMU->write16(static_cast<uint16_t>(addr), value, true);
            },
            [&] (uint8_t flag) {
                return emulatorDMG->managerCPU->isFlagSet(flag);
            },
            [&] (uint8_t flag) {
                return emulatorDMG->managerInterrupts->isInterruptEnabled(flag);
            },
            [&] () {
                return emulatorDMG->isGameRunning();
            },
            [&] () {
                return emulatorDMG->stopGame();
            },
            [&] () {
                return emulatorDMG->startGame();
            },
            [&] (bool val) {
                return emulatorDMG->logCPUCalls(val);
            }
        );
        debuggerDebugger->setMemory("dmg", emulatorDMG->managerMMU->memorySize);
    }
    else {
        debuggerMemoryEditor->setMemory("agb", nullptr, 0);
        debuggerDebugger->setMemory("agb", 0);
    }

    if (debuggersMemoryEditorVisible)
        debuggerMemoryEditor->render(&debuggersMemoryEditorVisible);
    if (debuggerTileViewerVisible)
        debuggerTileViewer->render(&debuggerTileViewerVisible);
    if (debuggerTilemapViewerVisible)
        debuggerTilemapViewer->render(&debuggerTilemapViewerVisible);
    if (debuggerSpriteViewerVisible)
        debuggerSpriteViewer->render(&debuggerSpriteViewerVisible);
    if (debuggerPaletteViewerVisible)
        debuggerPaletteViewer->render(&debuggerPaletteViewerVisible);
    if (debuggerDebuggerVisible && emulatorDMG && emulatorDMG->managerCPU && emulatorDMG && emulatorDMG->managerInterrupts) {
        debuggerDebugger->render(&debuggerDebuggerVisible, emulatorDMG->managerCPU->Registers);
    }
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

    settings.Set("Debuggers - Memory Editor", "visible", debuggersMemoryEditorVisible);
    settings.Set("Debuggers - Tile Viewer", "visible", debuggerTileViewerVisible);
    settings.Set("Debuggers - Tilemap Viewer", "visible", debuggerTilemapViewerVisible);
    settings.Set("Debuggers - Sprite Viewer", "visible", debuggerSpriteViewerVisible);
    settings.Set("Debuggers - Palette Viewer", "visible", debuggerPaletteViewerVisible);
    settings.Set("Debuggers - Debugger", "visible", debuggerDebuggerVisible);

    settings.Save();

    emulatorDMG->release(device);
    emulatorAGB->release(device);
    debuggerMemoryEditor->release();
    debuggerTileViewer->release();
    debuggerTilemapViewer->release();
    debuggerSpriteViewer->release();
    debuggerPaletteViewer->release();
    debuggerDebugger->release();
}

std::string Emulators::loadROM(const char* romFilePath) {
    std::string result = "";
    if (EMULATORS_SHOW_DMG)
        result = emulatorDMG->loadROM(romFilePath);
    if (EMULATORS_SHOW_AGB)
        result = emulatorAGB->loadROM(romFilePath);
    return result;
}

void Emulators::handleKey(uint32_t type, uint32_t key) {
    emulatorDMG->handleKey(type, key);
}