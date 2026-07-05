#include "NetworkPage.h"

#include <cstdio>

#include "display/Icons.h"

void NetworkPage::render(DisplayManager &dm, const DashboardData &data,
                         const AppState &state) {
  (void)state;
  char num[8];
  char line[24];

  // Hero: download speed with a down-arrow icon.
  dm.icon(2, 21, icons::ARROW_DOWN);
  dm.fontLarge();
  snprintf(num, sizeof(num), "%u", data.download);
  dm.text(13, 40, num);
  const int nw = dm.textWidth(num);
  dm.fontSmall();
  dm.text(13 + nw + 4, 40, "Mbps");

  // Footer: upload (left) + ping (right).
  dm.fontSmall();
  snprintf(line, sizeof(line), "UP %u Mbps", data.upload);
  dm.text(2, 60, line);
  snprintf(line, sizeof(line), "PING %u ms", data.ping);
  dm.textRight(126, 60, line);
}
