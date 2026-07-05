#pragma once
#include "providers/IDataProvider.h"

// =============================================================================
//  MockProvider — realistic, slowly-drifting fake data.
//
//  Values evolve as a bounded random walk so the UI looks alive without
//  jumping erratically. Used until the real HttpProvider is wired in.
// =============================================================================

class MockProvider : public IDataProvider {
public:
  void begin() override;
  bool fetch(DashboardData &out) override;

private:
  // Internal float state for smooth drift; exposed values are rounded.
  float cpu_ = 22.0f;
  float ram_ = 31.0f;
  float disk_ = 58.0f;
  float download_ = 900.0f;
  float upload_ = 110.0f;
  float ping_ = 8.0f;
  uint32_t adsBlocked_ = 183900;
  uint32_t queries_ = 312900;
  bool seeded_ = false;

  // Drift a value by up to +/-step, clamped to [lo, hi].
  static float drift(float value, float step, float lo, float hi);
};
