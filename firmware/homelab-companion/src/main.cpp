#include <Arduino.h>

#include "app/App.h"
#include "config/Config.h"
#include "core/Logger.h"
#include "display/DisplayManager.h"
#include "pages/AboutPage.h"
#include "pages/HomePage.h"
#include "pages/InfraPage.h"
#include "pages/NetworkPage.h"
#include "pages/PiholePage.h"
#include "pages/ResourcesPage.h"
#include "providers/HttpProvider.h"
#include "providers/MockProvider.h"
#include "wifi/WifiManager.h"

// --- Singletons (fixed, so no dynamic allocation / no global mutable soup) ---
DisplayManager display;
WifiManager wifi;

// Data source. To use the real Raspberry Pi API instead of mock data, swap the
// provider passed to App below for `httpProvider` — that is the ONLY change.
MockProvider mockProvider;
HttpProvider httpProvider;

// Pages, in rotation order.
HomePage homePage;
InfraPage infraPage;
ResourcesPage resourcesPage;
NetworkPage networkPage;
PiholePage piholePage;
AboutPage aboutPage;

IPage *pages[] = {
    &homePage, &infraPage, &resourcesPage,
    &networkPage, &piholePage, &aboutPage,
};

// Inject the provider here. Using the live companion-api on the Pi; switch to
// `mockProvider` to run without WiFi/the backend.
App app(display, wifi, httpProvider, pages,
        sizeof(pages) / sizeof(pages[0]));

void setup() {
  Serial.begin(115200);
  LOG_INFO("=== Homelab Companion %s ===", FW_VERSION);
  app.begin();
}

void loop() {
  app.tick();
}
