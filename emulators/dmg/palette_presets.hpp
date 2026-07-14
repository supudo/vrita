/*

GameBoy (DMG) - shared palette color presets.
Single source of truth for both the PPU renderer and the Palette Viewer debugger,
so they can never drift apart. Shade index 0 = lightest .. 3 = darkest. Packed 0xAARRGGBB.

*/

#ifndef VRITA_DMG_PALETTE_PRESETS_INCLUDES
#define VRITA_DMG_PALETTE_PRESETS_INCLUDES

#include <stdint.h>
#include <array>

// 0xAARRGGBB -> SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM
inline constexpr std::array<uint32_t, 4> DMG_PALETTE_DEFAULT = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F };
inline constexpr std::array<uint32_t, 4> DMG_PALETTE_DMG = { 0xFFE0F7D1, 0xFF8CBF6B, 0xFF386B2E, 0xFF0F260F };
inline constexpr std::array<uint32_t, 4> DMG_PALETTE_CGB = { 0xFF9CBD0F, 0xFF8CAB0F, 0xFF306130, 0xFF0F380F };
inline constexpr std::array<uint32_t, 4> DMG_PALETTE_MGB = { 0xFFC4CFA1, 0xFF8C946E, 0xFF4D543D, 0xFF1F1F1F };
inline constexpr std::array<uint32_t, 4> DMG_PALETTE_MGL = { 0xFF1CDECF, 0xFF1AC7B3, 0xFF17A696, 0xFF0A7A6E };

inline constexpr uint32_t DMG_PackForFramebuffer(uint32_t argb) {
    return (argb & 0xFF00FF00u) | ((argb & 0x00FF0000u) >> 16) | ((argb & 0x000000FFu) << 16);
}

#endif
