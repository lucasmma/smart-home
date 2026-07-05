#pragma once
#include <cstdint>

#include "models/DashboardData.h"
#include "pages/IPage.h"
#include "providers/IDataProvider.h"
#include "scheduler/Scheduler.h"

class DisplayManager;
class WifiManager;

// =============================================================================
//  App — top-level orchestrator.
//
//  Wires the display, WiFi, data provider (injected) and pages together, runs
//  the boot sequence, and drives everything from a non-blocking scheduler.
// =============================================================================

class App {
public:
  App(DisplayManager &dm, WifiManager &wifi, IDataProvider &provider,
      IPage *const *pages, uint8_t pageCount);

  void begin(); // one-time init + boot sequence
  void tick();  // call every loop()

private:
  // Scheduled jobs.
  void refreshData();
  void refreshWifi();
  void rotatePage();
  void renderFrame();

  // Boot / diagnostics.
  void bootSequence();
  void bootStep(const char *line);
  void scanI2C();
  void kickNtp();     // (re)issue an NTP sync request (multi-server)
  void updateClock(); // refresh timeStr; keep retrying NTP until synced
  void pause(uint32_t ms); // non-blocking spin (boot only; no delay())

  // Screens.
  void drawWifiDownScreen();

  DisplayManager &dm_;
  WifiManager &wifi_;
  IDataProvider &provider_;
  IPage *const *pages_;
  const uint8_t pageCount_;

  Scheduler scheduler_;
  DashboardData data_;
  AppState state_;

  // Page rotation / slide transition.
  uint8_t currentPage_ = 0;
  uint8_t nextPage_ = 0;
  bool transitioning_ = false;
  uint32_t transitionStart_ = 0;

  bool timeSynced_ = false;       // NTP has set the clock at least once
  uint32_t lastNtpAttemptMs_ = 0; // millis() of the last NTP (re)config
  uint32_t lastFetchMs_ = 0;      // millis() of the last successful provider fetch

  // Boot log (shows the last few lines on the OLED).
  static constexpr uint8_t MAX_BOOT_LINES = 8;
  static constexpr uint8_t BOOT_LINE_LEN = 24;
  char bootLines_[MAX_BOOT_LINES][BOOT_LINE_LEN];
  uint8_t bootLineCount_ = 0;
};
