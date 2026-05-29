#include "../include/dmg.hpp"

#include <fstream>
#include <vector>
#include <cstdint>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

// ===============
// rendering
// ===============

void DMG::release(SDL_GPUDevice* device) {
    SDL_ReleaseGPUTexture(device, gTexture);
}

bool DMG::createTexture(SDL_GPUDevice* device) {
    SDL_GPUTextureCreateInfo info = {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = DMG::WIDTH;
    info.height = DMG::HEIGHT;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    gTexture = SDL_CreateGPUTexture(device, &info);
    if (!gTexture) {
        printf("[DMG] Failed to create DMG texture: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

void DMG::generateTestPattern(float time) {
    for (uint32_t y = 0; y < DMG::HEIGHT; y++) {
        for (uint32_t x = 0; x < DMG::WIDTH; x++) {
            uint8_t r = (uint8_t)((x + (int)(time * 50.0f)) & 255);
            uint8_t g = (uint8_t)((y * 2) & 255);
            uint8_t b = (uint8_t)(128);
            gFramebuffer[y * DMG::WIDTH + x] = (255 << 24) | (b << 16) | (g << 8) | (r);
        }
    }
}

void DMG::uploadFramebufferToTexture(SDL_GPUDevice* device, SDL_GPUCommandBuffer* commandBuffer) {
    uint32_t framebufferSize = DMG::WIDTH * DMG::HEIGHT * sizeof(uint32_t);

    SDL_GPUTransferBufferCreateInfo transferInfo = {};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = framebufferSize;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);

    if (!transferBuffer) {
        printf("[DMG] Failed to create transfer buffer\n");
        return;
    }

    void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    memcpy(mapped, gFramebuffer, framebufferSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTextureTransferInfo source = {};
    source.transfer_buffer = transferBuffer;
    source.offset = 0;

    SDL_GPUTextureRegion destination = {};
    destination.texture = gTexture;
    destination.w = DMG::WIDTH;
    destination.h = DMG::HEIGHT;
    destination.d = 1;

    SDL_UploadToGPUTexture(copyPass, &source, &destination, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
}

void DMG::run(bool* windowOpened, const std::function<void(const char*)>& showFileBrowser, const std::function<void(const char*)>& onFocused) {
    float imgW = (float)(DMG::WIDTH * windowScale);
    float imgH = (float)(DMG::HEIGHT * windowScale);

    ImGuiStyle& style = ImGui::GetStyle();
    float decorH = ImGui::GetFrameHeight() + style.WindowPadding.y * 2.0f + ImGui::GetFrameHeight() + style.ItemSpacing.y + 1.0f;
    float padX = style.WindowPadding.x * 2.0f;

    if (windowScale != lastWindowScale && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImGui::SetNextWindowSize(ImVec2(imgW + padX, imgH + decorH), ImGuiCond_Always);
        lastWindowScale = windowScale;
    }

    struct ConstraintData { float aspect; float decorH; float padX; };
    static ConstraintData cd;
    cd = { (float)DMG::WIDTH / (float)DMG::HEIGHT, decorH, padX };

    ImGui::SetNextWindowSizeConstraints(
        ImVec2(padX + DMG::WIDTH, decorH + DMG::HEIGHT),
        ImVec2(FLT_MAX, FLT_MAX),
        [](ImGuiSizeCallbackData* data) {
            auto* c = (ConstraintData*)data->UserData;
            float contentW = data->DesiredSize.x - c->padX;
            data->DesiredSize.y = contentW / c->aspect + c->decorH;
        },
        &cd
    );

    ImGui::Begin("GameBoy (DMG)", windowOpened);
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        onFocused("dmg");

    if (ImGui::Button("Load ROM file", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        showFileBrowser("dmg");
    
    ImGui::SliderInt("Scale", &windowScale, 1, 20);

    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float aspect = (float)DMG::WIDTH / (float)DMG::HEIGHT;
    float dispW = avail.x;
    float dispH = dispW / aspect;
    if (dispH > avail.y) {
        dispH = avail.y;
        dispW = dispH * aspect;
    }
    float offX = (avail.x - dispW) * 0.5f;
    if (offX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);

    ImGui::Image((ImTextureID)gTexture, ImVec2(dispW, dispH));

    ImGui::End();
}

// ===============
// DMG
// ===============

std::string DMG::loadROM(const char* path) {
    int errorStatus = 0;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open ROM: " << path << "\n";
        return "Failed to open ROM";
    }
    std::streamsize size = file.tellg();
    if (size <= 0 || size > 0x8000) {
        std::cerr << "Invalid ROM size: " << size << " bytes\n";
        file.close();
        return "Invalid ROM size";
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read ROM data.\n";
        file.close();
        return "Failed to read ROM data";
    }
    file.close();
    if (errorStatus == 0) {
        for (std::streamsize i = 0; i < size; i++)
            memory[i] = buffer[i];
        return "";
    }
    return "";
}

bool DMG::initialize() {
    for (uint32_t i = 0; i < MEMORY_SIZE; i++) {
        memory[i] = 0;
    }

    halted = false;
    ime = false;
    cycles = 0;

    // initial register values
    CpuRegisters.A = 0x01;
    CpuRegisters.F = 0xB0;

    CpuRegisters.B = 0x00;
    CpuRegisters.C = 0x13;

    CpuRegisters.D = 0x00;
    CpuRegisters.E = 0xD8;

    CpuRegisters.H = 0x01;
    CpuRegisters.L = 0x4D;

    CpuRegisters.SP = 0xFFFE;
    CpuRegisters.PC = 0x0100;

    // hardware registers
    memory[0xFF00] = 0xCF; // JOYP
    memory[0xFF01] = 0x00; // SB
    memory[0xFF02] = 0x7E; // SC

    memory[0xFF04] = 0xAB; // DIV
    memory[0xFF05] = 0x00; // TIMA
    memory[0xFF06] = 0x00; // TMA
    memory[0xFF07] = 0xF8; // TAC

    memory[0xFF0F] = 0xE1; // IF

    memory[0xFF10] = 0x80;
    memory[0xFF11] = 0xBF;
    memory[0xFF12] = 0xF3;
    memory[0xFF13] = 0xFF;
    memory[0xFF14] = 0xBF;

    memory[0xFF16] = 0x3F;
    memory[0xFF17] = 0x00;
    memory[0xFF18] = 0xFF;
    memory[0xFF19] = 0xBF;

    memory[0xFF1A] = 0x7F;
    memory[0xFF1B] = 0xFF;
    memory[0xFF1C] = 0x9F;
    memory[0xFF1D] = 0xFF;
    memory[0xFF1E] = 0xBF;

    memory[0xFF20] = 0xFF;
    memory[0xFF21] = 0x00;
    memory[0xFF22] = 0x00;
    memory[0xFF23] = 0xBF;

    memory[0xFF24] = 0x77;
    memory[0xFF25] = 0xF3;
    memory[0xFF26] = 0xF1;

    memory[0xFF40] = 0x91; // LCDC
    memory[0xFF41] = 0x85; // STAT
    memory[0xFF42] = 0x00; // SCY
    memory[0xFF43] = 0x00; // SCX
    memory[0xFF44] = 0x00; // LY
    memory[0xFF45] = 0x00; // LYC

    memory[0xFF47] = 0xFC; // BGP
    memory[0xFF48] = 0xFF; // OBP0
    memory[0xFF49] = 0xFF; // OBP1

    memory[0xFF4A] = 0x00; // WY
    memory[0xFF4B] = 0x00; // WX

    memory[0xFFFF] = 0x00; // IE

    return true;
}

void DMG::stepCPU() {
    if (halted) {
        cycles += 4;
        return;
    }

    // =========================
    // FETCH
    // =========================
    uint8_t opcode = memory[CpuRegisters.PC++];

    // =========================
    // DECODE + EXECUTE
    // =========================
    switch (opcode) {
        // -------------------------
        // NOP
        // -------------------------
        case 0x00:
            cycles += 4;
            break;

            // -------------------------
            // LD BC, d16
            // -------------------------
        case 0x01:
        {
            uint8_t lo = memory[CpuRegisters.PC++];
            uint8_t hi = memory[CpuRegisters.PC++];
            CpuRegisters.setBC((hi << 8) | lo);
            cycles += 12;
            break;
        }

        // -------------------------
        // LD (BC), A
        // -------------------------
        case 0x02:
            memory[CpuRegisters.BC()] = CpuRegisters.A;
            cycles += 8;
            break;

            // -------------------------
            // INC BC
            // -------------------------
        case 0x03:
            CpuRegisters.setBC(CpuRegisters.BC() + 1);
            cycles += 8;
            break;

            // -------------------------
            // INC B
            // -------------------------
        case 0x04:
            CpuRegisters.B++;

            // flags: Z=1 if result is 0
            setFlag(FLAG_Z, CpuRegisters.B == 0);
            setFlag(FLAG_N, false);
            setFlag(FLAG_H, (CpuRegisters.B & 0x0F) == 0x00);

            cycles += 4;
            break;

            // -------------------------
            // STOP / HALT-like behavior (simplified)
            // -------------------------
        case 0x76:
            halted = true;
            cycles += 4;
            break;

            // =========================
            // CB PREFIX (extended opcodes)
            // =========================
        case 0xCB:
        {
            uint8_t cb = memory[CpuRegisters.PC++];

            switch (cb) {
                case 0x11: // RL C (example)
                {
                    uint8_t carry = getFlag(FLAG_C);
                    uint8_t newCarry = (CpuRegisters.C & 0x80) >> 7;

                    CpuRegisters.C = (CpuRegisters.C << 1) | carry;

                    setFlag(FLAG_Z, CpuRegisters.C == 0);
                    setFlag(FLAG_N, false);
                    setFlag(FLAG_H, false);
                    setFlag(FLAG_C, newCarry);

                    cycles += 8;
                    break;
                }

                default:
                    std::cerr << "Unimplemented CB opcode: "
                        << std::hex << (int)cb << "\n";
                    break;
            }

            break;
        }

        // =========================
        // UNKNOWN OPCODE
        // =========================
        default:
            std::cerr << "Unknown opcode: "
                << std::hex << (int)opcode << "\n";
            break;
    }
}