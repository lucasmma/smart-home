#pragma once
#include "pages/IPage.h"

// Network: download (hero), upload, ping.
class NetworkPage : public IPage {
public:
  const char *title() const override { return "NETWORK"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
