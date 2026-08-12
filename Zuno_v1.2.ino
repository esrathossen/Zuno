#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "zuno_core.h"

Adafruit_SH1106G display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
SensorState sensors;
RobotState robot;

static void bootLogo() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.setCursor(35, 16);
  display.print(F("Zuno"));
  display.setTextSize(1);
  display.setCursor(42, 40);
  display.print(F("v1.2"));
  display.display();
  delay(900);
}

static void readDht22Task() {
  static uint8_t data[5];
  memset(data, 0, sizeof(data));

  pinMode(PIN_DHT22, OUTPUT);
  digitalWrite(PIN_DHT22, LOW);
  delayMicroseconds(1100);
  digitalWrite(PIN_DHT22, HIGH);
  delayMicroseconds(30);
  pinMode(PIN_DHT22, INPUT_PULLUP);

  uint32_t start = micros();
  while (digitalRead(PIN_DHT22) == HIGH) {
    if ((uint32_t)(micros() - start) > 100) return;
  }
  start = micros();
  while (digitalRead(PIN_DHT22) == LOW) {
    if ((uint32_t)(micros() - start) > 100) return;
  }
  start = micros();
  while (digitalRead(PIN_DHT22) == HIGH) {
    if ((uint32_t)(micros() - start) > 100) return;
  }

  for (uint8_t i = 0; i < 40; i++) {
    start = micros();
    while (digitalRead(PIN_DHT22) == LOW) {
      if ((uint32_t)(micros() - start) > 120) return;
    }
    start = micros();
    while (digitalRead(PIN_DHT22) == HIGH) {
      if ((uint32_t)(micros() - start) > 120) return;
    }
    if ((uint32_t)(micros() - start) > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  const uint8_t checksum = data[0] + data[1] + data[2] + data[3];
  if (checksum != data[4]) {
    sensors.dhtValid = false;
    return;
  }

  const uint16_t rawHumidity = ((uint16_t)data[0] << 8) | data[1];
  const uint16_t rawTemp = (((uint16_t)(data[2] & 0x7F)) << 8) | data[3];
  sensors.humidity = rawHumidity / 10.0f;
  sensors.temperatureC = rawTemp / 10.0f;
  if (data[2] & 0x80) {
    sensors.temperatureC = -sensors.temperatureC;
  }
  sensors.dhtValid = true;
}

static void updateUptimeTask() {
  robot.uptimeMs = millis();
}

void setup() {
  Serial.begin(115200);
  randomSeed((uint32_t)esp_random());

  sensors.temperatureC = 0.0f;
  sensors.humidity = 0.0f;
  sensors.batteryVoltage = 0.0f;
  sensors.batteryPercent = 100;
  sensors.distanceCm = 250;
  sensors.obstacle = false;
  sensors.batteryLow = false;
  sensors.dhtValid = false;

  robot.mode = MODE_STILL;
  robot.face = FACE_HAPPY;
  robot.activeGame = GAME_NONE;
  robot.wifiReady = false;
  robot.motorsEnabled = true;
  robot.selfDriveScanning = false;
  robot.uptimeMs = 0;

  Wire.begin();
  display.begin(OLED_I2C_ADDR, true);
  display.setTextColor(SH110X_WHITE);
  bootLogo();

  LittleFS.begin(true);
  buzzerBegin();
  motorBegin();
  servoBegin();
  ultrasonicBegin();
  batteryBegin();
  faceBegin();
  gamesBegin();
  selfDriveBegin();
  webUIBegin();

  faceSetExpression(FACE_HAPPY, 1800);
  buzzerPlay(BUZZER_STARTUP);

  scheduler.add(faceUpdate, TASK_FACE_MS);
  scheduler.add(faceRender, TASK_OLED_MS);
  scheduler.add(motorUpdate, TASK_MOTOR_MS);
  scheduler.add(servoUpdate, TASK_SERVO_MS);
  scheduler.add(ultrasonicUpdate, TASK_ULTRASONIC_MS);
  scheduler.add(batteryUpdate, TASK_BATTERY_MS);
  scheduler.add(readDht22Task, TASK_DHT_MS);
  scheduler.add(webUIUpdate, TASK_WEB_MS);
  scheduler.add(gamesUpdate, TASK_GAME_MS);
  scheduler.add(selfDriveUpdate, TASK_SELFDRIVE_MS);
  scheduler.add(buzzerUpdate, TASK_BUZZER_MS);
  scheduler.add(updateUptimeTask, 1000UL);
}

void loop() {
  scheduler.update();
}
