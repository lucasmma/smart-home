#pragma once
#include "models/DashboardData.h"

// =============================================================================
//  Data provider seam (dependency injection).
//
//  The rest of the app depends only on this interface, so it never knows
//  whether data is mocked or fetched over HTTP. Swapping MockProvider for
//  HttpProvider is a one-line change in main.cpp.
// =============================================================================

class IDataProvider {
public:
  virtual ~IDataProvider() = default;

  // Optional one-time setup (e.g. seed RNG). Default: no-op.
  virtual void begin() {}

  // Populate `out` with the latest data. Return true on success. On failure the
  // caller keeps the previous (last known good) data — so `out` must be left
  // untouched when returning false.
  virtual bool fetch(DashboardData &out) = 0;
};
