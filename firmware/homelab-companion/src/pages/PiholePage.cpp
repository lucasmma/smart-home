#include "PiholePage.h"

#include <cstdint>
#include <cstdio>

#include "display/Icons.h"

void PiholePage::render(DisplayManager &dm, const DashboardData &data,
                        const AppState &state) {
  (void)state;
  char buf[24];

  // Label + block-rate badge on the top row.
  dm.fontSmall();
  dm.icon(2, 18, icons::SHIELD);
  dm.text(13, 26, "Ads Blocked Today");

  const uint32_t pct =
      data.queries > 0
          ? static_cast<uint32_t>((static_cast<uint64_t>(data.adsBlocked) * 100) /
                                  data.queries)
          : 0;
  snprintf(buf, sizeof(buf), "%lu%%", static_cast<unsigned long>(pct));
  dm.textRight(126, 26, buf);

  // Hero: total ads blocked.
  dm.fontLarge();
  snprintf(buf, sizeof(buf), "%lu", static_cast<unsigned long>(data.adsBlocked));
  dm.text(2, 48, buf);

  // Footer: total queries today.
  dm.fontSmall();
  snprintf(buf, sizeof(buf), "%lu queries today",
           static_cast<unsigned long>(data.queries));
  dm.text(2, 62, buf);
}
