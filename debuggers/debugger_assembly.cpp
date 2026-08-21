#include "debugger.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_set>

#include "assembly_dmg.inl"
#include "utilities/iconfonts/IconsFontAwesome7.h"
#include "utilities/fonts.hpp"
#include "emulators/dmg/cpu_registers.hpp"

void Debugger::initEditor() {
    editorAssembly.SetLanguage(CreateDMGLanguage());
    editorAssembly.SetReadOnlyEnabled(true);

    editorAssembly.SetLineNumberContextMenuCallback([this] (TextEditor::PopupData& data) {
        const int32_t line = static_cast<int32_t>(data.pos.line);
        const uint32_t addr = (line >= 0 && static_cast<size_t>(line) < lineToAddress.size()) ? lineToAddress[line] : 0;
        if (ImGui::MenuItem("Set Breakpoint"))
            breakpoints[addr] = DebuggerBreakpoint{ addr, line, true, false, editorAssembly.GetLineText(static_cast<size_t>(line)) };
        if (ImGui::MenuItem("Remove Breakpoint"))
            breakpoints.erase(addr);
    });

    editorAssembly.SetLineDecorator(2, [this] (TextEditor::Decorator& decorator) {
        if (gameIsRunning || followedLine == SIZE_MAX || decorator.line != followedLine)
            return;

        const ImVec2 p0 = ImGui::GetCursorScreenPos();
        const float pad = decorator.height * 0.1f;
        const ImVec2 p1(p0.x + pad, p0.y + pad);
        const ImVec2 p2(p0.x + pad, p0.y + decorator.height - pad);
        const ImVec2 p3(p0.x + decorator.width - pad, p0.y + decorator.height * 0.5f);
        ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, IM_COL32(220, 30, 30, 255));
    });
}

void Debugger::disassemblySource(DMGCpuRegisters& registers) {
    if (disassemblyDone.load()) {
        if (disassemblyThread.joinable())
            disassemblyThread.join();

        addressToLine = pendingAddressToLine;
        addressToLineByBank = std::move(pendingAddressToLineByBank);
        lineToAddress = std::move(pendingLineToAddress);
        editorAssembly.SetText(pendingAssemblySource);
        pendingAssemblySource.clear();
        pendingAssemblySource.shrink_to_fit();

        disassemblyDone.store(false);
        editorSourceSet = true;
        hideThinking();

        scrollToAddress(registers.PC);
        return;
    }

    if (!disassemblyRequested || editorSourceSet || disassemblyStarted)
        return;

    if (!funcMemoryRead)
        return; // try next frame?

    disassemblyStarted = true;
    showThinking();
    disassemblyThread = std::thread(&Debugger::disassembleWorkDiscovery, this);
}

int32_t Debugger::resolveAddressLine(uint16_t address) {
    if (address < 0x4000 || address >= 0x8000)
        return addressToLine[address];
    if (!funcCurrentRomBank)
        return -1;
    auto it = addressToLineByBank.find((static_cast<uint32_t>(funcCurrentRomBank()) << 16) | address);
    return it != addressToLineByBank.end() ? it->second : -1;
}

uint8_t Debugger::readROMByte(uint16_t bank, uint16_t addr) const {
    size_t offset = (addr < 0x4000) ? addr : (static_cast<size_t>(bank) * 0x4000 + (addr - 0x4000));
    return (romBuffer && offset < romBufferSize) ? romBuffer[offset] : 0xFF;
}

void Debugger::disassembleWorkDiscovery() {
    const auto startTime = std::chrono::steady_clock::now();

    auto localAddressToLine = std::make_unique<std::array<int32_t, 0x10000>>();
    localAddressToLine->fill(-1);

    std::string assemblySource;
    std::vector<uint16_t> localLineToAddress;
    std::unordered_map<uint32_t, int32_t> localAddressToLineByBank;
    uint32_t address = 0x0000;
    int line = 0;
    const int maxInstructions = 0x8000;
    assemblySource.reserve(static_cast<size_t>(maxInstructions) * 40);
    localLineToAddress.reserve(maxInstructions);

    breakpoints.clear();

    std::unordered_set<uint32_t> visited;
    std::unordered_map<uint32_t, DisassembledInstruction> decoded;
    std::unordered_map<uint32_t, LabelKind> labelTargets;
    std::unordered_set<uint32_t> reachedBanks;
    std::unordered_set<uint16_t> ramReferences;
    std::queue<WorkItem> worklist;

    auto labelRank = [] (LabelKind k) { return k == LabelKind::EntryPoint ? 2 : k == LabelKind::Function ? 1 : 0; };
    auto trackRamReferences = [&] (const DisassembledInstruction& instr) {
        for (const auto& operand : instr.operands) {
            if (operand.type != OperandType::Address16)
                continue;
            const bool isWRAM = operand.value >= 0xC000 && operand.value <= 0xDFFF;
            const bool isHRAM = operand.value >= 0xFF80 && operand.value <= 0xFFFE;
            if ((isWRAM || isHRAM) && getHardwareRegisterName(operand.value).empty())
                ramReferences.insert(operand.value);
        }
    };

    const uint16_t bootBank = funcCurrentRomBank ? funcCurrentRomBank() : 1;
    const uint16_t totalBanks = funcTotalRomBanks ? funcTotalRomBanks() : 2;

    static constexpr uint16_t seeds[] = {
        0x0100, // starting point
        0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038, // RST vectors
        0x0040, 0x0048, 0x0050, 0x0058, 0x0060 // VBlank/STAT/Timer/Serial/Joypad ISR entries
    };
    for (uint16_t seed : seeds) {
        worklist.push({ bootBank, seed });
        labelTargets[keyOf(bootBank, seed)] = LabelKind::EntryPoint;
    }

    while (!worklist.empty()) {
        WorkItem item = worklist.front();
        worklist.pop();

        if (item.address >= 0x8000)
            continue; // non-ROM

        const uint32_t key = keyOf(item.bank, item.address);
        if (visited.count(key))
            continue;
        visited.insert(key);

        const uint8_t opcode = readROMByte(item.bank, item.address);
        const uint16_t bank = item.bank;
        auto reader = [this, bank] (uint32_t a) { return readROMByte(bank, static_cast<uint16_t>(a)); };
        DisassembledInstruction instruction = disassembleInstruction(item.address, opcode, reader);
        decoded[key] = instruction;
        trackRamReferences(instruction);
        if (item.address >= 0x4000)
            reachedBanks.insert(item.bank);

        if (instruction.target.has_value() && *instruction.target < 0x8000) {
            const LabelKind kind = (instruction.flags & InstructionFlags::Call) ? LabelKind::Function : LabelKind::Branch;
            const uint32_t targetKey = keyOf(item.bank, *instruction.target);
            auto existing = labelTargets.find(targetKey);
            labelTargets[targetKey] = (existing != labelTargets.end() && labelRank(existing->second) > labelRank(kind)) ? existing->second : kind;
            worklist.push({ item.bank, *instruction.target });
        }

        const bool unconditionalReturn = (instruction.flags & InstructionFlags::Return) && !(instruction.flags & InstructionFlags::Conditional);
        const bool unconditionalBranch = (instruction.flags & InstructionFlags::Branch) && !(instruction.flags & InstructionFlags::Conditional);
        if (!unconditionalReturn && !unconditionalBranch) {
            const uint32_t next = static_cast<uint32_t>(item.address) + instruction.length;
            if (next < 0x8000)
                worklist.push({ item.bank, static_cast<uint16_t>(next) });
        }

        thinkingPercentage.store(50.0f * static_cast<float>(visited.size()) / static_cast<float>(0x4000 * totalBanks));
    }

    for (uint16_t bank = 1; bank < totalBanks; ++bank) {
        if (reachedBanks.count(bank))
            continue;
        uint16_t addr = 0x4000;
        while (addr < 0x8000) {
            const uint8_t opcode = readROMByte(bank, addr);
            auto reader = [this, bank] (uint32_t a) { return readROMByte(bank, static_cast<uint16_t>(a)); };
            DisassembledInstruction instruction = disassembleInstruction(addr, opcode, reader);
            decoded[keyOf(bank, addr)] = instruction;
            trackRamReferences(instruction);
            addr = static_cast<uint16_t>(addr + instruction.length);
        }
    }

    auto emitInstructionLine = [&] (uint16_t bank, uint16_t addr, const DisassembledInstruction& instr) {
        char prefix[16];
        snprintf(prefix, sizeof(prefix), "$%04X     ", addr);
        assemblySource += prefix;
        assemblySource += formatBytes(instr);
        for (uint8_t j = instr.length; j < 3; ++j)
            assemblySource += "    ";
        assemblySource += "  ";
        assemblySource += instructionToString(instr.mnemonic);

        bool first = true;
        for (const auto& operand : instr.operands) {
            if (operand.type == OperandType::None)
                continue;
            assemblySource += first ? " " : ", ";
            assemblySource += formatOperandWithLabels(operand, instr, bank, labelTargets, ramReferences);
            first = false;
        }
        assemblySource += "\n";

        if (addr < 0x4000)
            (*localAddressToLine)[addr] = line;
        else
            localAddressToLineByBank[keyOf(bank, addr)] = line;
        localLineToAddress.push_back(addr);
        ++line;
    };

    auto emitLabelHeader = [&] (uint16_t bank, uint16_t addr, LabelKind kind) {
        const std::string name = labelName(bank, addr, kind);
        assemblySource += "\n";
        ++line;
        localLineToAddress.push_back(addr);
        assemblySource += "; ---- " + name + " ----\n";
        ++line;
        localLineToAddress.push_back(addr);
        assemblySource += name + ":\n";
        ++line;
        localLineToAddress.push_back(addr);
    };

    auto emitDataGap = [&] (uint16_t bank, uint16_t startAddr, uint16_t endAddrExclusive) {
        char name[24];
        if (startAddr < 0x4000)
            snprintf(name, sizeof(name), "Data_%04X", startAddr);
        else
            snprintf(name, sizeof(name), "Data_%02X_%04X", bank, startAddr);
        assemblySource += "\n";
        ++line;
        localLineToAddress.push_back(startAddr);
        assemblySource += "; ---- " + std::string(name) + " (" + std::to_string(endAddrExclusive - startAddr) + " bytes) ----\n";
        ++line;
        localLineToAddress.push_back(startAddr);
        assemblySource += std::string(name) + ":\n";
        ++line;
        localLineToAddress.push_back(startAddr);

        for (uint16_t a = startAddr; a < endAddrExclusive; ) {
            const uint16_t chunkLen = std::min<uint16_t>(3, endAddrExclusive - a);
            char prefix[16];
            snprintf(prefix, sizeof(prefix), "$%04X     ", a);
            assemblySource += prefix;
            std::string bytesText, dbText = "db ";
            for (uint16_t j = 0; j < chunkLen; ++j) {
                char b[8];
                snprintf(b, sizeof(b), "%02X ", readROMByte(bank, a + j));
                if (j)
                    bytesText += ' ';
                bytesText += b;
                char d[8];
                snprintf(d, sizeof(d), "$%02X", readROMByte(bank, a + j));
                dbText += (j ? ", " : "");
                dbText += d;
            }
            assemblySource += bytesText;
            for (uint16_t j = chunkLen; j < 3; ++j)
                assemblySource += "    ";
            assemblySource += "  " + dbText + "\n";
            if (a < 0x4000)
                (*localAddressToLine)[a] = line;
            else
                localAddressToLineByBank[keyOf(bank, a)] = line;
            localLineToAddress.push_back(a);
            ++line;
            a = static_cast<uint16_t>(a + chunkLen);
        }
    };

    auto emitLogoBlock = [&] (uint16_t bank, uint16_t startAddr, uint16_t byteLen) {
        for (uint16_t a = startAddr; a < startAddr + byteLen; a += 8) {
            char prefix[16];
            snprintf(prefix, sizeof(prefix), "$%04X     ", a);
            assemblySource += prefix;
            std::string dbText = "db ";
            for (uint16_t j = 0; j < 8; ++j) {
                char d[8];
                snprintf(d, sizeof(d), "$%02X", readROMByte(bank, a + j));
                dbText += (j ? ", " : "");
                dbText += d;
            }
            assemblySource += dbText + "\n";
            (*localAddressToLine)[a] = line;
            localLineToAddress.push_back(a);
            ++line;
        }
    };

    auto emitHeaderField = [&] (uint16_t bank, uint16_t addr, uint16_t byteLen, const char* comment) {
        char prefix[16];
        snprintf(prefix, sizeof(prefix), "$%04X     ", addr);
        assemblySource += prefix;
        std::string dbText = "db ";
        for (uint16_t j = 0; j < byteLen; ++j) {
            char d[8];
            snprintf(d, sizeof(d), "$%02X", readROMByte(bank, addr + j));
            dbText += (j ? ", " : "");
            dbText += d;
        }
        assemblySource += dbText;
        assemblySource += std::string("  ; ") + comment + "\n";
        (*localAddressToLine)[addr] = line;
        localLineToAddress.push_back(addr);
        ++line;
    };

    auto emitCartridgeHeader = [&] (uint16_t bank) {
        assemblySource += "\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        assemblySource += "; ==== GameBoy (DMG) Cartridge Header($0104 - $014F) ====\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        assemblySource += "          ; Nintendo Logo\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        emitLogoBlock(bank, 0x0104, 48);
        for (const auto& field : cartridgeHeaderFields)
            emitHeaderField(bank, field.address, field.length, field.comment);
    };

    auto emitBankSection = [&] (uint16_t bank, uint16_t rangeStart, uint16_t rangeEnd, bool isFallback) {
        uint16_t addr = rangeStart;
        while (addr < rangeEnd) {
            auto it = decoded.find(keyOf(bank, addr));
            if (it == decoded.end()) {
                uint16_t gapEnd = addr;
                while (gapEnd < rangeEnd && decoded.find(keyOf(bank, gapEnd)) == decoded.end())
                    ++gapEnd;
                emitDataGap(bank, addr, gapEnd);
                addr = gapEnd;
                continue;
            }
            if (!isFallback) {
                auto lbl = labelTargets.find(keyOf(bank, addr));
                if (lbl != labelTargets.end())
                    emitLabelHeader(bank, addr, lbl->second);
            }
            emitInstructionLine(bank, addr, it->second);
            addr = static_cast<uint16_t>(addr + it->second.length);
        }
    };

    char romTitle[17];
    for (int i = 0; i < 16; i++)
        romTitle[i] = static_cast<char>(funcMemoryRead(0x0134 + i));
    romTitle[16] = '\0';

    char header[64];
    assemblySource += "; Disassebmly of " + std::string(romTitle) + "\n";
    assemblySource += "; This disassembly was created by Vrita (https://github.com/supudo/vrita)\n";
    assemblySource += "; In no way, shape or form this is 100% correct, bugs do exist\n";
    assemblySource += "; Still work in progress ...\n";
    assemblySource += "\n";
    assemblySource += "\n";
    line += 6;
    localLineToAddress.insert(localLineToAddress.end(), 6, 0);
    assemblySource += "; ==== ROM Bank 00 (fixed, $0000 - $3FFF) ====\n";
    line += 1;
    localLineToAddress.push_back(0);
    emitBankSection(bootBank, 0x0000, 0x0104, false);
    emitCartridgeHeader(bootBank);
    emitBankSection(bootBank, 0x0150, 0x4000, false);

    for (uint16_t bank = 1; bank < totalBanks; ++bank) {
        snprintf(header, sizeof(header), "; ==== ROM Bank %02X ($4000 - $7FFF) ====\n", bank);
        assemblySource += header;
        ++line;
        localLineToAddress.push_back(0x4000);
        const bool fallback = !reachedBanks.count(bank);
        if (fallback) {
            assemblySource += "; ---- unreached by static control flow, linear scan ----\n";
            ++line;
            localLineToAddress.push_back(0x4000);
        }
        emitBankSection(bank, 0x4000, 0x8000, fallback);
        thinkingPercentage.store(50.0f * static_cast<float>(bank) / static_cast<float>(totalBanks));
    }

    pendingAddressToLine = *localAddressToLine;
    pendingAddressToLineByBank = std::move(localAddressToLineByBank);
    pendingLineToAddress = std::move(localLineToAddress);
    pendingAssemblySource = std::move(assemblySource);

    const auto elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    logger.log("[Debugger] Discovery disassembly took %.2f ms", elapsedMs);

    disassemblyDone.store(true);
}

void Debugger::disassembleWork() {
    const auto startTime = std::chrono::steady_clock::now();

    auto localAddressToLine = std::make_unique<std::array<int32_t, 0x10000>>();
    localAddressToLine->fill(-1);

    std::string assemblySource;
    std::vector<uint16_t> localLineToAddress;
    uint32_t address = 0x0000;
    int line = 0;
    const int maxInstructions = 0x8000;
    assemblySource.reserve(static_cast<size_t>(maxInstructions) * 40);
    localLineToAddress.reserve(maxInstructions);

    breakpoints.clear();

    for (int i = 0; i < maxInstructions && address <= 0xFFFF; i++) {
        const uint16_t instructionAddress = static_cast<uint16_t>(address);
        const uint8_t opcode = funcMemoryRead(instructionAddress);
        DisassembledInstruction instruction = disassembleInstruction(instructionAddress, opcode, funcMemoryRead);

        (*localAddressToLine)[instructionAddress] = line;

        char prefix[16];
        snprintf(prefix, sizeof(prefix), "$%04X     ", instructionAddress);
        assemblySource += prefix;

        assemblySource += formatBytes(instruction);

        for (uint8_t j = instruction.length; j < 3; ++j)
            assemblySource += "    ";
        assemblySource += "  ";

        assemblySource += instructionToString(instruction.mnemonic);

        bool first = true;

        for (const auto& operand : instruction.operands) {
            if (operand.type == OperandType::None)
                continue;
            assemblySource += first ? " " : ", ";
            assemblySource += instructionFormatOperand(operand);

            first = false;
        }

        assemblySource += "\n";

        localLineToAddress.push_back(instructionAddress);
        line++;
        address += instruction.length;
        thinkingPercentage.store(100.0f * static_cast<float>(address) / 0x10000);
    }

    pendingAddressToLine = *localAddressToLine;
    pendingLineToAddress = std::move(localLineToAddress);
    pendingAssemblySource = std::move(assemblySource);

    const auto elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    logger.log("[Debugger] Linear disassembly took %.2f ms", elapsedMs);

    disassemblyDone.store(true);
}

void Debugger::scrollToAddress(uint16_t address) {
    if (!editorSourceSet)
        return;

    const int32_t line = resolveAddressLine(address);
    if (line < 0)
        return;

    followedLine = static_cast<size_t>(line);
    editorAssembly.SetCursor(TextEditor::DocPos(followedLine, 0));
    editorAssembly.SelectLine(followedLine);
    editorAssembly.ScrollToLine(followedLine, TextEditor::Scroll::alignMiddle);
}

void Debugger::followPC(DMGCpuRegisters& registers) {
    if (!breakpointsDisabled) {
        for (auto& [addr, bp] : breakpoints)
            bp.isHit = (bp.enabled && addr == registers.PC);
    }

    if (!gameIsRunning)
        return;

    if (!breakpointsDisabled) {
        auto it = breakpoints.find(registers.PC);
        if (it != breakpoints.end() && it->second.enabled) {
            gameIsRunning = false;
            funcStopGame();
        }
    }

    const int32_t line = resolveAddressLine(registers.PC);
    if (line < 0 || static_cast<size_t>(line) == followedLine)
        return;
    scrollToAddress(registers.PC);
}

void Debugger::stepIn() {
}

void Debugger::stepOver(DMGCpuRegisters& registers) {
    if (gameIsRunning || !funcStepInstruction)
        return;
    funcStepInstruction();
    scrollToAddress(registers.PC);
}

void Debugger::stepBack() {
}

void Debugger::stepReturn() {
}

void Debugger::advanceFrame() {
}

void Debugger::renderAssembly(DMGCpuRegisters& registers, float height) {
    disassemblySource(registers);
    followPC(registers);

    if (!gameIsRunning && funcIsGameRunning())
        funcStopGame();
    else if (gameIsRunning && !funcIsGameRunning())
        funcStartGame();

    ImGui::BeginChild("childAssembly", ImVec2(0, height), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushFont(VritaFontSmall);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.5f, 1.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.2f, 0.2f, 1.0));
    bool wasGameRunning = gameIsRunning;
    if (wasGameRunning) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_PLAY)) {
        gameIsRunning = true;
        funcStartGame();
    }
    if (wasGameRunning) ImGui::EndDisabled();
    ImGui::SetItemTooltip("Run");
    ImGui::SameLine();
    if (!wasGameRunning) ImGui::BeginDisabled();
    if (ImGui::Button(ICON_FA_PAUSE)) {
        gameIsRunning = false;
        funcStopGame();
    }
    if (!wasGameRunning) ImGui::EndDisabled();
    ImGui::SetItemTooltip("Pause");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_DOWN))
        stepIn();
    ImGui::SetItemTooltip("Step In");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_DOWN))
        stepOver(registers);
    ImGui::SetItemTooltip("Step Over");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_UP))
        stepBack();
    ImGui::SetItemTooltip("Step Back");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_TURN_UP))
        stepReturn();
    ImGui::SetItemTooltip("Step Return");
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_UP_RIGHT_FROM_SQUARE))
        advanceFrame();
    ImGui::SetItemTooltip("Advance Frame");

    // separator
    ImGui::SameLine();
    {
        float spacing = 8.0f;
        float lineHeight = ImGui::GetFrameHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = p.x + spacing * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(lineX, p.y), ImVec2(lineX, p.y + lineHeight), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
        ImGui::Dummy(ImVec2(spacing, lineHeight));
    }

    ImGui::SameLine();
    if (ImGui::Button(breakpointsDisabled ? ICON_FA_TOGGLE_ON : ICON_FA_TOGGLE_OFF)) {
        breakpointsDisabled = !breakpointsDisabled;
    }
    ImGui::SetItemTooltip(breakpointsDisabled ? "Disable Breakpoints" : "Enable Breakpoints");

    // separator
    ImGui::SameLine();
    {
        float spacing = 8.0f;
        float lineHeight = ImGui::GetFrameHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float lineX = p.x + spacing * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(lineX, p.y), ImVec2(lineX, p.y + lineHeight), ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
        ImGui::Dummy(ImVec2(spacing, lineHeight));
    }

    ImGui::SameLine();
    if (ImGui::Button(logCPUCalls ? "Log CPU Calls OFF" : "Log CPU Calls ON")) {
        logCPUCalls = !logCPUCalls;
        funcLogCPUCalls(logCPUCalls);
    }
    ImGui::SetItemTooltip("Log CPU calls");
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::Separator();

    editorAssembly.ClearMarkers();
    const size_t markerLine = (gameIsRunning && followedLine != SIZE_MAX) ? followedLine : editorAssembly.GetCurrentCursorPosition().line;
    editorAssembly.AddMarker(markerLine, IM_COL32(55, 55, 60, 255), IM_COL32(55, 55, 60, 255), "", "");

    for (const auto& [addr, bp] : breakpoints) {
        const int32_t bpLine = resolveAddressLine(static_cast<uint16_t>(addr));
        if (bpLine >= 0)
            editorAssembly.AddMarker(static_cast<size_t>(bpLine), breakpointsDisabled ? IM_COL32(255, 0, 0, 100) : IM_COL32(255, 0, 0, 255), 0, "", "Breakpoint");
    }

    if (editorSourceSet) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
        ImGui::SameLine(54);
        ImGui::Text("Address");
        ImGui::SameLine(128);
        ImGui::Text("Bytes");
        ImGui::SameLine(216);
        ImGui::Text("Code");
        ImGui::PopStyleColor();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));

        editorAssembly.Render("Assembly");

        ImGui::PopStyleVar();
    }
    else if (ImGui::Button("Disassemble ROM"))
        disassemblyRequested = true;

    ImGui::EndChild();
}

void Debugger::renderRestBreakpoints() {
    float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    ImGuiTableFlags table_flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_HighlightHoveredColumn;
    std::optional<int32_t> breakpointToRemove;

    if (ImGui::BeginTable("tableBreakpoints", 5, table_flags)) {
        ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Disable", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Hit", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 8.0f);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_NoResize);
        ImGui::TableHeadersRow();

        for (auto& [addr, bp] : breakpoints) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            char lineLabel[16];
            snprintf(lineLabel, sizeof(lineLabel), "%d", bp.line);
            if (ImGui::Selectable(lineLabel, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, ImGui::GetFrameHeight())))
                scrollToAddress(static_cast<uint16_t>(bp.address));

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::Selectable("Remove breakpoint"))
                    breakpointToRemove = addr;
                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            {
                const float checkboxWidth = ImGui::GetFrameHeight();
                const float cellWidth = ImGui::GetContentRegionAvail().x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - checkboxWidth) * 0.5f);
                ImGui::Checkbox("##bpEnabled", &bp.enabled);
            }

            ImGui::TableNextColumn();
            {
                const float checkboxWidth = ImGui::GetFrameHeight();
                const float cellWidth = ImGui::GetContentRegionAvail().x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - checkboxWidth) * 0.5f);
                ImGui::BeginDisabled();
                ImGui::Checkbox("##bpHit", &bp.isHit);
                ImGui::EndDisabled();
            }

            ImGui::TableNextColumn();
            ImGui::Text("$%04X", bp.address);

            ImGui::TableNextColumn();
            ImGui::Text("%s", bp.description.c_str());
        }

        ImGui::EndTable();
    }

    if (breakpointToRemove.has_value())
        breakpoints.erase(*breakpointToRemove);
}
