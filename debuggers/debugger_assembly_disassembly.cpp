#include "debugger.hpp"

#include <chrono>
#include <memory>
#include <queue>
#include <set>
#include <unordered_set>

#include "emulators/dmg/cpu_registers.hpp"

constexpr uint16_t CONST_MinDsRunLength = 4;

void Debugger::disassemblySource(DMGCpuRegisters& registers) {
    if (disassemblyDone.load()) {
        if (disassemblyThread.joinable())
            disassemblyThread.join();

        addressToLine = pendingAddressToLine;
        addressToLineByBank = std::move(pendingAddressToLineByBank);
        lineToAddress = std::move(pendingLineToAddress);
        lineToBytes = std::move(pendingLineToBytes);
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
    std::vector<std::string> localLineToBytes;
    std::unordered_map<uint32_t, int32_t> localAddressToLineByBank;
    uint32_t address = 0x0000;
    int line = 0;
    const int maxInstructions = 0x8000;
    assemblySource.reserve(static_cast<size_t>(maxInstructions) * 40);
    localLineToAddress.reserve(maxInstructions);
    localLineToBytes.reserve(maxInstructions);

    breakpoints.clear();

    std::unordered_set<uint32_t> visited;
    std::unordered_map<uint32_t, DisassembledInstruction> decoded;
    std::unordered_map<uint32_t, LabelKind> labelTargets;
    std::unordered_set<uint32_t> reachedBanks;
    std::unordered_set<uint16_t> ramReferences;
    std::unordered_map<uint16_t, std::set<uint16_t>> romDataReferencesByBank;
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
    auto trackDataReferences = [&] (uint16_t bank, const DisassembledInstruction& instr) {
        for (const auto& operand : instr.operands) {
            if (operand.type != OperandType::Immediate16)
                continue;
            const uint16_t value = operand.value;
            if (value >= 0x8000)
                continue; // outside of ROM
            if (instr.target.has_value() && value == *instr.target)
                continue; // tracked as branch/call target
            if (labelTargets.count(keyOf(bank, value)))
                continue; // code label, not data ref
            romDataReferencesByBank[value < 0x4000 ? 0 : bank].insert(value);
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
        trackDataReferences(bank, instruction);
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
            trackDataReferences(bank, instruction);
            addr = static_cast<uint16_t>(addr + instruction.length);
        }
    }

    auto emitInstructionLine = [&] (uint16_t bank, uint16_t addr, const DisassembledInstruction& instr) {
        assemblySource += "    ";
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
        localLineToBytes.push_back(formatBytes(instr));
        ++line;
    };

    auto emitLabelHeader = [&] (uint16_t bank, uint16_t addr, LabelKind kind) {
        const std::string name = labelName(bank, addr, kind);
        assemblySource += "\n";
        ++line;
        localLineToAddress.push_back(addr);
        localLineToBytes.push_back("");
        assemblySource += "; ---- " + name + " ----\n";
        ++line;
        localLineToAddress.push_back(addr);
        localLineToBytes.push_back("");
        assemblySource += name + ":\n";
        ++line;
        localLineToAddress.push_back(addr);
        localLineToBytes.push_back("");
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
        localLineToBytes.push_back("");
        assemblySource += "; ---- " + std::string(name) + " (" + std::to_string(endAddrExclusive - startAddr) + " bytes) ----\n";
        ++line;
        localLineToAddress.push_back(startAddr);
        localLineToBytes.push_back("");
        assemblySource += std::string(name) + ":\n";
        ++line;
        localLineToAddress.push_back(startAddr);
        localLineToBytes.push_back("");

        for (uint16_t a = startAddr; a < endAddrExclusive; ) {
            const uint8_t fillByte = readROMByte(bank, a);
            uint16_t runEnd = a + 1;
            while (runEnd < endAddrExclusive && readROMByte(bank, runEnd) == fillByte)
                ++runEnd;
            const uint16_t runLen = runEnd - a;

            if (runLen >= CONST_MinDsRunLength) {
                char ds[32];
                snprintf(ds, sizeof(ds), "    ds %u, $%02X", runLen, fillByte);
                assemblySource += std::string(ds) + "\n";
                if (a < 0x4000)
                    (*localAddressToLine)[a] = line;
                else
                    localAddressToLineByBank[keyOf(bank, a)] = line;
                localLineToAddress.push_back(a);
                localLineToBytes.push_back("");
                ++line;
                a = runEnd;
                continue;
            }

            uint16_t chunkLen = 16 - (a & 0xF);
            chunkLen = std::min<uint16_t>(chunkLen, endAddrExclusive - a);
            std::string dbText = "    db ";
            for (uint16_t j = 0; j < chunkLen; ++j) {
                char d[8];
                snprintf(d, sizeof(d), "$%02X", readROMByte(bank, a + j));
                dbText += (j ? ", " : "");
                dbText += d;
            }
            assemblySource += dbText + "\n";
            if (a < 0x4000)
                (*localAddressToLine)[a] = line;
            else
                localAddressToLineByBank[keyOf(bank, a)] = line;
            localLineToAddress.push_back(a);
            localLineToBytes.push_back("");
            ++line;
            a = static_cast<uint16_t>(a + chunkLen);
        }
    };

    auto emitLogoBlock = [&] (uint16_t bank, uint16_t startAddr, uint16_t byteLen) {
        for (uint16_t a = startAddr; a < startAddr + byteLen; a += 8) {
            std::string dbText = "    db ";
            for (uint16_t j = 0; j < 8; ++j) {
                char d[8];
                snprintf(d, sizeof(d), "$%02X", readROMByte(bank, a + j));
                dbText += (j ? ", " : "");
                dbText += d;
            }
            assemblySource += dbText + "\n";
            (*localAddressToLine)[a] = line;
            localLineToAddress.push_back(a);
            localLineToBytes.push_back("");
            ++line;
        }
    };

    auto emitHeaderField = [&] (uint16_t bank, uint16_t addr, uint16_t byteLen, const char* comment) {
        std::string dbText = "    db ";
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
        localLineToBytes.push_back("");
        ++line;
    };

    auto emitCartridgeHeader = [&] (uint16_t bank) {
        assemblySource += "\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        localLineToBytes.push_back("");
        assemblySource += "; ==== GameBoy (DMG) Cartridge Header($0104 - $014F) ====\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        localLineToBytes.push_back("");
        assemblySource += "    ; Nintendo Logo\n";
        ++line;
        localLineToAddress.push_back(0x0104);
        localLineToBytes.push_back("");
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
                uint16_t subStart = addr;
                const uint16_t refBank = (addr < 0x4000) ? 0 : bank;
                auto refIt = romDataReferencesByBank.find(refBank);
                if (refIt != romDataReferencesByBank.end()) {
                    for (uint16_t splitAt : refIt->second) {
                        if (splitAt <= subStart)
                            continue;
                        if (splitAt >= gapEnd)
                            break;
                        emitDataGap(bank, subStart, splitAt);
                        subStart = splitAt;
                    }
                }
                emitDataGap(bank, subStart, gapEnd);
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
    localLineToBytes.insert(localLineToBytes.end(), 6, "");
    assemblySource += "; ==== ROM Bank 00 (fixed, $0000 - $3FFF) ====\n";
    line += 1;
    localLineToAddress.push_back(0);
    localLineToBytes.push_back("");
    emitBankSection(bootBank, 0x0000, 0x0104, false);
    emitCartridgeHeader(bootBank);
    emitBankSection(bootBank, 0x0150, 0x4000, false);

    for (uint16_t bank = 1; bank < totalBanks; ++bank) {
        snprintf(header, sizeof(header), "; ==== ROM Bank %02X ($4000 - $7FFF) ====\n", bank);
        assemblySource += header;
        ++line;
        localLineToAddress.push_back(0x4000);
        localLineToBytes.push_back("");
        const bool fallback = !reachedBanks.count(bank);
        if (fallback) {
            assemblySource += "; ---- unreached by static control flow, linear scan ----\n";
            ++line;
            localLineToAddress.push_back(0x4000);
            localLineToBytes.push_back("");
        }
        emitBankSection(bank, 0x4000, 0x8000, fallback);
        thinkingPercentage.store(50.0f * static_cast<float>(bank) / static_cast<float>(totalBanks));
    }

    pendingAddressToLine = *localAddressToLine;
    pendingAddressToLineByBank = std::move(localAddressToLineByBank);
    pendingLineToAddress = std::move(localLineToAddress);
    pendingLineToBytes = std::move(localLineToBytes);
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
    std::vector<std::string> localLineToBytes;
    uint32_t address = 0x0000;
    int line = 0;
    const int maxInstructions = 0x8000;
    assemblySource.reserve(static_cast<size_t>(maxInstructions) * 40);
    localLineToAddress.reserve(maxInstructions);
    localLineToBytes.reserve(maxInstructions);

    breakpoints.clear();

    for (int i = 0; i < maxInstructions && address <= 0xFFFF; i++) {
        const uint16_t instructionAddress = static_cast<uint16_t>(address);
        const uint8_t opcode = funcMemoryRead(instructionAddress);
        DisassembledInstruction instruction = disassembleInstruction(instructionAddress, opcode, funcMemoryRead);

        (*localAddressToLine)[instructionAddress] = line;

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
        localLineToBytes.push_back(formatBytes(instruction));
        line++;
        address += instruction.length;
        thinkingPercentage.store(100.0f * static_cast<float>(address) / 0x10000);
    }

    pendingAddressToLine = *localAddressToLine;
    pendingLineToAddress = std::move(localLineToAddress);
    pendingLineToBytes = std::move(localLineToBytes);
    pendingAssemblySource = std::move(assemblySource);

    const auto elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime).count();
    logger.log("[Debugger] Linear disassembly took %.2f ms", elapsedMs);

    disassemblyDone.store(true);
}

int32_t Debugger::resolveBankAddressLine(uint16_t bank, uint16_t address) {
    if (address < 0x4000 || address >= 0x8000)
        return addressToLine[address];
    auto it = addressToLineByBank.find((static_cast<uint32_t>(bank) << 16) | address);
    return it != addressToLineByBank.end() ? it->second : -1;
}

void Debugger::scrollToBankAddress(uint16_t bank, uint16_t address) {
    if (!editorSourceSet)
        return;
    const int32_t line = resolveBankAddressLine(bank, address);
    if (line < 0)
        return;
    editorAssembly.SetCursor(TextEditor::DocPos(static_cast<size_t>(line), 0));
    editorAssembly.SelectLine(static_cast<size_t>(line));
    editorAssembly.ScrollToLine(static_cast<size_t>(line), TextEditor::Scroll::alignMiddle);
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