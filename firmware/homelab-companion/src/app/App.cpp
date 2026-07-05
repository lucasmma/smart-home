#include "App.h"

#include <Arduino.h>
#include <WiFi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/time.h>

#include "config/Config.h"
#include "core/Logger.h"
#include "display/DisplayManager.h"
#include "wifi/WifiManager.h"

App::App(DisplayManager &dm, WifiManager &wifi, IDataProvider &provider,
         IPage *const *pages, uint8_t pageCount)
    : dm_(dm), wifi_(wifi), provider_(provider), pages_(pages),
      pageCount_(pageCount) {}

// -----------------------------------------------------------------------------
//  Lifecycle
// -----------------------------------------------------------------------------
void App::begin() {
  // Apply the local timezone up front so both API-epoch and NTP paths render
  // local time via getLocalTime().
  setenv("TZ", config::POSIX_TZ, 1);
  tzset();

  provider_.begin();

  if (!dm_.begin()) {
    // OLED missing: dump a detailed I2C scan and run headless. WiFi/data still
    // work so the device recovers automatically if the panel is reseated+reset.
    LOG_ERROR("Display init failed — running headless. I2C diagnostics:");
    scanI2C();
  }

  bootSequence();

  // Register periodic jobs (order doesn't matter; each has its own interval).
  scheduler_.every(config::DATA_INTERVAL_MS, [this] { refreshData(); });
  scheduler_.every(config::WIFI_CHECK_MS, [this] { refreshWifi(); });
  scheduler_.every(config::PAGE_ROTATE_MS, [this] { rotatePage(); });
  scheduler_.every(config::DISPLAY_INTERVAL_MS, [this] { renderFrame(); });

  // Prime state so the first frame isn't blank.
  refreshWifi();
  refreshData();

  LOG_INFO("Dashboard running (%u pages)", pageCount_);
}

void App::tick() {
  wifi_.loop();      // drive the connection state machine every loop
  scheduler_.tick(); // run any due jobs
}

// -----------------------------------------------------------------------------
//  Scheduled jobs
// -----------------------------------------------------------------------------
void App::refreshData() {
  const uint32_t now = millis();
  if (provider_.fetch(data_)) {
    state_.dataValid = true;
    lastFetchMs_ = now;
  }
  state_.uptimeSec = now / 1000;
  state_.freeHeap = ESP.getFreeHeap();

  // Stale: we've had data before but none recently (backend/Pi/network down).
  state_.stale = state_.dataValid && (now - lastFetchMs_) > config::STALE_AFTER_MS;
  // Degraded: last good fetch reported a source as failing (backend reachable).
  state_.degraded = !state_.stale && !(data_.health.prometheus &&
                                       data_.health.pihole && data_.health.speedtest);

  updateClock();
}

void App::refreshWifi() {
  state_.wifiConnected = wifi_.isConnected();
  state_.rssi = wifi_.rssi();
  strncpy(state_.ip, wifi_.ip(), sizeof(state_.ip) - 1);
  state_.ip[sizeof(state_.ip) - 1] = '\0';
}

void App::rotatePage() {
  if (transitioning_ || pageCount_ < 2) {
    return; // don't stack transitions
  }
  nextPage_ = static_cast<uint8_t>((currentPage_ + 1) % pageCount_);
  transitioning_ = true;
  transitionStart_ = millis();
}

void App::renderFrame() {
  if (!dm_.ready()) {
    return; // headless mode
  }

  dm_.clear();

  // Cold start with no link and no data yet -> dedicated status screen.
  if (!wifi_.isConnected() && !state_.dataValid) {
    drawWifiDownScreen();
    dm_.send();
    return;
  }

  const char *title;
  if (transitioning_) {
    float t = static_cast<float>(millis() - transitionStart_) / config::TRANSITION_MS;
    if (t >= 1.0f) t = 1.0f;
    const int dx = static_cast<int>(t * config::SCREEN_WIDTH);

    // Outgoing slides left; incoming follows in from the right.
    dm_.setOffsetX(-dx);
    pages_[currentPage_]->render(dm_, data_, state_);
    dm_.setOffsetX(config::SCREEN_WIDTH - dx);
    pages_[nextPage_]->render(dm_, data_, state_);
    dm_.setOffsetX(0);

    title = (t < 0.5f) ? pages_[currentPage_]->title() : pages_[nextPage_]->title();

    if (t >= 1.0f) {
      currentPage_ = nextPage_;
      transitioning_ = false;
    }
  } else {
    dm_.setOffsetX(0);
    pages_[currentPage_]->render(dm_, data_, state_);
    title = pages_[currentPage_]->title();
  }

  dm_.header(title, state_);
  dm_.send();
}

// -----------------------------------------------------------------------------
//  Screens
// -----------------------------------------------------------------------------
void App::drawWifiDownScreen() {
  dm_.setOffsetX(0);
  dm_.fontMedium();
  dm_.textCentered(64, 30, "WiFi Disconnected");
  dm_.fontSmall();
  dm_.textCentered(64, 48, "Reconnecting...");
}

// -----------------------------------------------------------------------------
//  Boot
// -----------------------------------------------------------------------------
void App::bootSequence() {
  // 1. Logo splash.
  if (dm_.ready()) {
    dm_.clear();
    dm_.setOffsetX(0);
    dm_.logo(64, 24);
    dm_.fontMedium();
    dm_.textCentered(64, 56, "HOMELAB");
    dm_.send();
  }
  LOG_INFO("Homelab Companion %s", FW_VERSION);
  pause(1200);

  bootLineCount_ = 0;
  bootStep("Initializing...");
  pause(500);

  // 2. WiFi.
  bootStep("Connecting WiFi...");
  wifi_.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
  const uint32_t start = millis();
  while (!wifi_.isConnected() && wifi_.state() != WifiManager::State::Failed &&
         (millis() - start) < 10000) {
    wifi_.loop();
    yield();
  }
  if (wifi_.isConnected()) {
    char b[BOOT_LINE_LEN];
    snprintf(b, sizeof(b), "Connected %s", wifi_.ip());
    bootStep(b);
    // No NTP kick here: the clock is set from the API epoch on the first fetch.
    // NTP only engages via updateClock() as a fallback if no epoch ever arrives.
  } else {
    bootStep("WiFi failed (retrying)");
  }
  pause(700);

  // 3. I2C / OLED.
  bootStep("Scanning I2C...");
  pause(400);
  if (dm_.ready()) {
    char b[BOOT_LINE_LEN];
    snprintf(b, sizeof(b), "OLED found 0x%02X", config::OLED_I2C_ADDRESS);
    bootStep(b);
  } else {
    bootStep("OLED NOT FOUND");
  }
  pause(500);

  bootStep("Starting Dashboard...");
  pause(700);
}

void App::bootStep(const char *line) {
  if (bootLineCount_ < MAX_BOOT_LINES) {
    strncpy(bootLines_[bootLineCount_], line, BOOT_LINE_LEN - 1);
    bootLines_[bootLineCount_][BOOT_LINE_LEN - 1] = '\0';
    bootLineCount_++;
  }
  LOG_INFO("BOOT: %s", line);

  if (!dm_.ready()) {
    return;
  }

  // Show the last few lines under a small title bar.
  dm_.clear();
  dm_.setOffsetX(0);
  dm_.fontMedium();
  dm_.text(2, 11, "BOOTING");
  dm_.hline(0, 13, config::SCREEN_WIDTH);

  dm_.fontSmall();
  const uint8_t visible = 5;
  const uint8_t startIdx = (bootLineCount_ > visible) ? bootLineCount_ - visible : 0;
  for (uint8_t i = startIdx; i < bootLineCount_; ++i) {
    dm_.text(2, 26 + (i - startIdx) * 9, bootLines_[i]);
  }
  dm_.send();
}

void App::scanI2C() {
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; ++addr) {
    if (dm_.i2cProbe(addr)) {
      LOG_ERROR("  I2C device at 0x%02X", addr);
      ++found;
    }
  }
  if (found == 0) {
    LOG_ERROR("  No I2C devices found. Check SDA=%u SCL=%u wiring/power.",
              config::I2C_SDA_PIN, config::I2C_SCL_PIN);
  }
}

void App::kickNtp() {
  if (!wifi_.isConnected()) {
    return;
  }
  configTzTime(config::POSIX_TZ, config::NTP_SERVER, config::NTP_SERVER_2,
               config::NTP_SERVER_3);
  lastNtpAttemptMs_ = millis();
  LOG_INFO("NTP sync requested (%s / %s / %s, TZ=%s)", config::NTP_SERVER,
           config::NTP_SERVER_2, config::NTP_SERVER_3, config::POSIX_TZ);
}

void App::updateClock() {
  // Primary source: the Pi's clock, delivered with the dashboard data over the
  // already-working HTTP path. Set the RTC once from it; it then free-runs.
  if (!timeSynced_ && data_.epoch > 0) {
    const struct timeval tv = {.tv_sec = static_cast<time_t>(data_.epoch), .tv_usec = 0};
    settimeofday(&tv, nullptr);
    LOG_INFO("clock set from API epoch=%lu", static_cast<unsigned long>(data_.epoch));
  }

  struct tm tinfo;
  if (getLocalTime(&tinfo, 5)) {
    strftime(state_.timeStr, sizeof(state_.timeStr), "%H:%M:%S", &tinfo);
    if (!timeSynced_) {
      timeSynced_ = true;
      LOG_INFO("clock synced=%s", state_.timeStr);
    }
    return;
  }

  // Fallback (networks that permit NTP): only engages after NTP_RETRY_MS with no
  // API epoch. In normal operation epoch arrives within ~1-2s and sets the clock
  // first, so this never fires and no NTP traffic is generated.
  const uint32_t now = millis();
  if (wifi_.isConnected() && (now - lastNtpAttemptMs_) > config::NTP_RETRY_MS) {
    LOG_WARN("clock not set from API after %lus, falling back to NTP",
             static_cast<unsigned long>(config::NTP_RETRY_MS / 1000));
    kickNtp();
  }
}

void App::pause(uint32_t ms) {
  // Boot-only cooperative wait: no delay(), keeps WiFi + watchdog serviced.
  const uint32_t start = millis();
  while ((millis() - start) < ms) {
    wifi_.loop();
    yield();
  }
}
