#pragma once
#include "providers/IDataProvider.h"

// =============================================================================
//  HttpProvider — fetches the dashboard from the companion-api on the Pi.
//
//  GET {API_BASE_URL}{API_PATH} -> the DisplayData JSON contract. On any
//  failure fetch() returns false and leaves `out` untouched, so the UI keeps
//  showing the last known good data.
//
//  Drop-in replacement for MockProvider: change one line in main.cpp.
// =============================================================================

class HttpProvider : public IDataProvider {
public:
  void begin() override;
  bool fetch(DashboardData &out) override;
};
