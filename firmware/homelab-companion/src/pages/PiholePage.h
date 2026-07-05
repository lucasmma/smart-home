#pragma once
#include "pages/IPage.h"

// Pi-hole: ads blocked today (hero), total queries, block rate.
class PiholePage : public IPage {
public:
  const char *title() const override { return "PI-HOLE"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
