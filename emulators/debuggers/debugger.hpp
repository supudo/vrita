#ifndef VRITA_DEBUGGER_INCLUDES
#define VRITA_DEBUGGER_INCLUDES

#include <stdint.h>
#include <cstdint>
#include <string>
#include <vector>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "debuggers_defines.hpp"

class Settings;
struct DMGCpuRegisters;
struct MemoryRegion;

class Debugger {
public:
    Debugger(Logger& logger, Settings& settings) : logger(logger), settings(settings) {}

    bool init();
    void setCallbacks(std::function<uint8_t(uint16_t)> read8,
                      std::function<void(uint16_t, uint8_t)> write8,
                      std::function<bool(uint8_t)> getFlag,
                      std::function<bool(uint8_t)> interruptsEnabled);
    void setMemory(const char* emulatorType, uint32_t size);
    void release();
    void render(bool* windowOpened, DMGCpuRegisters& registers);

private:
    Logger& logger;
    Settings& settings;

    int windowPositionX = 40;
    int windowPositionY = 40;
    int windowWidth = 300;
    int windowHeight = 300;
    ImVec2 lastWindowPosition = ImVec2(44, 44);
    ImVec2 lastWindowSize = ImVec2(300, 300);

    std::function<uint8_t(uint32_t)> funcMemoryRead;
    std::function<void(uint32_t, uint8_t)> funcMemoryWrite;
    std::function<bool(uint8_t)> funcCpuGetFlag;
    std::function<bool(uint8_t)> funcInterruptsEnabled;

    uint32_t memorySize = 0;
    uint8_t emulatorType = 0;

    float memoryPanelHeight = 260.0f;

    bool running = false;
    int selectedMemoryRegion = 0;

    void renderPerspective(DMGCpuRegisters& registers);
    void renderAssembly();

    void renderRest();
    void renderRestMemory();
    void renderRestCustomExpression();
    void renderRestBreakpoints();
    void renderRestOverlays();
    void renderMemoryRegion();

    void initRegisters();
    void renderRegisters(DMGCpuRegisters& registers);
    void renderRegisterNode(DebuggerRegisterTreeNode* node, bool isRoot = false);
    void renderRegisterValue(DebuggerRegisterTreeNode* node);

    std::vector<DebuggerRegisterTreeNode> registerNodes;
    uint8_t getAddressValue8(uint32_t address) const;
    void renderFlags(DebuggerRegisterTreeNode* node);
    void renderInterrupts(DebuggerRegisterTreeNode* node);
    void renderLCDCBit(DebuggerRegisterTreeNode* node, uint8_t bit);
    void renderLCDSBit(DebuggerRegisterTreeNode* node, uint8_t bit);
    void renderInput(DebuggerRegisterTreeNode* node, bool isButton, uint8_t bit);

    void renderCPULoad();
};

#endif
