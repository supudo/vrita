#include "cpu.hpp"

void DMG_CPU::bit(const char* logMessage, uint8_t bit, uint8_t value) {
    logCall(false, logMessage, " - bit");
    setFlag(FLAG_ZERO, !(value & bit));
    setFlag(FLAG_HALF_CARRY, true);
    setFlag(FLAG_SUBTRACT, false);
}

void DMG_CPU::res(const char* logMessage, uint8_t bit, uint8_t* rgst) {
    logCall(false, logMessage, " - res");
    *rgst &= ~(bit);
}

void DMG_CPU::set(const char* logMessage, uint8_t bit, uint8_t* rgst) {
    logCall(false, logMessage, " - set");
    *rgst |= bit;
}

void DMG_CPU::rotateLeft(const char* logMessage, uint8_t* value, bool throughCarry) {
    logCall(false, logMessage, " - rotateLeft");
    int oldBit7 = (*value >> 7) & 0x01;
    int newBit0 = throughCarry ? (isFlagSet(FLAG_CARRY) ? 1 : 0) : oldBit7;
    setFlag(FLAG_CARRY, oldBit7);
    *value <<= 1;
    *value += newBit0;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::rotateRight(const char* logMessage, uint8_t* value, bool throughCarry) {
    logCall(false, logMessage, " - rotateRight");
    int oldBit0 = *value & 0x01;
    int newBit7 = throughCarry ? (isFlagSet(FLAG_CARRY) ? 1 : 0) : oldBit0;
    setFlag(FLAG_CARRY, oldBit0);
    *value >>= 1;
    *value |= (newBit7 << 7);
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sla(const char* logMessage, uint8_t* value) {
    logCall(false, logMessage, " - sla");
    setFlag(FLAG_CARRY, *value & (1 << 7));
    *value <<= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::sra(const char* logMessage, uint8_t* value) {
    logCall(false, logMessage, " - sra");
    setFlag(FLAG_CARRY, *value & 0x01);
    int msb = *value & (1 << 7);
    *value >>= 1;
    *value |= msb;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::srl(const char* logMessage, uint8_t* value) {
    logCall(false, logMessage, " - srl");
    setFlag(FLAG_CARRY, *value & 0x01);
    *value >>= 1;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY, false);
}

void DMG_CPU::swap(const char* logMessage, uint8_t* value) {
    logCall(false, logMessage, " - swap");
    uint8_t lower = *value << 4;
    *value = (*value >> 4) | lower;
    setFlag(FLAG_ZERO, !*value);
    setFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY | FLAG_CARRY, false);
}