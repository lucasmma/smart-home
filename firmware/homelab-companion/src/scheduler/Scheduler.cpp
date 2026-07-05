#include "Scheduler.h"

bool Scheduler::every(uint32_t intervalMs, Callback cb) {
  if (count_ >= MAX_TASKS) {
    return false;
  }
  Task &t = tasks_[count_++];
  t.interval = intervalMs;
  t.lastRun = millis();
  t.cb = std::move(cb);
  t.used = true;
  return true;
}

void Scheduler::tick() {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < count_; ++i) {
    Task &t = tasks_[i];
    // Unsigned subtraction handles millis() rollover correctly.
    if (t.used && (now - t.lastRun) >= t.interval) {
      t.lastRun = now;
      if (t.cb) {
        t.cb();
      }
    }
  }
}
