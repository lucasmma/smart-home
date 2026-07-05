#include "HttpProvider.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "config/Config.h"
#include "core/Logger.h"

void HttpProvider::begin() {
  LOG_INFO("HttpProvider target: %s%s", config::API_BASE_URL, config::API_PATH);
}

bool HttpProvider::fetch(DashboardData &out) {
  // No point issuing a request without a link.
  if (WiFi.status() != WL_CONNECTED) {
    LOG_DEBUG("HttpProvider: wifi down, skipping fetch");
    return false;
  }

  HTTPClient http;
  String url = String(config::API_BASE_URL) + config::API_PATH;

  if (!http.begin(url)) {
    LOG_WARN("HttpProvider: begin() failed for %s", url.c_str());
    return false;
  }
  http.setConnectTimeout(config::HTTP_TIMEOUT_MS);
  http.setTimeout(config::HTTP_TIMEOUT_MS);
  if (strlen(config::API_TOKEN) > 0) {
    http.addHeader("Authorization", String("Bearer ") + config::API_TOKEN);
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    LOG_WARN("HttpProvider: GET %s -> %d", url.c_str(), code);
    http.end();
    return false;
  }

  // Parse straight from the stream to keep memory use low.
  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();

  if (err) {
    LOG_WARN("HttpProvider: JSON parse failed: %s", err.c_str());
    return false;
  }

  // Only commit into `out` once parsing has succeeded (keep last-good on error).
  out.internet = doc["internet"] | false;
  out.raspberry = doc["raspberry"] | false;
  out.ubuntu = doc["ubuntu"] | false;
  out.cpu = doc["cpu"] | 0;
  out.ram = doc["ram"] | 0;
  out.disk = doc["disk"] | 0;
  out.download = doc["download"] | 0;
  out.upload = doc["upload"] | 0;
  out.ping = doc["ping"] | 0;
  out.adsBlocked = doc["adsBlocked"] | 0UL;
  out.queries = doc["queries"] | 0UL;

  // Per-source health (defaults true if the backend omits it).
  JsonVariantConst health = doc["health"];
  out.health.prometheus = health["prometheus"] | true;
  out.health.pihole = health["pihole"] | true;
  out.health.speedtest = health["speedtest"] | true;

  return true;
}
