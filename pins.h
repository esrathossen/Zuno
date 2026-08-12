#pragma once

#include <Arduino.h>

namespace ZunoPins {

// I2C SH1106 OLED
constexpr uint8_t oledSda = 21;
constexpr uint8_t oledScl = 22;

// HC-SR04 ultrasonic sensor
constexpr uint8_t ultrasonicTrig = 27;
constexpr uint8_t ultrasonicEcho = 34;  // Input-only; use a 5V-to-3.3V divider.

// SG90 servo
constexpr uint8_t servoSignal = 13;

// DHT22
constexpr uint8_t dhtData = 32;

// L298N motor driver
constexpr uint8_t motorIn1 = 25;
constexpr uint8_t motorIn2 = 26;
constexpr uint8_t motorIn3 = 17;
constexpr uint8_t motorIn4 = 16;
constexpr uint8_t motorEna = 18;
constexpr uint8_t motorEnb = 19;

// Voltage sensor module, ADC1 input-only pin.
constexpr uint8_t batteryAdc = 35;

// Active buzzer
constexpr uint8_t buzzer = 23;

static_assert(ultrasonicEcho >= 34 && ultrasonicEcho <= 39, "Echo pin should stay on an input-only ADC pin.");
static_assert(batteryAdc >= 34 && batteryAdc <= 39, "Battery ADC should stay on an input-only ADC1 pin.");
static_assert(oledSda != oledScl, "I2C SDA/SCL pins must be different.");

}  // namespace ZunoPins