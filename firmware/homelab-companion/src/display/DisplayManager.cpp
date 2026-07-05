#include "DisplayManager.h"

#include <Wire.h>

#include "config/Config.h"
#include "core/Logger.h"
#include "display/Icons.h"

DisplayManager::DisplayManager()
    : u8g2_(U8G2_R0, /*reset=*/U8X8_PIN_NONE) {}

bool DisplayManager::i2cProbe(uint8_t addr7) {
  Wire.beginTransmission(addr7);
  return Wire.endTransmission() == 0;
}

bool DisplayManager::begin() {
  Wire.begin(config::I2C_SDA_PIN, config::I2C_SCL_PIN);
  Wire.setClock(config::I2C_CLOCK_HZ);

  if (!i2cProbe(config::OLED_I2C_ADDRESS)) {
    LOG_ERROR("OLED not found at 0x%02X (SDA=%u SCL=%u)", config::OLED_I2C_ADDRESS,
              config::I2C_SDA_PIN, config::I2C_SCL_PIN);
    ready_ = false;
    return false;
  }

  u8g2_.setI2CAddress(config::OLED_I2C_ADDRESS << 1); // U8g2 wants the 8-bit form
  u8g2_.begin();
  u8g2_.setFontPosBaseline();
  u8g2_.setContrast(255);
  ready_ = true;
  LOG_INFO("OLED initialised at 0x%02X", config::OLED_I2C_ADDRESS);
  return true;
}

void DisplayManager::clear() {
  u8g2_.clearBuffer();
}

void DisplayManager::send() {
  u8g2_.sendBuffer();
}

// --- Fonts ------------------------------------------------------------------
void DisplayManager::fontSmall() { u8g2_.setFont(u8g2_font_6x10_tf); }
void DisplayManager::fontMedium() { u8g2_.setFont(u8g2_font_helvB10_tf); }
void DisplayManager::fontLarge() { u8g2_.setFont(u8g2_font_helvB18_tf); }

// --- Text -------------------------------------------------------------------
void DisplayManager::text(int x, int y, const char *s) {
  u8g2_.drawStr(ox(x), y, s);
}

int DisplayManager::textWidth(const char *s) {
  return u8g2_.getStrWidth(s);
}

void DisplayManager::textCentered(int cx, int y, const char *s) {
  text(cx - textWidth(s) / 2, y, s);
}

void DisplayManager::textRight(int rx, int y, const char *s) {
  text(rx - textWidth(s), y, s);
}

// --- Widgets ----------------------------------------------------------------
void DisplayManager::progressBar(int x, int y, int w, int h, uint8_t percent) {
  if (percent > 100) percent = 100;
  u8g2_.drawRFrame(ox(x), y, w, h, 2); // rounded outline
  const int inner = w - 4;
  const int fill = (inner * percent) / 100;
  if (fill > 0) {
    u8g2_.drawBox(ox(x) + 2, y + 2, fill, h - 4);
  }
}

void DisplayManager::statusDot(int x, int y, bool ok) {
  // 5px dot: filled disc when ok, hollow ring when down.
  if (ok) {
    u8g2_.drawDisc(ox(x) + 2, y + 2, 2);
  } else {
    u8g2_.drawCircle(ox(x) + 2, y + 2, 2);
  }
}

void DisplayManager::icon(int x, int y, const uint8_t *xbm) {
  u8g2_.drawXBM(ox(x), y, icons::SIZE, icons::SIZE, xbm);
}

void DisplayManager::warningGlyph(int x, int y) {
  // Filled triangle with an exclamation mark punched out (background color).
  const int px = ox(x);
  u8g2_.drawTriangle(px + 4, y, px, y + 8, px + 8, y + 8);
  u8g2_.setDrawColor(0);
  u8g2_.drawVLine(px + 4, y + 3, 3);
  u8g2_.drawPixel(px + 4, y + 7);
  u8g2_.setDrawColor(1);
}

void DisplayManager::hline(int x, int y, int w) {
  u8g2_.drawHLine(ox(x), y, w);
}

// --- Chrome (no offset) -----------------------------------------------------
void DisplayManager::header(const char *title, const AppState &state) {
  const int savedOffset = offsetX_;
  offsetX_ = 0; // header is always fixed

  fontMedium();
  text(2, 11, title);
  // Warning triangle (left of the signal bars) when data is stale or a source
  // is failing. Stale takes precedence conceptually; both render the glyph.
  if (state.stale || state.degraded) {
    warningGlyph(99, 2);
  }
  signalBars(112, 2, state);
  u8g2_.drawHLine(0, HEADER_H - 1, config::SCREEN_WIDTH);

  offsetX_ = savedOffset;
}

void DisplayManager::signalBars(int x, int y, const AppState &state) {
  if (!state.wifiConnected) {
    // Small "x" to signal no link.
    u8g2_.drawLine(x + 2, y + 2, x + 10, y + 9);
    u8g2_.drawLine(x + 10, y + 2, x + 2, y + 9);
    return;
  }

  // Map RSSI (dBm) to 0..4 bars.
  const int r = state.rssi;
  int bars = 0;
  if (r >= -55) bars = 4;
  else if (r >= -65) bars = 3;
  else if (r >= -75) bars = 2;
  else if (r >= -85) bars = 1;

  const int baseline = y + 9;
  for (int i = 0; i < 4; ++i) {
    const int bh = 2 + i * 2;             // 2,4,6,8 px tall
    const int bx = x + i * 4;             // 3px bars + 1px gap
    if (i < bars) {
      u8g2_.drawBox(bx, baseline - bh, 3, bh);
    } else {
      u8g2_.drawFrame(bx, baseline - bh, 3, bh);
    }
  }
}

void DisplayManager::logo(int cx, int cy) {
  // Stylised "server" mark: a rounded box with stacked rack lines + LED.
  const int w = 40, h = 28;
  const int x = cx - w / 2, y = cy - h / 2;
  u8g2_.drawRFrame(x, y, w, h, 4);
  for (int i = 0; i < 3; ++i) {
    const int ry = y + 6 + i * 7;
    u8g2_.drawHLine(x + 6, ry, w - 18);
    u8g2_.drawDisc(x + w - 8, ry, 1); // status LED per "unit"
  }
}
