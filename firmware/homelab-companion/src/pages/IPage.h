#pragma once
#include "display/DisplayManager.h"
#include "models/DashboardData.h"

// =============================================================================
//  IPage — one dashboard screen.
//
//  Implementations draw ONLY the content region (the header is drawn by App).
//  Draw through the DisplayManager helpers so the slide-transition offset is
//  applied. Content region: y in [DisplayManager::CONTENT_Y, SCREEN_HEIGHT-1].
// =============================================================================

class IPage {
public:
  virtual ~IPage() = default;

  // Short, uppercase title shown in the header.
  virtual const char *title() const = 0;

  // Render this page's content.
  virtual void render(DisplayManager &dm, const DashboardData &data,
                      const AppState &state) = 0;
};
