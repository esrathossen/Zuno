# Zuno v1.2 Wiring Guide

## Power

Use a common ground for every module.

- 2x18650 battery pack positive -> L298N +12V
- 2x18650 battery pack negative -> L298N GND
- L298N 5V output -> ESP32 VIN/5V
- L298N GND -> ESP32 GND
- ESP32 3.3V -> SH1106 OLED, DHT22, voltage sensor logic side
- Servo VCC -> 5V
- HC-SR04 VCC -> 5V

Check the L298N regulator temperature under load. If the ESP32 resets when motors start, use a separate 5V buck converter for the ESP32 and servo.

## ESP32 Pin Map

| Device | Signal | ESP32 Pin |
|---|---|---:|
| SH1106 OLED | SDA | GPIO21 |
| SH1106 OLED | SCL | GPIO22 |
| L298N Left | IN1 | GPIO26 |
| L298N Left | IN2 | GPIO27 |
| L298N Left | ENA | GPIO25 |
| L298N Right | IN3 | GPIO14 |
| L298N Right | IN4 | GPIO12 |
| L298N Right | ENB | GPIO13 |
| HC-SR04 | TRIG | GPIO5 |
| HC-SR04 | ECHO | GPIO18 |
| DHT22 | DATA | GPIO4 |
| Voltage Sensor | OUT | GPIO34 |
| SG90 Servo | SIGNAL | GPIO15 |
| Active Buzzer | SIGNAL | GPIO23 |

## Notes

- GPIO34 is input-only and ideal for battery ADC.
- If your HC-SR04 ECHO outputs 5V, add a resistor divider before GPIO18.
- Keep motor wires away from OLED I2C wires where possible.
- Place a 470uF or larger capacitor near the motor driver supply if motor noise causes resets.
