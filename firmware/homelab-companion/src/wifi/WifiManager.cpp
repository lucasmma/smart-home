#include "WifiManager.h"

#include <WiFi.h>

#include "core/Logger.h"

void WifiManager::begin(const char *ssid, const char *password) {
  ssid_ = ssid;
  password_ = password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  startConnect();
}

void WifiManager::startConnect() {
  LOG_INFO("WiFi: connecting to \"%s\"", ssid_);
  WiFi.disconnect();
  WiFi.begin(ssid_, password_);
  state_ = State::Connecting;
  attemptStart_ = millis();
  lastRetry_ = attemptStart_;
}

void WifiManager::cacheIp() {
  WiFi.localIP().toString().toCharArray(ipStr_, sizeof(ipStr_));
}

void WifiManager::loop() {
  const bool linkUp = WiFi.status() == WL_CONNECTED;
  const uint32_t now = millis();

  switch (state_) {
    case State::Connecting:
      if (linkUp) {
        state_ = State::Connected;
        cacheIp();
        LOG_INFO("WiFi: connected, ip=%s rssi=%d dBm", ipStr_, WiFi.RSSI());
      } else if (now - attemptStart_ >= CONNECT_TIMEOUT_MS) {
        state_ = State::Failed;
        LOG_WARN("WiFi: connect timed out");
      }
      break;

    case State::Connected:
      if (!linkUp) {
        state_ = State::Disconnected;
        strncpy(ipStr_, "0.0.0.0", sizeof(ipStr_));
        LOG_WARN("WiFi: link lost");
      }
      break;

    case State::Disconnected:
    case State::Failed:
      // Retry on a fixed backoff.
      if (now - lastRetry_ >= RETRY_BACKOFF_MS) {
        startConnect();
      }
      break;
  }
}

int WifiManager::rssi() const {
  return isConnected() ? WiFi.RSSI() : 0;
}

const char *WifiManager::ip() const {
  return ipStr_;
}

void WifiManager::startCaptivePortal() {
  // TODO: bring up a SoftAP + DNS captive portal to collect SSID/password,
  // persist them (Preferences/NVS), then re-run begin(). Left as a future hook.
  LOG_WARN("WiFi: captive portal not implemented yet");
}
