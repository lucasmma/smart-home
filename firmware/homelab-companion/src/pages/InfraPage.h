#pragma once
#include "pages/IPage.h"

// Infrastructure: up/down status of Raspberry Pi, Ubuntu, Internet, Pi-hole.
class InfraPage : public IPage {
public:
  const char *title() const override { return "INFRA"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
