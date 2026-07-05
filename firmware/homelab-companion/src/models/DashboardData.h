#pragma once
#include <cstdint>

// =============================================================================
//  Data models.
//
//  DashboardData mirrors the companion-api `GET /api/display` JSON contract
//  1:1 — keep the two in sync. AppState holds device-local state that never
//  comes from the API (WiFi, uptime, heap, clock).
// =============================================================================

// Per-source liveness reported by the backend. false => that source's fields
// are frozen at their last-good value. Defaults true so a missing `health`
// object (older backend / mock data) never raises a false alarm.
struct SourceHealth {
  bool prometheus = true;
  bool pihole = true;
  bool speedtest = true;
};

struct DashboardData {
  bool internet = false;
  bool raspberry = false;
  bool ubuntu = false;
  uint8_t cpu = 0;  // percent 0-100
  uint8_t ram = 0;  // percent 0-100
  uint8_t disk = 0; // percent 0-100
  uint16_t download = 0; // Mbps
  uint16_t upload = 0;   // Mbps
  uint16_t ping = 0;     // ms
  uint32_t adsBlocked = 0;
  uint32_t queries = 0;
  SourceHealth health;
};

struct AppState {
  bool wifiConnected = false;
  int32_t rssi = 0;              // dBm (0 when disconnected)
  char ip[16] = "0.0.0.0";
  char timeStr[9] = "--:--:--";  // "HH:MM:SS", "--" until NTP sync
  uint32_t uptimeSec = 0;        // device uptime
  uint32_t freeHeap = 0;         // bytes
  bool dataValid = false;        // has the provider produced good data at least once?
  bool stale = false;            // no successful fetch within STALE_AFTER_MS (backend down)
  bool degraded = false;         // backend reachable but a source is failing
};
