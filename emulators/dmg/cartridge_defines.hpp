/*

GameBoy (DMG,CGB)

*/

#ifndef VRITA_DMG_CARTRIDGE_DEFINES_INCLUDES
#define VRITA_DMG_CARTRIDGE_DEFINES_INCLUDES

const uint8_t CONST_CGBFlagSupported = 0x80;
const uint8_t CONST_CGBFlagOnly = 0xC0;

struct CartridgeHeader {
    uint8_t cgbFlag;
    uint8_t sgbFlag;
    uint8_t cartridgeType;
    uint8_t romSize;
    uint8_t ramSize;
    uint8_t destinationCode;
    uint8_t version;
    uint8_t oldLicenseeCode;

    std::string title;
    std::string manufacturerCode;
};

enum CartridgeGBType : uint8_t {
    GB_DMG = 0,
    GB_CGB = 1
};

#endif
