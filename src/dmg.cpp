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
        [] (ImGuiSizeCallbackData* data) {
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
// PPU
// ===============

void DMG::stepFrame() {
    const int CYCLES_PER_FRAME = 70224;
    int frameCycles = 0;
    while (frameCycles < CYCLES_PER_FRAME) {
        uint64_t before = cycles;
        stepCPU();
        int elapsed = (int)(cycles - before);
        if (elapsed <= 0) elapsed = 4;
        stepPPU(elapsed);
        frameCycles += elapsed;
    }
}

void DMG::stepPPU(int elapsed) {
    uint8_t lcdc = memory[0xFF40];
    if (!(lcdc & 0x80)) {
        memory[0xFF44] = 0;
        ppuCycles = 0;
        return;
    }

    ppuCycles += elapsed;
    uint8_t& ly = memory[0xFF44];

    while (ppuCycles >= 456) {
        ppuCycles -= 456;
        if (ly < 144)
            renderScanline(ly);
        ly++;
        if (ly > 153) ly = 0;
    }
}

void DMG::renderScanline(uint8_t ly) {
    uint8_t lcdc = memory[0xFF40];
    uint8_t bgp = memory[0xFF47];
    uint8_t scx = memory[0xFF43];
    uint8_t scy = memory[0xFF42];

    uint16_t bgMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    bool unsignedAddr = (lcdc & 0x10) != 0;

    uint8_t bgY = (scy + ly) & 0xFF;
    uint8_t tileRow = bgY >> 3;
    uint8_t pixRow = bgY & 7;

    for (int x = 0; x < 160; x++) {
        uint8_t bgX = (scx + x) & 0xFF;
        uint8_t tileCol = bgX >> 3;
        uint8_t pixCol = bgX & 7;

        uint8_t tileIdx = memory[bgMapBase + tileRow * 32 + tileCol];

        uint16_t tileAddr;
        if (unsignedAddr)
            tileAddr = 0x8000 + tileIdx * 16;
        else
            tileAddr = (uint16_t)(0x9000 + (int8_t)tileIdx * 16);

        uint8_t lo = memory[tileAddr + pixRow * 2];
        uint8_t hi = memory[tileAddr + pixRow * 2 + 1];

        uint8_t bit = 7 - pixCol;
        uint8_t colorIdx = (((hi >> bit) & 1) << 1) | ((lo >> bit) & 1);
        uint8_t shade = (bgp >> (colorIdx * 2)) & 0x03;

        static const uint32_t palette[4] = {
            0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000
        };

        gFramebuffer[ly * WIDTH + x] = palette[shade];
    }
}

// ===============
// CPU helpers
// ===============

uint8_t DMG::getR(uint8_t idx) {
    switch (idx & 7) {
        case 0: return CpuRegisters.B;
        case 1: return CpuRegisters.C;
        case 2: return CpuRegisters.D;
        case 3: return CpuRegisters.E;
        case 4: return CpuRegisters.H;
        case 5: return CpuRegisters.L;
        case 6: return memory[CpuRegisters.HL()];
        case 7: return CpuRegisters.A;
        default: return 0;
    }
}

void DMG::setR(uint8_t idx, uint8_t val) {
    switch (idx & 7) {
        case 0: CpuRegisters.B = val; break;
        case 1: CpuRegisters.C = val; break;
        case 2: CpuRegisters.D = val; break;
        case 3: CpuRegisters.E = val; break;
        case 4: CpuRegisters.H = val; break;
        case 5: CpuRegisters.L = val; break;
        case 6: write8(CpuRegisters.HL(), val); break;
        case 7: CpuRegisters.A = val; break;
    }
}

void DMG::push16(uint16_t val) {
    memory[--CpuRegisters.SP] = val >> 8;
    memory[--CpuRegisters.SP] = val & 0xFF;
}

uint16_t DMG::pop16() {
    uint8_t lo = memory[CpuRegisters.SP++];
    uint8_t hi = memory[CpuRegisters.SP++];
    return (hi << 8) | lo;
}

void DMG::addHL(uint16_t val) {
    uint32_t result = CpuRegisters.HL() + val;
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, ((CpuRegisters.HL() ^ val ^ result) & 0x1000) != 0);
    setFlag(FLAG_C, result > 0xFFFF);
    CpuRegisters.setHL(result & 0xFFFF);
}

uint8_t DMG::incR(uint8_t val) {
    uint8_t result = val + 1;
    setFlag(FLAG_Z, result == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, (val & 0x0F) == 0x0F);
    return result;
}

uint8_t DMG::decR(uint8_t val) {
    uint8_t result = val - 1;
    setFlag(FLAG_Z, result == 0);
    setFlag(FLAG_N, true);
    setFlag(FLAG_H, (val & 0x0F) == 0x00);
    return result;
}

void DMG::addA(uint8_t val) {
    uint16_t result = CpuRegisters.A + val;
    setFlag(FLAG_Z, (result & 0xFF) == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, ((CpuRegisters.A ^ val ^ result) & 0x10) != 0);
    setFlag(FLAG_C, result > 0xFF);
    CpuRegisters.A = result & 0xFF;
}

void DMG::adcA(uint8_t val) {
    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
    uint16_t result = CpuRegisters.A + val + carry;
    setFlag(FLAG_Z, (result & 0xFF) == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, ((CpuRegisters.A ^ val ^ result) & 0x10) != 0);
    setFlag(FLAG_C, result > 0xFF);
    CpuRegisters.A = result & 0xFF;
}

void DMG::subA(uint8_t val) {
    uint8_t result = CpuRegisters.A - val;
    setFlag(FLAG_Z, result == 0);
    setFlag(FLAG_N, true);
    setFlag(FLAG_H, (CpuRegisters.A & 0x0F) < (val & 0x0F));
    setFlag(FLAG_C, CpuRegisters.A < val);
    CpuRegisters.A = result;
}

void DMG::sbcA(uint8_t val) {
    uint8_t carry = getFlag(FLAG_C) ? 1 : 0;
    int result = CpuRegisters.A - val - carry;
    setFlag(FLAG_Z, (result & 0xFF) == 0);
    setFlag(FLAG_N, true);
    setFlag(FLAG_H, ((CpuRegisters.A ^ val ^ result) & 0x10) != 0);
    setFlag(FLAG_C, result < 0);
    CpuRegisters.A = result & 0xFF;
}

void DMG::andA(uint8_t val) {
    CpuRegisters.A &= val;
    setFlag(FLAG_Z, CpuRegisters.A == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, true);
    setFlag(FLAG_C, false);
}

void DMG::xorA(uint8_t val) {
    CpuRegisters.A ^= val;
    setFlag(FLAG_Z, CpuRegisters.A == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, false);
    setFlag(FLAG_C, false);
}

void DMG::orA(uint8_t val) {
    CpuRegisters.A |= val;
    setFlag(FLAG_Z, CpuRegisters.A == 0);
    setFlag(FLAG_N, false);
    setFlag(FLAG_H, false);
    setFlag(FLAG_C, false);
}

void DMG::cpA(uint8_t val) {
    setFlag(FLAG_Z, CpuRegisters.A == val);
    setFlag(FLAG_N, true);
    setFlag(FLAG_H, (CpuRegisters.A & 0x0F) < (val & 0x0F));
    setFlag(FLAG_C, CpuRegisters.A < val);
}

// ===============
// DMG
// ===============

std::string DMG::loadROM(const char* path) {
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
    initialize();
    for (std::streamsize i = 0; i < size; i++)
        memory[i] = buffer[i];
    ppuCycles = 0;
    return "";
}

bool DMG::initialize() {
    for (uint32_t i = 0; i < MEMORY_SIZE; i++)
        memory[i] = 0;

    halted = false;
    ime = false;
    cycles = 0;
    ppuCycles = 0;

    CpuRegisters.A = 0x01; CpuRegisters.F = 0xB0;
    CpuRegisters.B = 0x00; CpuRegisters.C = 0x13;
    CpuRegisters.D = 0x00; CpuRegisters.E = 0xD8;
    CpuRegisters.H = 0x01; CpuRegisters.L = 0x4D;
    CpuRegisters.SP = 0xFFFE;
    CpuRegisters.PC = 0x0100;

    memory[0xFF00] = 0xCF; // JOYP
    memory[0xFF01] = 0x00; // SB
    memory[0xFF02] = 0x7E; // SC
    memory[0xFF04] = 0xAB; // DIV
    memory[0xFF05] = 0x00; // TIMA
    memory[0xFF06] = 0x00; // TMA
    memory[0xFF07] = 0xF8; // TAC
    memory[0xFF0F] = 0xE1; // IF
    memory[0xFF10] = 0x80; memory[0xFF11] = 0xBF; memory[0xFF12] = 0xF3;
    memory[0xFF13] = 0xFF; memory[0xFF14] = 0xBF;
    memory[0xFF16] = 0x3F; memory[0xFF17] = 0x00; memory[0xFF18] = 0xFF;
    memory[0xFF19] = 0xBF; memory[0xFF1A] = 0x7F; memory[0xFF1B] = 0xFF;
    memory[0xFF1C] = 0x9F; memory[0xFF1D] = 0xFF; memory[0xFF1E] = 0xBF;
    memory[0xFF20] = 0xFF; memory[0xFF21] = 0x00; memory[0xFF22] = 0x00;
    memory[0xFF23] = 0xBF; memory[0xFF24] = 0x77; memory[0xFF25] = 0xF3;
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

    // test checkerboard tile — remove once real ROMs run correctly
    for (int row = 0; row < 8; row++) {
        uint8_t pat = (row % 2 == 0) ? 0xAA : 0x55;
        memory[0x8000 + row * 2] = pat;
        memory[0x8000 + row * 2 + 1] = pat;
    }

    return true;
}

// ===============
// CPU
// ===============

void DMG::stepCPU() {
    // interrupt dispatch
    if (ime) {
        uint8_t pending = memory[0xFF0F] & memory[0xFFFF] & 0x1F;
        if (pending) {
            halted = false;
            ime = false;
            for (int i = 0; i < 5; i++) {
                if (pending & (1 << i)) {
                    memory[0xFF0F] &= ~(1 << i);
                    push16(CpuRegisters.PC);
                    static const uint16_t vectors[5] = { 0x40, 0x48, 0x50, 0x58, 0x60 };
                    CpuRegisters.PC = vectors[i];
                    cycles += 20;
                    return;
                }
            }
        }
    }

    if (halted) {
        if (memory[0xFF0F] & memory[0xFFFF] & 0x1F)
            halted = false;
        cycles += 4;
        return;
    }

    uint8_t op = memory[CpuRegisters.PC++];

    switch (op) {
        case 0x00: cycles += 4; break; // NOP

            // LD rr, d16
        case 0x01: { uint8_t lo = memory[CpuRegisters.PC++]; uint8_t hi = memory[CpuRegisters.PC++]; CpuRegisters.setBC((hi << 8) | lo); cycles += 12; break; }
        case 0x11: { uint8_t lo = memory[CpuRegisters.PC++]; uint8_t hi = memory[CpuRegisters.PC++]; CpuRegisters.setDE((hi << 8) | lo); cycles += 12; break; }
        case 0x21: { uint8_t lo = memory[CpuRegisters.PC++]; uint8_t hi = memory[CpuRegisters.PC++]; CpuRegisters.setHL((hi << 8) | lo); cycles += 12; break; }
        case 0x31: { uint8_t lo = memory[CpuRegisters.PC++]; uint8_t hi = memory[CpuRegisters.PC++]; CpuRegisters.SP = (hi << 8) | lo; cycles += 12; break; }

                 // LD (rr), A
        case 0x02: write8(CpuRegisters.BC(), CpuRegisters.A); cycles += 8; break;
        case 0x12: write8(CpuRegisters.DE(), CpuRegisters.A); cycles += 8; break;
        case 0x22: write8(CpuRegisters.HL(), CpuRegisters.A); CpuRegisters.setHL(CpuRegisters.HL() + 1); cycles += 8; break;
        case 0x32: write8(CpuRegisters.HL(), CpuRegisters.A); CpuRegisters.setHL(CpuRegisters.HL() - 1); cycles += 8; break;

            // LD A, (rr)
        case 0x0A: CpuRegisters.A = read8(CpuRegisters.BC()); cycles += 8; break;
        case 0x1A: CpuRegisters.A = read8(CpuRegisters.DE()); cycles += 8; break;
        case 0x2A: CpuRegisters.A = read8(CpuRegisters.HL()); CpuRegisters.setHL(CpuRegisters.HL() + 1); cycles += 8; break;
        case 0x3A: CpuRegisters.A = read8(CpuRegisters.HL()); CpuRegisters.setHL(CpuRegisters.HL() - 1); cycles += 8; break;

            // INC rr
        case 0x03: CpuRegisters.setBC(CpuRegisters.BC() + 1); cycles += 8; break;
        case 0x13: CpuRegisters.setDE(CpuRegisters.DE() + 1); cycles += 8; break;
        case 0x23: CpuRegisters.setHL(CpuRegisters.HL() + 1); cycles += 8; break;
        case 0x33: CpuRegisters.SP++; cycles += 8; break;

            // DEC rr
        case 0x0B: CpuRegisters.setBC(CpuRegisters.BC() - 1); cycles += 8; break;
        case 0x1B: CpuRegisters.setDE(CpuRegisters.DE() - 1); cycles += 8; break;
        case 0x2B: CpuRegisters.setHL(CpuRegisters.HL() - 1); cycles += 8; break;
        case 0x3B: CpuRegisters.SP--; cycles += 8; break;

            // INC r
        case 0x04: CpuRegisters.B = incR(CpuRegisters.B); cycles += 4; break;
        case 0x0C: CpuRegisters.C = incR(CpuRegisters.C); cycles += 4; break;
        case 0x14: CpuRegisters.D = incR(CpuRegisters.D); cycles += 4; break;
        case 0x1C: CpuRegisters.E = incR(CpuRegisters.E); cycles += 4; break;
        case 0x24: CpuRegisters.H = incR(CpuRegisters.H); cycles += 4; break;
        case 0x2C: CpuRegisters.L = incR(CpuRegisters.L); cycles += 4; break;
        case 0x34: write8(CpuRegisters.HL(), incR(read8(CpuRegisters.HL()))); cycles += 12; break;
        case 0x3C: CpuRegisters.A = incR(CpuRegisters.A); cycles += 4; break;

            // DEC r
        case 0x05: CpuRegisters.B = decR(CpuRegisters.B); cycles += 4; break;
        case 0x0D: CpuRegisters.C = decR(CpuRegisters.C); cycles += 4; break;
        case 0x15: CpuRegisters.D = decR(CpuRegisters.D); cycles += 4; break;
        case 0x1D: CpuRegisters.E = decR(CpuRegisters.E); cycles += 4; break;
        case 0x25: CpuRegisters.H = decR(CpuRegisters.H); cycles += 4; break;
        case 0x2D: CpuRegisters.L = decR(CpuRegisters.L); cycles += 4; break;
        case 0x35: write8(CpuRegisters.HL(), decR(read8(CpuRegisters.HL()))); cycles += 12; break;
        case 0x3D: CpuRegisters.A = decR(CpuRegisters.A); cycles += 4; break;

            // LD r, d8
        case 0x06: CpuRegisters.B = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x0E: CpuRegisters.C = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x16: CpuRegisters.D = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x1E: CpuRegisters.E = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x26: CpuRegisters.H = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x2E: CpuRegisters.L = memory[CpuRegisters.PC++]; cycles += 8; break;
        case 0x36: write8(CpuRegisters.HL(), memory[CpuRegisters.PC++]); cycles += 12; break;
        case 0x3E: CpuRegisters.A = memory[CpuRegisters.PC++]; cycles += 8; break;

            // RLCA
        case 0x07:
        {
            uint8_t c = CpuRegisters.A >> 7;
            CpuRegisters.A = (CpuRegisters.A << 1) | c;
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, c);
            cycles += 4; break;
        }
        // RRCA
        case 0x0F:
        {
            uint8_t c = CpuRegisters.A & 1;
            CpuRegisters.A = (CpuRegisters.A >> 1) | (c << 7);
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, c);
            cycles += 4; break;
        }
        // RLA
        case 0x17:
        {
            uint8_t oc = getFlag(FLAG_C) ? 1 : 0, nc = CpuRegisters.A >> 7;
            CpuRegisters.A = (CpuRegisters.A << 1) | oc;
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, nc);
            cycles += 4; break;
        }
        // RRA
        case 0x1F:
        {
            uint8_t oc = getFlag(FLAG_C) ? 1 : 0, nc = CpuRegisters.A & 1;
            CpuRegisters.A = (CpuRegisters.A >> 1) | (oc << 7);
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, nc);
            cycles += 4; break;
        }

        // LD (a16), SP
        case 0x08:
        {
            uint8_t lo = memory[CpuRegisters.PC++]; uint8_t hi = memory[CpuRegisters.PC++];
            uint16_t addr = (hi << 8) | lo;
            write8(addr, CpuRegisters.SP & 0xFF); write8(addr + 1, CpuRegisters.SP >> 8);
            cycles += 20; break;
        }

        // ADD HL, rr
        case 0x09: addHL(CpuRegisters.BC()); cycles += 8; break;
        case 0x19: addHL(CpuRegisters.DE()); cycles += 8; break;
        case 0x29: addHL(CpuRegisters.HL()); cycles += 8; break;
        case 0x39: addHL(CpuRegisters.SP);   cycles += 8; break;

            // STOP
        case 0x10: CpuRegisters.PC++; cycles += 4; break;

            // JR r8
        case 0x18: { int8_t o = (int8_t)memory[CpuRegisters.PC++]; CpuRegisters.PC += o; cycles += 12; break; }
        case 0x20: { int8_t o = (int8_t)memory[CpuRegisters.PC++]; if (!getFlag(FLAG_Z)) { CpuRegisters.PC += o; cycles += 12; } else cycles += 8; break; }
        case 0x28: { int8_t o = (int8_t)memory[CpuRegisters.PC++]; if (getFlag(FLAG_Z)) { CpuRegisters.PC += o; cycles += 12; } else cycles += 8; break; }
        case 0x30: { int8_t o = (int8_t)memory[CpuRegisters.PC++]; if (!getFlag(FLAG_C)) { CpuRegisters.PC += o; cycles += 12; } else cycles += 8; break; }
        case 0x38: { int8_t o = (int8_t)memory[CpuRegisters.PC++]; if (getFlag(FLAG_C)) { CpuRegisters.PC += o; cycles += 12; } else cycles += 8; break; }

                 // DAA
        case 0x27:
        {
            uint8_t a = CpuRegisters.A;
            if (!getFlag(FLAG_N)) {
                if (getFlag(FLAG_H) || (a & 0x0F) > 9) a += 0x06;
                if (getFlag(FLAG_C) || a > 0x99) { a += 0x60; setFlag(FLAG_C, true); }
            }
            else {
                if (getFlag(FLAG_H)) a -= 0x06;
                if (getFlag(FLAG_C)) a -= 0x60;
            }
            CpuRegisters.A = a;
            setFlag(FLAG_Z, a == 0); setFlag(FLAG_H, false);
            cycles += 4; break;
        }

        // CPL
        case 0x2F: CpuRegisters.A = ~CpuRegisters.A; setFlag(FLAG_N, true); setFlag(FLAG_H, true); cycles += 4; break;
            // SCF
        case 0x37: setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, true); cycles += 4; break;
            // CCF
        case 0x3F: setFlag(FLAG_N, false); setFlag(FLAG_H, false); setFlag(FLAG_C, !getFlag(FLAG_C)); cycles += 4; break;

            // LD B, r
        case 0x40: CpuRegisters.B = CpuRegisters.B; cycles += 4; break;
        case 0x41: CpuRegisters.B = CpuRegisters.C; cycles += 4; break;
        case 0x42: CpuRegisters.B = CpuRegisters.D; cycles += 4; break;
        case 0x43: CpuRegisters.B = CpuRegisters.E; cycles += 4; break;
        case 0x44: CpuRegisters.B = CpuRegisters.H; cycles += 4; break;
        case 0x45: CpuRegisters.B = CpuRegisters.L; cycles += 4; break;
        case 0x46: CpuRegisters.B = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x47: CpuRegisters.B = CpuRegisters.A; cycles += 4; break;
            // LD C, r
        case 0x48: CpuRegisters.C = CpuRegisters.B; cycles += 4; break;
        case 0x49: CpuRegisters.C = CpuRegisters.C; cycles += 4; break;
        case 0x4A: CpuRegisters.C = CpuRegisters.D; cycles += 4; break;
        case 0x4B: CpuRegisters.C = CpuRegisters.E; cycles += 4; break;
        case 0x4C: CpuRegisters.C = CpuRegisters.H; cycles += 4; break;
        case 0x4D: CpuRegisters.C = CpuRegisters.L; cycles += 4; break;
        case 0x4E: CpuRegisters.C = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x4F: CpuRegisters.C = CpuRegisters.A; cycles += 4; break;
            // LD D, r
        case 0x50: CpuRegisters.D = CpuRegisters.B; cycles += 4; break;
        case 0x51: CpuRegisters.D = CpuRegisters.C; cycles += 4; break;
        case 0x52: CpuRegisters.D = CpuRegisters.D; cycles += 4; break;
        case 0x53: CpuRegisters.D = CpuRegisters.E; cycles += 4; break;
        case 0x54: CpuRegisters.D = CpuRegisters.H; cycles += 4; break;
        case 0x55: CpuRegisters.D = CpuRegisters.L; cycles += 4; break;
        case 0x56: CpuRegisters.D = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x57: CpuRegisters.D = CpuRegisters.A; cycles += 4; break;
            // LD E, r
        case 0x58: CpuRegisters.E = CpuRegisters.B; cycles += 4; break;
        case 0x59: CpuRegisters.E = CpuRegisters.C; cycles += 4; break;
        case 0x5A: CpuRegisters.E = CpuRegisters.D; cycles += 4; break;
        case 0x5B: CpuRegisters.E = CpuRegisters.E; cycles += 4; break;
        case 0x5C: CpuRegisters.E = CpuRegisters.H; cycles += 4; break;
        case 0x5D: CpuRegisters.E = CpuRegisters.L; cycles += 4; break;
        case 0x5E: CpuRegisters.E = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x5F: CpuRegisters.E = CpuRegisters.A; cycles += 4; break;
            // LD H, r
        case 0x60: CpuRegisters.H = CpuRegisters.B; cycles += 4; break;
        case 0x61: CpuRegisters.H = CpuRegisters.C; cycles += 4; break;
        case 0x62: CpuRegisters.H = CpuRegisters.D; cycles += 4; break;
        case 0x63: CpuRegisters.H = CpuRegisters.E; cycles += 4; break;
        case 0x64: CpuRegisters.H = CpuRegisters.H; cycles += 4; break;
        case 0x65: CpuRegisters.H = CpuRegisters.L; cycles += 4; break;
        case 0x66: CpuRegisters.H = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x67: CpuRegisters.H = CpuRegisters.A; cycles += 4; break;
            // LD L, r
        case 0x68: CpuRegisters.L = CpuRegisters.B; cycles += 4; break;
        case 0x69: CpuRegisters.L = CpuRegisters.C; cycles += 4; break;
        case 0x6A: CpuRegisters.L = CpuRegisters.D; cycles += 4; break;
        case 0x6B: CpuRegisters.L = CpuRegisters.E; cycles += 4; break;
        case 0x6C: CpuRegisters.L = CpuRegisters.H; cycles += 4; break;
        case 0x6D: CpuRegisters.L = CpuRegisters.L; cycles += 4; break;
        case 0x6E: CpuRegisters.L = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x6F: CpuRegisters.L = CpuRegisters.A; cycles += 4; break;
            // LD (HL), r
        case 0x70: write8(CpuRegisters.HL(), CpuRegisters.B); cycles += 8; break;
        case 0x71: write8(CpuRegisters.HL(), CpuRegisters.C); cycles += 8; break;
        case 0x72: write8(CpuRegisters.HL(), CpuRegisters.D); cycles += 8; break;
        case 0x73: write8(CpuRegisters.HL(), CpuRegisters.E); cycles += 8; break;
        case 0x74: write8(CpuRegisters.HL(), CpuRegisters.H); cycles += 8; break;
        case 0x75: write8(CpuRegisters.HL(), CpuRegisters.L); cycles += 8; break;
        case 0x76: halted = true; cycles += 4; break; // HALT
        case 0x77: write8(CpuRegisters.HL(), CpuRegisters.A); cycles += 8; break;
            // LD A, r
        case 0x78: CpuRegisters.A = CpuRegisters.B; cycles += 4; break;
        case 0x79: CpuRegisters.A = CpuRegisters.C; cycles += 4; break;
        case 0x7A: CpuRegisters.A = CpuRegisters.D; cycles += 4; break;
        case 0x7B: CpuRegisters.A = CpuRegisters.E; cycles += 4; break;
        case 0x7C: CpuRegisters.A = CpuRegisters.H; cycles += 4; break;
        case 0x7D: CpuRegisters.A = CpuRegisters.L; cycles += 4; break;
        case 0x7E: CpuRegisters.A = read8(CpuRegisters.HL()); cycles += 8; break;
        case 0x7F: CpuRegisters.A = CpuRegisters.A; cycles += 4; break;

            // ADD A, r
        case 0x80: addA(CpuRegisters.B); cycles += 4; break;
        case 0x81: addA(CpuRegisters.C); cycles += 4; break;
        case 0x82: addA(CpuRegisters.D); cycles += 4; break;
        case 0x83: addA(CpuRegisters.E); cycles += 4; break;
        case 0x84: addA(CpuRegisters.H); cycles += 4; break;
        case 0x85: addA(CpuRegisters.L); cycles += 4; break;
        case 0x86: addA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0x87: addA(CpuRegisters.A); cycles += 4; break;
            // ADC A, r
        case 0x88: adcA(CpuRegisters.B); cycles += 4; break;
        case 0x89: adcA(CpuRegisters.C); cycles += 4; break;
        case 0x8A: adcA(CpuRegisters.D); cycles += 4; break;
        case 0x8B: adcA(CpuRegisters.E); cycles += 4; break;
        case 0x8C: adcA(CpuRegisters.H); cycles += 4; break;
        case 0x8D: adcA(CpuRegisters.L); cycles += 4; break;
        case 0x8E: adcA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0x8F: adcA(CpuRegisters.A); cycles += 4; break;
            // SUB r
        case 0x90: subA(CpuRegisters.B); cycles += 4; break;
        case 0x91: subA(CpuRegisters.C); cycles += 4; break;
        case 0x92: subA(CpuRegisters.D); cycles += 4; break;
        case 0x93: subA(CpuRegisters.E); cycles += 4; break;
        case 0x94: subA(CpuRegisters.H); cycles += 4; break;
        case 0x95: subA(CpuRegisters.L); cycles += 4; break;
        case 0x96: subA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0x97: subA(CpuRegisters.A); cycles += 4; break;
            // SBC A, r
        case 0x98: sbcA(CpuRegisters.B); cycles += 4; break;
        case 0x99: sbcA(CpuRegisters.C); cycles += 4; break;
        case 0x9A: sbcA(CpuRegisters.D); cycles += 4; break;
        case 0x9B: sbcA(CpuRegisters.E); cycles += 4; break;
        case 0x9C: sbcA(CpuRegisters.H); cycles += 4; break;
        case 0x9D: sbcA(CpuRegisters.L); cycles += 4; break;
        case 0x9E: sbcA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0x9F: sbcA(CpuRegisters.A); cycles += 4; break;
            // AND r
        case 0xA0: andA(CpuRegisters.B); cycles += 4; break;
        case 0xA1: andA(CpuRegisters.C); cycles += 4; break;
        case 0xA2: andA(CpuRegisters.D); cycles += 4; break;
        case 0xA3: andA(CpuRegisters.E); cycles += 4; break;
        case 0xA4: andA(CpuRegisters.H); cycles += 4; break;
        case 0xA5: andA(CpuRegisters.L); cycles += 4; break;
        case 0xA6: andA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0xA7: andA(CpuRegisters.A); cycles += 4; break;
            // XOR r
        case 0xA8: xorA(CpuRegisters.B); cycles += 4; break;
        case 0xA9: xorA(CpuRegisters.C); cycles += 4; break;
        case 0xAA: xorA(CpuRegisters.D); cycles += 4; break;
        case 0xAB: xorA(CpuRegisters.E); cycles += 4; break;
        case 0xAC: xorA(CpuRegisters.H); cycles += 4; break;
        case 0xAD: xorA(CpuRegisters.L); cycles += 4; break;
        case 0xAE: xorA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0xAF: xorA(CpuRegisters.A); cycles += 4; break;
            // OR r
        case 0xB0: orA(CpuRegisters.B); cycles += 4; break;
        case 0xB1: orA(CpuRegisters.C); cycles += 4; break;
        case 0xB2: orA(CpuRegisters.D); cycles += 4; break;
        case 0xB3: orA(CpuRegisters.E); cycles += 4; break;
        case 0xB4: orA(CpuRegisters.H); cycles += 4; break;
        case 0xB5: orA(CpuRegisters.L); cycles += 4; break;
        case 0xB6: orA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0xB7: orA(CpuRegisters.A); cycles += 4; break;
            // CP r
        case 0xB8: cpA(CpuRegisters.B); cycles += 4; break;
        case 0xB9: cpA(CpuRegisters.C); cycles += 4; break;
        case 0xBA: cpA(CpuRegisters.D); cycles += 4; break;
        case 0xBB: cpA(CpuRegisters.E); cycles += 4; break;
        case 0xBC: cpA(CpuRegisters.H); cycles += 4; break;
        case 0xBD: cpA(CpuRegisters.L); cycles += 4; break;
        case 0xBE: cpA(read8(CpuRegisters.HL())); cycles += 8; break;
        case 0xBF: cpA(CpuRegisters.A); cycles += 4; break;

            // RET cc
        case 0xC0: if (!getFlag(FLAG_Z)) { CpuRegisters.PC = pop16(); cycles += 20; }
                 else cycles += 8; break;
        case 0xC8: if (getFlag(FLAG_Z)) { CpuRegisters.PC = pop16(); cycles += 20; }
                 else cycles += 8; break;
        case 0xD0: if (!getFlag(FLAG_C)) { CpuRegisters.PC = pop16(); cycles += 20; }
                 else cycles += 8; break;
        case 0xD8: if (getFlag(FLAG_C)) { CpuRegisters.PC = pop16(); cycles += 20; }
                 else cycles += 8; break;

            // POP
        case 0xC1: CpuRegisters.setBC(pop16()); cycles += 12; break;
        case 0xD1: CpuRegisters.setDE(pop16()); cycles += 12; break;
        case 0xE1: CpuRegisters.setHL(pop16()); cycles += 12; break;
        case 0xF1: CpuRegisters.setAF(pop16()); cycles += 12; break;

            // JP cc, a16
        case 0xC2: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (!getFlag(FLAG_Z)) { CpuRegisters.PC = a; cycles += 16; } else cycles += 12; break; }
        case 0xCA: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (getFlag(FLAG_Z)) { CpuRegisters.PC = a; cycles += 16; } else cycles += 12; break; }
        case 0xD2: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (!getFlag(FLAG_C)) { CpuRegisters.PC = a; cycles += 16; } else cycles += 12; break; }
        case 0xDA: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (getFlag(FLAG_C)) { CpuRegisters.PC = a; cycles += 16; } else cycles += 12; break; }

                 // JP a16
        case 0xC3: CpuRegisters.PC = read16(CpuRegisters.PC); cycles += 16; break;

            // CALL cc, a16
        case 0xC4: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (!getFlag(FLAG_Z)) { push16(CpuRegisters.PC); CpuRegisters.PC = a; cycles += 24; } else cycles += 12; break; }
        case 0xCC: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (getFlag(FLAG_Z)) { push16(CpuRegisters.PC); CpuRegisters.PC = a; cycles += 24; } else cycles += 12; break; }
        case 0xD4: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (!getFlag(FLAG_C)) { push16(CpuRegisters.PC); CpuRegisters.PC = a; cycles += 24; } else cycles += 12; break; }
        case 0xDC: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; if (getFlag(FLAG_C)) { push16(CpuRegisters.PC); CpuRegisters.PC = a; cycles += 24; } else cycles += 12; break; }

                 // CALL a16
        case 0xCD: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; push16(CpuRegisters.PC); CpuRegisters.PC = a; cycles += 24; break; }

                 // PUSH
        case 0xC5: push16(CpuRegisters.BC()); cycles += 16; break;
        case 0xD5: push16(CpuRegisters.DE()); cycles += 16; break;
        case 0xE5: push16(CpuRegisters.HL()); cycles += 16; break;
        case 0xF5: push16(CpuRegisters.AF()); cycles += 16; break;

            // ALU immediate
        case 0xC6: addA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xCE: adcA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xD6: subA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xDE: sbcA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xE6: andA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xEE: xorA(memory[CpuRegisters.PC++]); cycles += 8; break;
        case 0xF6: orA(memory[CpuRegisters.PC++]);  cycles += 8; break;
        case 0xFE: cpA(memory[CpuRegisters.PC++]);  cycles += 8; break;

            // RST
        case 0xC7: push16(CpuRegisters.PC); CpuRegisters.PC = 0x00; cycles += 16; break;
        case 0xCF: push16(CpuRegisters.PC); CpuRegisters.PC = 0x08; cycles += 16; break;
        case 0xD7: push16(CpuRegisters.PC); CpuRegisters.PC = 0x10; cycles += 16; break;
        case 0xDF: push16(CpuRegisters.PC); CpuRegisters.PC = 0x18; cycles += 16; break;
        case 0xE7: push16(CpuRegisters.PC); CpuRegisters.PC = 0x20; cycles += 16; break;
        case 0xEF: push16(CpuRegisters.PC); CpuRegisters.PC = 0x28; cycles += 16; break;
        case 0xF7: push16(CpuRegisters.PC); CpuRegisters.PC = 0x30; cycles += 16; break;
        case 0xFF: push16(CpuRegisters.PC); CpuRegisters.PC = 0x38; cycles += 16; break;

            // RET
        case 0xC9: CpuRegisters.PC = pop16(); cycles += 16; break;
            // RETI
        case 0xD9: CpuRegisters.PC = pop16(); ime = true; cycles += 16; break;

            // LDH (a8), A  /  LDH A, (a8)
        case 0xE0: write8(0xFF00 | memory[CpuRegisters.PC++], CpuRegisters.A); cycles += 12; break;
        case 0xF0: CpuRegisters.A = read8(0xFF00 | memory[CpuRegisters.PC++]); cycles += 12; break;

            // LD (C), A  /  LD A, (C)
        case 0xE2: write8(0xFF00 | CpuRegisters.C, CpuRegisters.A); cycles += 8; break;
        case 0xF2: CpuRegisters.A = read8(0xFF00 | CpuRegisters.C); cycles += 8; break;

            // ADD SP, r8
        case 0xE8:
        {
            int8_t o = (int8_t)memory[CpuRegisters.PC++];
            uint32_t r = CpuRegisters.SP + o;
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false);
            setFlag(FLAG_H, ((CpuRegisters.SP ^ o ^ r) & 0x10) != 0);
            setFlag(FLAG_C, ((CpuRegisters.SP ^ o ^ r) & 0x100) != 0);
            CpuRegisters.SP = r & 0xFFFF; cycles += 16; break;
        }

        // JP (HL)
        case 0xE9: CpuRegisters.PC = CpuRegisters.HL(); cycles += 4; break;

            // LD (a16), A  /  LD A, (a16)
        case 0xEA: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; write8(a, CpuRegisters.A); cycles += 16; break; }
        case 0xFA: { uint16_t a = read16(CpuRegisters.PC); CpuRegisters.PC += 2; CpuRegisters.A = read8(a); cycles += 16; break; }

                 // DI / EI
        case 0xF3: ime = false; cycles += 4; break;
        case 0xFB: ime = true;  cycles += 4; break;

            // LD HL, SP+r8
        case 0xF8:
        {
            int8_t o = (int8_t)memory[CpuRegisters.PC++];
            uint32_t r = CpuRegisters.SP + o;
            setFlag(FLAG_Z, false); setFlag(FLAG_N, false);
            setFlag(FLAG_H, ((CpuRegisters.SP ^ o ^ r) & 0x10) != 0);
            setFlag(FLAG_C, ((CpuRegisters.SP ^ o ^ r) & 0x100) != 0);
            CpuRegisters.setHL(r & 0xFFFF); cycles += 12; break;
        }

        // LD SP, HL
        case 0xF9: CpuRegisters.SP = CpuRegisters.HL(); cycles += 8; break;

            // CB prefix — all 256 sub-opcodes
        case 0xCB:
        {
            uint8_t cb = memory[CpuRegisters.PC++];
            uint8_t reg = cb & 7;
            uint8_t bit = (cb >> 3) & 7;
            uint8_t val = getR(reg);
            int xc = (reg == 6) ? 8 : 0;

            if (cb < 0x40) {
                uint8_t res = 0;
                switch (cb >> 3) {
                    case 0: { uint8_t c = val >> 7; res = (val << 1) | c; setFlag(FLAG_C, c); break; }           // RLC
                    case 1: { uint8_t c = val & 1;  res = (val >> 1) | (c << 7); setFlag(FLAG_C, c); break; }       // RRC
                    case 2: { uint8_t c = val >> 7; res = (val << 1) | (getFlag(FLAG_C) ? 1 : 0); setFlag(FLAG_C, c); break; } // RL
                    case 3: { uint8_t c = val & 1;  res = (val >> 1) | (getFlag(FLAG_C) ? 0x80 : 0); setFlag(FLAG_C, c); break; } // RR
                    case 4: { setFlag(FLAG_C, val >> 7); res = val << 1; break; }                              // SLA
                    case 5: { setFlag(FLAG_C, val & 1);  res = (val >> 1) | (val & 0x80); break; }                // SRA
                    case 6: { res = ((val & 0x0F) << 4) | (val >> 4); setFlag(FLAG_C, false); break; }            // SWAP
                    case 7: { setFlag(FLAG_C, val & 1);  res = val >> 1; break; }                              // SRL
                }
                setFlag(FLAG_Z, res == 0); setFlag(FLAG_N, false); setFlag(FLAG_H, false);
                setR(reg, res); cycles += 8 + xc;
            }
            else if (cb < 0x80) { // BIT
                setFlag(FLAG_Z, !(val & (1 << bit))); setFlag(FLAG_N, false); setFlag(FLAG_H, true);
                cycles += 8 + xc;
            }
            else if (cb < 0xC0) { // RES
                setR(reg, val & ~(1 << bit)); cycles += 8 + xc;
            }
            else {                 // SET
                setR(reg, val | (1 << bit));  cycles += 8 + xc;
            }
            break;
        }

        // illegal opcodes — treat as NOP
        case 0xD3: case 0xDB: case 0xDD:
        case 0xE3: case 0xE4: case 0xEB: case 0xEC: case 0xED:
        case 0xF4: case 0xFC: case 0xFD:
            cycles += 4; break;

        default: cycles += 4; break;
    }
}
