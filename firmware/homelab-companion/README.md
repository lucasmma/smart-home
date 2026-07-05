# Homelab Companion (ESP32 firmware)

A desk device that displays the status of the homelab on a small OLED, rotating
through polished pages. Built with **PlatformIO** + **U8g2**, clean modular C++.

Data is **mocked** out of the box so it works before the backend exists. When
you're ready, point it at [`services/companion-api`](../../services/companion-api)
by swapping one line (see [Switching to real data](#switching-to-real-data)).

## Hardware

| Signal | ESP32 DevKit V1 | SSD1306 OLED |
|--------|-----------------|--------------|
| SDA    | GPIO21          | SDA          |
| SCL    | GPIO22          | SCL          |
| 3V3    | 3V3             | VDD          |
| GND    | GND             | GND          |

Default I2C address `0x3C` (some panels are `0x3D` — change `OLED_I2C_ADDRESS`
in `src/config/Config.h`).

## Build & flash

```bash
# from this directory
pio run                       # build (debug, DEBUG logs on)
pio run -e esp32-release      # build (release, DEBUG logs stripped)
pio run -t upload             # flash
pio device monitor            # serial @ 115200
```

Before flashing, edit `src/config/Config.h`: set `WIFI_SSID` / `WIFI_PASSWORD`
and `API_BASE_URL` (the Pi running the companion-api).

## Pages

The display rotates every 5 s with a horizontal slide transition:

1. **Home** — clock, uptime, WiFi signal
2. **Infrastructure** — Raspberry Pi / Ubuntu / Internet / Pi-hole up/down
3. **Resources** — CPU / RAM / Disk progress bars
4. **Network** — download / upload / ping
5. **Pi-hole** — ads blocked today, queries, block rate
6. **About** — firmware version, IP, free heap

## Architecture

```
src/
  main.cpp                 wiring + dependency injection
  config/Config.h          all constants, credentials, pins, intervals
  core/Logger.h            INFO/WARN/ERROR/DEBUG (DEBUG off in release)
  scheduler/               non-blocking millis() task runner (no delay())
  models/DashboardData.h   the API contract + device AppState
  providers/               IDataProvider + MockProvider + HttpProvider
  wifi/                    WifiManager: reconnect, RSSI, IP, state
  display/                 DisplayManager (U8g2 toolkit) + Icons
  pages/                   IPage + one class per screen
  app/                     App: boot sequence, scheduling, transitions
```

Everything depends on interfaces (`IDataProvider`, `IPage`), so screens and data
sources are swappable without touching the rest of the app.

### Scheduling

| Job            | Interval |
|----------------|----------|
| Display render | ~30 FPS  |
| Data refresh   | 1 s      |
| Page rotation  | 5 s      |
| WiFi status    | 10 s     |

`loop()` only calls `app.tick()`, which drives the WiFi state machine and the
scheduler. No blocking anywhere (the boot splash uses a cooperative
`millis()` spin, not `delay()`).

## Switching to real data

`MockProvider` and `HttpProvider` both implement `IDataProvider`. In
`src/main.cpp`, change the provider passed to `App`:

```cpp
App app(display, wifi, httpProvider, pages, /* count */);
//                     ^^^^^^^^^^^^  was: mockProvider
```

`HttpProvider` fetches `GET {API_BASE_URL}{API_PATH}` and parses the JSON into
`DashboardData`. On any failure it keeps the last known good data on screen.

## Error handling

- **WiFi fails** — a dedicated "WiFi Disconnected / Reconnecting" screen on cold
  start; once any data has been shown, the last known values stay up.
- **OLED not found** — detailed I2C scan + wiring hints logged to Serial; the
  device keeps running headless and recovers on reset once reseated.
- **HTTP fails** (real-data mode) — last known data remains on screen.
