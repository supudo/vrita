/*

GameBoy (DMG)

*/

#ifndef VRITA_DMG_INCLUDES
#define VRITA_DMG_INCLUDES

#include "emulators.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdint.h>
#include <iostream>

class DMG {
public:
    // rendering
    bool createTexture(SDL_GPUDevice* device);
    void generateTestPattern(float time);
    void uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer);
    void run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused);
    void release(SDL_GPUDevice* device);

    // DMG specifics
    bool initialize();
    std::string loadROM(const char* romFilePath);
    void stepCPU();

private:
    // rendering
    static const uint32_t WIDTH = 160;
    static const uint32_t HEIGHT = 144;
    uint32_t gFramebuffer[WIDTH * HEIGHT];
    SDL_GPUTexture* gTexture = nullptr;
    int windowScale = 1;
    int lastWindowScale = -1;

    // DMG specifics
    struct CpuRegisters {
        uint8_t A, F; // registers
        uint8_t B, C; // registers
        uint8_t D, E; // registers
        uint8_t H, L; // registers

        uint16_t SP = 0; // stack pointer
        uint16_t PC = 0; // program counter

        // 16-bit registers combos
        uint16_t AF() const { return (A << 8) | F; }
        void setAF(uint16_t val) { A = val >> 8; F = val & 0xF0; } // F lower nibble is always 0

        uint16_t BC() const { return (B << 8) | C; }
        void setBC(uint16_t val) { B = val >> 8; C = val & 0xFF; }

        uint16_t DE() const { return (D << 8) | E; }
        void setDE(uint16_t val) { D = val >> 8; E = val & 0xFF; }

        uint16_t HL() const { return (H << 8) | L; }
        void setHL(uint16_t val) { H = val >> 8; L = val & 0xFF; }
    } CpuRegisters;

    enum CpuFlags {
        FLAG_Z = 0x80, // zero
        FLAG_N = 0x40, // subtract
        FLAG_H = 0x20, // half carry
        FLAG_C = 0x10 // carry
    };

    inline void setFlag(uint8_t flag, bool enabled) { if (enabled) CpuRegisters.F |= flag; else CpuRegisters.F &= ~flag; CpuRegisters.F &= 0xF0; }
    inline bool getFlag(uint8_t flag) const { return (CpuRegisters.F & flag) != 0; }

    // memory
    static const uint32_t MEMORY_SIZE = 0x10000; // 64 KB
    uint8_t memory[MEMORY_SIZE] = {};

    inline uint8_t read8(uint16_t address) const { return memory[address]; }
    inline void write8(uint16_t address, uint8_t value) { memory[address] = value; }
    inline uint16_t read16(uint16_t address) const { return memory[address] | (memory[address + 1] << 8); }
    inline void write16(uint16_t address, uint16_t value) { memory[address] = value & 0xFF; memory[address + 1] = value >> 8; }

    //cpu
    bool halted = false;
    bool ime = false; // interrupt master enable
    uint64_t cycles = 0;
};

#endif