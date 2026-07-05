#pragma once
#include "pages/IPage.h"

// Home: large clock, device uptime, WiFi signal.
class HomePage : public IPage {
public:
  const char *title() const override { return "HOME"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
