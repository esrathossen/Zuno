#pragma once

#include <Arduino.h>

namespace ZunoConfig {

constexpr uint32_t serialBaud = 115200UL;

// Display
constexpr uint8_t oledWidth = 128;
constexpr uint8_t oledHeight = 64;
constexpr uint8_t oledAddress = 0x3C;
constexpr uint8_t oledTargetFps = 30;
constexpr uint16_t oledFrameIntervalMs = 1000U / oledTargetFps;

// Robot behaviour
constexpr uint8_t stopDistanceCm = 20;
constexpr uint8_t slowDistanceCm = 40;
constexpr uint8_t selfDriveSpeed = 90;
constexpr uint8_t selfDriveSlowSpeed = 50;
constexpr uint8_t manualSpeed = 90;

// Timing
constexpr uint32_t dhtReadIntervalMs = 4000UL;
constexpr uint32_t wifiCheckIntervalMs = 5000UL;
constexpr uint32_t hudRefreshIntervalMs = 1000UL;
constexpr uint32_t batteryReadIntervalMs = 5000UL;
constexpr uint32_t lookAroundMinMs = 1500UL;
constexpr uint32_t lookAroundMaxMs = 3500UL;
constexpr uint32_t randomFaceIntervalMs = 30000UL;
constexpr uint32_t randomFaceDurationMs = 5000UL;
constexpr uint32_t standbyAngryTimeoutMs = 60UL * 1000UL;
constexpr uint32_t sleepIdleTimeoutMs = 5UL * 60UL * 1000UL;
constexpr uint32_t scanCooldownMs = 5000UL;
constexpr uint32_t selfDriveAutoStopMs = 3UL * 60UL * 1000UL;

// Battery: 2S Li-ion pack through common 30k/7.5k voltage sensor module.
constexpr float batteryMinV = 6.0f;
constexpr float batteryMaxV = 8.4f;
constexpr float batteryDividerRatio = 5.0f;
constexpr float batteryAdcVref = 3.3f;
constexpr uint16_t batteryAdcMax = 4095U;
constexpr uint8_t batteryLowPercent = 20;

// WiFi fallback access point. Station credentials are stored later through
// Preferences by the web module; no heap-backed String is needed here.
constexpr char apSsid[] PROGMEM = "Zuno-Setup";
constexpr char apPassword[] PROGMEM = "12345678";
constexpr char mdnsName[] PROGMEM = "zuno";

// Weather defaults: Dhaka, Bangladesh.
constexpr float defaultTimezoneHours = 6.0f;
constexpr float defaultWeatherLat = 23.8103f;
constexpr float defaultWeatherLon = 90.4125f;

static_assert(oledWidth == 128 && oledHeight == 64, "Zuno v1.2 targets a 128x64 OLED.");
static_assert(oledTargetFps <= 30, "OLED FPS must stay within the ESP32 SH1106 budget.");
static_assert(batteryLowPercent <= 100, "Battery percentage must be 0..100.");
static_assert(selfDriveSlowSpeed <= selfDriveSpeed, "Slow speed must not exceed drive speed.");
static_assert(lookAroundMinMs <= lookAroundMaxMs, "Look-around timing range is invalid.");

}  // namespace ZunoConfig
