#pragma once
#include <Arduino.h>
#include <cstdint>

// =============================================================================
//  WifiManager — non-blocking connection management.
//
//  Owns the STA lifecycle: connect, auto-reconnect with backoff, and exposes
//  signal strength / IP / state. A captive-portal provisioning mode is stubbed
//  for the future (startCaptivePortal()).
// =============================================================================

class WifiManager {
public:
  enum class State : uint8_t {
    Disconnected,
    Connecting,
    Connected,
    Failed,
  };

  // Configure credentials and start the first (non-blocking) connect attempt.
  void begin(const char *ssid, const char *password);

  // Drive the state machine; call periodically (does not block).
  void loop();

  State state() const { return state_; }
  bool isConnected() const { return state_ == State::Connected; }

  int rssi() const;              // dBm; 0 when disconnected
  const char *ip() const;        // dotted string; "0.0.0.0" when disconnected

  // Future: bring up a SoftAP + captive portal to provision credentials.
  void startCaptivePortal(); // TODO: not yet implemented

private:
  void startConnect();
  void cacheIp();

  const char *ssid_ = nullptr;
  const char *password_ = nullptr;
  State state_ = State::Disconnected;

  uint32_t attemptStart_ = 0;   // when the current connect attempt began
  uint32_t lastRetry_ = 0;      // when we last (re)started a connect
  char ipStr_[16] = "0.0.0.0";

  static constexpr uint32_t CONNECT_TIMEOUT_MS = 15000;
  static constexpr uint32_t RETRY_BACKOFF_MS = 5000;
};
