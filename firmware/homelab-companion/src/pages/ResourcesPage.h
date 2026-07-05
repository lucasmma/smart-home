#pragma once
#include "pages/IPage.h"

// Resources: CPU / RAM / Disk usage as rounded progress bars.
class ResourcesPage : public IPage {
public:
  const char *title() const override { return "RESOURCES"; }
  void render(DisplayManager &dm, const DashboardData &data,
              const AppState &state) override;
};
