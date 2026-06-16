#ifndef VRITA_EMULATORS_INCLUDES
#define VRITA_EMULATORS_INCLUDES

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <functional>
#include <string>

#include "utilities/logger.hpp"
#include "debuggers/memory_editor.hpp"
#include "debuggers/tile_viewer.hpp"
#include "debuggers/tilemap_viewer.hpp"
#include "debuggers/sprite_viewer.hpp"
#include "debuggers/palette_viewer.hpp"
#include "debuggers/debugger.hpp"

class Settings;
class DMG;
class AGB;

class Emulators {
public:
    Emulators(Logger& logger) : logger(logger) {}

    bool EMULATORS_SHOW_DMG = false;
    bool EMULATORS_SHOW_AGB = false;

    void init(Settings& settings);
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(const std::function<void(const char*)>& loadRom, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device, Settings& settings);
    std::string loadROM(const char* romFilePath);

    std::shared_ptr<MemoryEditor> debuggerMemoryEditor;
    bool debuggersMemoryEditorVisible = false;

    std::shared_ptr<TileViewer> debuggerTileViewer;
    bool debuggerTileViewerVisible = false;

    std::shared_ptr<TilemapViewer> debuggerTilemapViewer;
    bool debuggerTilemapViewerVisible = false;

    std::shared_ptr<SpriteViewer> debuggerSpriteViewer;
    bool debuggerSpriteViewerVisible = false;

    std::shared_ptr<PaletteViewer> debuggerPaletteViewer;
    bool debuggerPaletteViewerVisible = false;

    std::shared_ptr<Debugger> debuggerDebugger;
    bool debuggerDebuggerVisible = false;

private:
    Logger& logger;

    std::shared_ptr<DMG> emulatorDMG;
    std::shared_ptr<AGB> emulatorAGB;
};

#endif