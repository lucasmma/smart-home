#include "InfraPage.h"

void InfraPage::render(DisplayManager &dm, const DashboardData &data,
                       const AppState &state) {
  dm.fontSmall();

  // Pi-hole is "up" if we actually received query stats from it.
  const bool piholeOk = state.dataValid && data.queries > 0;

  struct Item {
    int x, y;
    const char *label;
    bool ok;
  };
  const Item items[] = {
      {4, 32, "Rasp Pi", data.raspberry},
      {68, 32, "Ubuntu", data.ubuntu},
      {4, 52, "Internet", data.internet},
      {68, 52, "Pi-hole", piholeOk},
  };

  for (const Item &it : items) {
    dm.statusDot(it.x, it.y - 6, it.ok);
    dm.text(it.x + 10, it.y, it.label);
  }
}
