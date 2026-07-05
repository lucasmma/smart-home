#pragma once
#include <Arduino.h>

// =============================================================================
//  Lightweight logging. Four levels; DEBUG is compiled out when -D RELEASE is
//  set (see the esp32-release env in platformio.ini). Everything routes to the
//  USB serial monitor at 115200 baud.
// =============================================================================

#define LOG_PRINTF(tag, fmt, ...) Serial.printf("[" tag "] " fmt "\n", ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) LOG_PRINTF("INFO", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_PRINTF("WARN", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_PRINTF("ERROR", fmt, ##__VA_ARGS__)

#ifdef RELEASE
#define LOG_DEBUG(fmt, ...) ((void)0)
#else
#define LOG_DEBUG(fmt, ...) LOG_PRINTF("DEBUG", fmt, ##__VA_ARGS__)
#endif
