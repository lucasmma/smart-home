#include "ResourcesPage.h"

#include <cstdio>

namespace {
void row(DisplayManager &dm, int baseline, const char *label, uint8_t pct) {
  char buf[6];
  dm.fontSmall();
  dm.text(2, baseline, label);
  dm.progressBar(30, baseline - 8, 70, 9, pct);
  snprintf(buf, sizeof(buf), "%u%%", pct);
  dm.textRight(126, baseline, buf);
}
} // namespace

void ResourcesPage::render(DisplayManager &dm, const DashboardData &data,
                           const AppState &state) {
  (void)state;
  row(dm, 28, "CPU", data.cpu);
  row(dm, 44, "RAM", data.ram);
  row(dm, 60, "DISK", data.disk);
}
