#pragma once
#include "pages/IPage.h"

// About: firmware version, IP address, free heap.
class AboutPage : public IPage {
public:
  const char *title() const override { return "ABOUT"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
