#include "AboutPage.h"

#include <cstdio>

#include "config/Config.h"

namespace {
void row(DisplayManager &dm, int baseline, const char *label, const char *value) {
  dm.text(2, baseline, label);
  dm.textRight(126, baseline, value);
}
} // namespace

void AboutPage::render(DisplayManager &dm, const DashboardData &data,
                       const AppState &state) {
  dm.fontSmall();

  char buf[16];
  row(dm, 25, "Firmware", FW_VERSION);
  row(dm, 37, "IP", state.ip);
  snprintf(buf, sizeof(buf), "%lu KB",
           static_cast<unsigned long>(state.freeHeap / 1024));
  row(dm, 49, "Free heap", buf);

  // Source health: overall STALE when the backend is unreachable, otherwise a
  // per-source dot for Prometheus / Pi-hole / Speedtest.
  dm.text(2, 61, "Sources");
  if (state.stale) {
    dm.textRight(126, 61, "STALE");
  } else {
    struct Src {
      const char *l;
      bool ok;
    };
    const Src src[] = {
        {"P", data.health.prometheus},
        {"H", data.health.pihole},
        {"S", data.health.speedtest},
    };
    int x = 74;
    for (const Src &s : src) {
      dm.text(x, 61, s.l);
      dm.statusDot(x + 8, 55, s.ok);
      x += 18;
    }
  }
}
