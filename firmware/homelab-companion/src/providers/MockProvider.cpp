#include "MockProvider.h"

#include <Arduino.h>
#include <cmath>

void MockProvider::begin() {
  // Seed from a floating analog pin for a little run-to-run variety.
  randomSeed(analogRead(A0) ^ micros());
  seeded_ = true;
}

float MockProvider::drift(float value, float step, float lo, float hi) {
  // random(-1000, 1001) -> [-1, 1] scaled by step.
  const float delta = (static_cast<float>(random(-1000, 1001)) / 1000.0f) * step;
  float next = value + delta;
  if (next < lo) next = lo;
  if (next > hi) next = hi;
  return next;
}

bool MockProvider::fetch(DashboardData &out) {
  cpu_ = drift(cpu_, 4.0f, 3.0f, 95.0f);
  ram_ = drift(ram_, 2.0f, 15.0f, 90.0f);
  disk_ = drift(disk_, 0.2f, 40.0f, 85.0f); // disk changes slowly
  download_ = drift(download_, 25.0f, 200.0f, 940.0f);
  upload_ = drift(upload_, 6.0f, 40.0f, 120.0f);
  ping_ = drift(ping_, 1.5f, 3.0f, 45.0f);

  // Counters only ever grow.
  adsBlocked_ += static_cast<uint32_t>(random(0, 12));
  queries_ += static_cast<uint32_t>(random(3, 40));

  out.cpu = static_cast<uint8_t>(lroundf(cpu_));
  out.ram = static_cast<uint8_t>(lroundf(ram_));
  out.disk = static_cast<uint8_t>(lroundf(disk_));
  out.download = static_cast<uint16_t>(lroundf(download_));
  out.upload = static_cast<uint16_t>(lroundf(upload_));
  out.ping = static_cast<uint16_t>(lroundf(ping_));
  out.adsBlocked = adsBlocked_;
  out.queries = queries_;

  // Occasionally flip service/internet status to exercise the UI.
  out.internet = random(0, 100) > 3;  // ~97% up
  out.raspberry = true;
  out.ubuntu = random(0, 100) > 6;    // ~94% up

  return true;
}
