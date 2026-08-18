#ifndef VRITA_DEBUGGER_INCLUDES
#define VRITA_DEBUGGER_INCLUDES

#include <stdint.h>
#include <cstdint>
#include <array>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <imgui.h>

#include "utilities/logger.hpp"
#include "debuggers_defines.hpp"
#include "emulators/dmg/apu_structs.hpp"

#include "third_party/ImGuiColorTextEdit/TextEditor.h"

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
                      std::function<bool(uint8_t)> interruptsEnabled,
                      std::function<bool()> isGameRunning,
                      std::function<void()> stopGame,
                      std::function<void()> startGame,
                      std::function<void(bool)> logCPUCalls,
                      std::function<void()> stepInstruction);
    void setAPUCallbacks(std::function<const PulseChannel&()> channelPulse1,
                         std::function<const PulseChannel&()> channelPulse2,
                         std::function<const WaveChannel&()> channelWave,
                         std::function<const NoiseChannel&()> channelNoise,
                         std::function<uint8_t(uint8_t)> channelOutput);
    void setMemory(const char* emulatorType, uint32_t size);
    void release();
    void render(bool* windowOpened, DMGCpuRegisters& registers);
    void pushCPUFrameTime(float ms);

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
    std::function<bool()> funcIsGameRunning;
    std::function<void()> funcStopGame;
    std::function<void()> funcStartGame;
    std::function<void(bool)> funcLogCPUCalls;
    std::function<void()> funcStepInstruction;

    std::function<const PulseChannel& ()> funcAPUChannel1;
    std::function<const PulseChannel& ()> funcAPUChannel2;
    std::function<const WaveChannel& ()> funcAPUChannelWave;
    std::function<const NoiseChannel& ()> funcAPUChannelNoise;
    std::function<uint8_t(uint8_t)> funcAPUChannelOutput;

    uint32_t memorySize = 0;
    uint8_t emulatorType = 0;

    float memoryPanelHeight = 260.0f;

    bool gameIsRunning = false;
    int selectedMemoryRegion = 0;

    bool logCPUCalls = false;

    TextEditor editorAssembly;
    TextEditor::Language editorLanguage;
    bool editorInitialized = false;
    bool editorSourceSet = false;
    bool breakpointsDisabled = false;
    void initEditor();
    void disassembleWork();

    std::thread disassemblyThread;
    std::atomic<bool> disassemblyDone { false };
    bool disassemblyStarted = false;
    std::string pendingAssemblySource;
    std::array<int32_t, 0x10000> pendingAddressToLine{};

    std::array<int32_t, 0x10000> addressToLine {};
    size_t followedLine = SIZE_MAX;
    void followPC(DMGCpuRegisters& registers);
    void scrollToAddress(uint16_t address);

    void renderPerspective(DMGCpuRegisters& registers);
    void renderAssembly(DMGCpuRegisters& registers, float height);
    void disassemblySource(DMGCpuRegisters& registers);
    void stepIn();
    void stepOver(DMGCpuRegisters& registers);
    void stepBack();
    void stepReturn();
    void advanceFrame();

    bool isThinking = false;
    std::atomic<float> thinkingPercentage = 0.0f;
    void showThinking();
    void hideThinking();

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
    void renderInterruptBit(DebuggerRegisterTreeNode* node, bool isIE, uint8_t bit);
    void renderCartridgeData(DebuggerRegisterTreeNode* node, uint8_t type);
    void renderWavePattern(DebuggerRegisterTreeNode* node);
    void renderAPUChannelData(DebuggerRegisterTreeNode* node, uint8_t channel, uint8_t prop);

    static constexpr int cpuLoadHistorySize = 90;
    float cpuLoadHistory[cpuLoadHistorySize] = {};
    int cpuLoadHistoryOffset = 0;

    void renderCPULoad();
};

#endif
