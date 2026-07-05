#pragma once
#include <cstdint>

// =============================================================================
//  Central compile-time configuration. No magic numbers live outside this file.
//  Copy your real WiFi credentials + API host in here before flashing.
// =============================================================================

// --- Secrets -----------------------------------------------------------------
// WiFi credentials, API host and token are injected as -D build flags from the
// git-ignored secrets.ini (the firmware's ".env"; see secrets.ini.example).
// These fallbacks keep the project compiling if that file is absent — they are
// safe placeholders, NOT real credentials.
#ifndef CFG_WIFI_SSID
#define CFG_WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef CFG_WIFI_PASSWORD
#define CFG_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif
#ifndef CFG_API_BASE_URL
#define CFG_API_BASE_URL "http://192.168.1.10:8090"
#endif
#ifndef CFG_API_TOKEN
#define CFG_API_TOKEN ""
#endif

namespace config {

// --- WiFi -------------------------------------------------------------------
inline constexpr char WIFI_SSID[] = CFG_WIFI_SSID;
inline constexpr char WIFI_PASSWORD[] = CFG_WIFI_PASSWORD;

// --- Companion API ----------------------------------------------------------
// The single endpoint the firmware fetches. Point at the Pi running
// services/companion-api (see compose/companion-api.yml).
inline constexpr char API_BASE_URL[] = CFG_API_BASE_URL;
inline constexpr char API_PATH[] = "/api/display";
// Optional bearer token. Empty => no Authorization header.
// Must match COMPANION_API_TOKEN on the backend when set.
inline constexpr char API_TOKEN[] = CFG_API_TOKEN;
inline constexpr uint32_t HTTP_TIMEOUT_MS = 4000;

// --- Time (NTP) -------------------------------------------------------------
// Three independent servers: if DNS/one provider is blocked (e.g. via Pi-hole),
// the others still resolve. The clock re-syncs on every boot (no battery RTC).
inline constexpr char NTP_SERVER[] = "pool.ntp.org";
inline constexpr char NTP_SERVER_2[] = "time.google.com";
inline constexpr char NTP_SERVER_3[] = "time.cloudflare.com";
// How often to re-request a sync while the clock is still unset.
inline constexpr uint32_t NTP_RETRY_MS = 10000;
// POSIX TZ string. Default: America/Sao_Paulo (UTC-3, no DST).
inline constexpr char POSIX_TZ[] = "<-03>3";

// --- I2C / OLED -------------------------------------------------------------
inline constexpr uint8_t I2C_SDA_PIN = 21;
inline constexpr uint8_t I2C_SCL_PIN = 22;
inline constexpr uint8_t OLED_I2C_ADDRESS = 0x3C; // 7-bit; common alt is 0x3D
inline constexpr uint32_t I2C_CLOCK_HZ = 400000;  // fast mode

inline constexpr uint8_t SCREEN_WIDTH = 128;
inline constexpr uint8_t SCREEN_HEIGHT = 64;

// --- Scheduler intervals (milliseconds) -------------------------------------
inline constexpr uint32_t DISPLAY_INTERVAL_MS = 33;   // ~30 FPS
inline constexpr uint32_t DATA_INTERVAL_MS = 10000;   // provider refresh (API poll)
inline constexpr uint32_t CLOCK_INTERVAL_MS = 1000;   // clock string refresh (local)
inline constexpr uint32_t PAGE_ROTATE_MS = 5000;      // page rotation
inline constexpr uint32_t WIFI_CHECK_MS = 10000;      // wifi status poll

// --- UI ---------------------------------------------------------------------
inline constexpr uint32_t TRANSITION_MS = 350; // page slide duration
// If no successful fetch within this window, the header shows a "stale" warning
// (backend/Pi/network unreachable). Should comfortably exceed DATA_INTERVAL_MS so
// a single missed/slow poll doesn't false-flag stale (~3x the poll interval).
inline constexpr uint32_t STALE_AFTER_MS = 35000;

// --- Firmware ---------------------------------------------------------------
#ifndef FW_VERSION
#define FW_VERSION "0.0.0-dev"
#endif

} // namespace config
