#pragma once
#include <Arduino.h>
#include <functional>

// =============================================================================
//  Cooperative, non-blocking scheduler. Register periodic tasks; call tick()
//  from loop(). Uses millis() and is rollover-safe. No delay() anywhere.
// =============================================================================

class Scheduler {
public:
  using Callback = std::function<void()>;
  static constexpr uint8_t MAX_TASKS = 8;

  // Register a task to run every intervalMs. Returns false if full.
  bool every(uint32_t intervalMs, Callback cb);

  // Run any due tasks. Call once per loop().
  void tick();

private:
  struct Task {
    uint32_t interval = 0;
    uint32_t lastRun = 0;
    Callback cb;
    bool used = false;
  };

  Task tasks_[MAX_TASKS];
  uint8_t count_ = 0;
};
