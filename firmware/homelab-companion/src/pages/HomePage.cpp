#include "HomePage.h"

#include <cstdio>

void HomePage::render(DisplayManager &dm, const DashboardData &data,
                      const AppState &state) {
  (void)data;

  // Hero clock.
  dm.fontLarge();
  dm.textCentered(64, 42, state.timeStr);

  // Footer: uptime (left) + RSSI (right).
  dm.fontSmall();
  char buf[24];
  const uint32_t s = state.uptimeSec;
  const uint32_t d = s / 86400;
  const uint32_t h = (s % 86400) / 3600;
  const uint32_t m = (s % 3600) / 60;
  snprintf(buf, sizeof(buf), "UP %lud %02lu:%02lu",
           static_cast<unsigned long>(d), static_cast<unsigned long>(h),
           static_cast<unsigned long>(m));
  dm.text(2, 62, buf);

  if (state.wifiConnected) {
    snprintf(buf, sizeof(buf), "%ld dBm", static_cast<long>(state.rssi));
  } else {
    snprintf(buf, sizeof(buf), "offline");
  }
  dm.textRight(126, 62, buf);
}
