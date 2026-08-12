# Zuno v1.2

Professional modular ESP32 robot firmware with SH1106 OLED face animation, self driving, Web dashboard, battery protection, buzzer sounds, and OLED games.

## Required Libraries

- ESP32 Arduino core
- Adafruit GFX
- Adafruit SH110X

The firmware avoids external Servo and DHT libraries. Servo PWM and DHT22 timing are handled by small local modules/code to keep flash usage low.

## Project Files

The root firmware folder is intentionally kept to 10 files for easier maintenance.

- `Zuno_v1.2.ino` - boot, globals, scheduler registration, DHT22 task
- `zuno_core.h` - config, pin map, shared types, prototypes, tiny inline helpers
- `zuno_core.cpp` - millis based cooperative scheduler
- `drivers.cpp` - motor, servo, ultrasonic, battery, buzzer modules
- `face.cpp` - procedural OLED face engine
- `selfdrive.cpp` - obstacle avoidance state machine
- `web_ui.cpp` - LittleFS WebServer routes
- `games.cpp` - Tic Tac Toe and Catch Me OLED games
- `data/` - LittleFS dashboard assets
- `WIRING_GUIDE.md` - wiring notes and pin map
- `data/` - LittleFS HTML, CSS, and JS dashboard

## Pin Mapping

| Module | ESP32 Pin |
|---|---:|
| L298N Left IN1 | GPIO26 |
| L298N Left IN2 | GPIO27 |
| L298N Left EN | GPIO25 |
| L298N Right IN1 | GPIO14 |
| L298N Right IN2 | GPIO12 |
| L298N Right EN | GPIO13 |
| HC-SR04 TRIG | GPIO5 |
| HC-SR04 ECHO | GPIO18 |
| DHT22 DATA | GPIO4 |
| Battery ADC | GPIO34 |
| SG90 Servo | GPIO15 |
| Buzzer | GPIO23 |
| OLED SDA | GPIO21 |
| OLED SCL | GPIO22 |

## LittleFS Upload

Upload the `data/` folder to LittleFS before using the dashboard. The ESP32 starts an access point:

- SSID: `Zuno-v1.2`
- Password: `zuno12345`
- Dashboard: `http://192.168.4.1/`

## Battery Calibration

Default calibration assumes a common 0-25V voltage sensor module:

- `VOLTAGE_DIVIDER_RATIO = 5.00`
- empty 2S pack: `6.40V`
- full 2S pack: `8.40V`

Measure the battery with a multimeter and adjust `zuno_core.h` if the dashboard voltage differs.

## Safety

When battery percentage is at or below 20 percent, Zuno stops motors, shows Sleep face, and plays a low battery tone.
