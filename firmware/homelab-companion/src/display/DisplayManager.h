#pragma once
#include <U8g2lib.h>
#include <cstdint>

#include "models/DashboardData.h"

// =============================================================================
//  DisplayManager — thin, opinionated UI toolkit over U8g2 (SSD1306 128x64,
//  hardware I2C, full frame buffer).
//
//  Pages draw through these helpers rather than touching U8g2 directly, which
//  lets the manager apply a horizontal draw offset for slide transitions.
//  Coordinates passed to helpers are "content" coordinates; the current offset
//  is added automatically.
// =============================================================================

class DisplayManager {
public:
  // Layout constants shared with pages.
  static constexpr int HEADER_H = 14;    // header band height
  static constexpr int CONTENT_Y = HEADER_H + 2; // first usable content row

  DisplayManager();

  // Init I2C + probe the panel. Returns false if the OLED is not found; caller
  // logs details to Serial.
  bool begin();

  // Was the panel found on the bus at begin()?
  bool ready() const { return ready_; }

  // Probe a 7-bit I2C address (used by begin() and the boot I2C scan).
  bool i2cProbe(uint8_t addr7);

  // --- Frame lifecycle ---
  void clear();               // wipe the buffer
  void send();                // flush the buffer to the panel

  // Horizontal offset applied to all subsequent content draws (for slides).
  void setOffsetX(int dx) { offsetX_ = dx; }

  // --- Fonts ---
  void fontSmall();   // compact labels / secondary text
  void fontMedium();  // headings / values
  void fontLarge();   // hero numbers

  // --- Text (baseline y) ---
  void text(int x, int y, const char *s);
  void textCentered(int cx, int y, const char *s);
  void textRight(int rx, int y, const char *s);
  int textWidth(const char *s);

  // --- Widgets (content coords) ---
  void progressBar(int x, int y, int w, int h, uint8_t percent);
  void statusDot(int x, int y, bool ok);          // filled = ok, ring = down
  void icon(int x, int y, const uint8_t *xbm);     // 8x8 icon from Icons.h
  void warningGlyph(int x, int y);                 // 9x9 warning triangle "!"
  void hline(int x, int y, int w);

  // --- Chrome (drawn without offset; always fixed) ---
  void header(const char *title, const AppState &state);
  void logo(int cx, int cy);                       // boot splash mark

private:
  int ox(int x) const { return x + offsetX_; }
  void signalBars(int x, int y, const AppState &state); // top-right of header

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_;
  int offsetX_ = 0;
  bool ready_ = false;
};
