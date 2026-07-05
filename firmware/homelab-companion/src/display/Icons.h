#pragma once
#include <cstdint>

// =============================================================================
//  Small monochrome XBM icons (bit order LSB-first, as U8g2::drawXBM expects).
//  8x8 glyphs used to decorate pages. Signal bars and status dots are drawn
//  procedurally in DisplayManager instead.
// =============================================================================

namespace icons {

// 8x8 downward arrow (download).
inline const uint8_t ARROW_DOWN[] = {
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x18,
};

// 8x8 upward arrow (upload).
inline const uint8_t ARROW_UP[] = {
    0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18,
};

// 8x8 shield outline (Pi-hole).
inline const uint8_t SHIELD[] = {
    0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x18, 0x00,
};

inline constexpr uint8_t SIZE = 8;

} // namespace icons
