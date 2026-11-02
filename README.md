# Zuno

An ESP32-based robot car with an expressive OLED face, a self-hosted web control app, and games you can play on the robot itself. No cloud, no account, no separate mobile app: the ESP32 serves the control UI, and your phone's browser is the controller.

## Features

- **3 drive modes:** Self Driving (ultrasonic + servo obstacle avoidance), Manual (D-pad), Still. Self Driving auto-stops after 3 minutes.
- **Expressive OLED face:** Normal, Happy, Angry, Sleep, Love, Sad, Surprised. Idle look-around animation, a "searching" look during the obstacle scan, and a random emotion flash every 30 seconds.
- **Buzzer sounds:** a short chirp or beep for each expression.
- **Battery monitor:** percentage and voltage in the web app. At 20% or below, the OLED shows a low-battery icon, the face goes to Sleep, and the motors are cut.
- **Games:** Tic Tac Toe is drawn on the OLED with the phone as the controller. In Catch Me, Zuno evades you and the app shows a live timer.
- **Sensors:** DHT22 temperature and humidity (TEMP button) and live weather on the OLED (WEATHER button).
- **WiFi provisioning:** on first boot Zuno starts a setup hotspot. After that you can switch networks any time from Settings.
- **Settings in the app:** timezone, default weather location, WiFi.

## Hardware

| Part | Notes |
|---|---|
| ESP32 DevKitC (WROOM-32) | main controller |
| 1.3" I2C OLED (SH1106 128x64) | face and games |
| L298N motor driver + 2 DC motors | 2WD drive |
| HC-SR04 ultrasonic + SG90 servo | obstacle sensing and scan |
| DHT22 | temperature and humidity |
| Passive piezo buzzer | expression sounds |
| Voltage sensor module (25V, 5:1) | battery readout |
| 2x 3.7V cells (7.4V) + ON/OFF switch | external charger |

## Pin map

| Function | GPIO |
|---|---|
| OLED SDA / SCL | 21 / 22 |
| HC-SR04 TRIG / ECHO | 27 / 34 |
| Servo signal | 13 |
| DHT22 data | 32 |
| L298N IN1, IN2, IN3, IN4 | 25, 26, 17, 16 |
| L298N ENA / ENB | 18 / 19 |
| Buzzer | 4 |
| Voltage sensor "S" | 35 |

## Power

```
Battery 7.4V -> switch -> L298N (7.4V) -> motors
                          L298N 5V out -> ESP32 VIN
                          ESP32 3.3V   -> OLED, DHT22
HC-SR04 and servo: 5V.  All grounds shared.
```

Notes:
- HC-SR04 ECHO is 5V logic. Use a voltage divider before GPIO34.
- Connect the voltage sensor's "-" pin to ESP32 GND, and "S" to GPIO35.
- If the ESP32 resets when the motors start or the battery is low, power it from a separate 5V buck converter instead of the L298N regulator.

## Build and flash

1. Install these libraries in Arduino IDE: Adafruit GFX, Adafruit SH110X, ESP32Servo, DHT sensor library (with Adafruit Unified Sensor), ArduinoJson v6.
2. Board: **ESP32 Dev Module**.
3. Tools > Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**. The sketch is large and may not fit the default partition.
4. Put `zuno_v1.1.ino` and `web_ui.h` in the same sketch folder, then upload.

## First-time WiFi setup

1. Power on Zuno, then connect your phone to the WiFi `RobotCar-Setup` (password `12345678`).
2. Open `http://192.168.4.1/` in a browser.
3. Enter your home WiFi name and password, then tap **CONNECT WIFI**.
4. Zuno reboots and joins your network. Find its IP in the Serial Monitor, or try `http://robotcar.local/`.

To change networks later, use **Settings > WiFi** in the app.

## License

MIT. See [LICENSE](LICENSE).
