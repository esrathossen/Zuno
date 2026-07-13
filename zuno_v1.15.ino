/*
  ESP32 (classic WROOM-32 DevKitC) "Zuno" Robot Car - Web App Controlled
  -----------------------------------------------------------------------
  v1.1 changes: battery voltage sensor module (web app only, small OLED
  low-battery icon + forced Sleep + motors cut at/below 20%), a unified
  idle "personality" cycle shared by Self Driving/Still/Manual (30s Normal
  with a look-around eye animation, then 5s flashing a random emotion,
  repeat), a "searching" look-around face during the Self Driving obstacle
  scan, a Catch Me evasion game, a Tic Tac Toe game played ON THE OLED with
  the app as the controller, and mood sound effects on a piezo buzzer.

  MEMORY NOTE: this sketch is close to filling the default flash partition.
  If you get "sketch too big" at compile time, go to Tools > Partition
  Scheme and pick one with a bigger APP partition (e.g. "Huge APP (3MB No
  OTA/1MB SPIFFS)", or "Minimal SPIFFS"). Also double check Bluetooth is
  disabled in your board config if you're not using it.

  Board setting in Arduino IDE: Tools > Board > "ESP32 Dev Module"

  Features:
    - 2WD drive via L298N motor driver, HC-SR04 + SG90 servo for obstacle
      sensing, DHT22 for temp/humidity, a voltage-sensor-module battery
      readout (web app only), and a piezo buzzer for mood sound effects
    - 1.3" I2C OLED (SH1106 128x64):
        * Boot sequence: "Zuno" splash logo, a startup chirp, then a brief
          "Hi! I'm Zuno" greeting, before it settles into WiFi connect.
        * Persistent HUD: real-time clock (top-left), WiFi signal bars
          (top-right). Temp/humidity is app-only - tap TEMP for a 10s
          overlay. Battery % is app-only too - EXCEPT a small low-battery
          icon appears left side, just below the clock, at/below 20%.
        * 7 expressions: Normal, Happy, Angry, Sleep, Love, Sad, Surprised.
          No face buttons in the app - faces are fully automatic:
            - Battery at/below 20%: Sleep, always - overrides every mode
              and cuts the motors, until it's charged back up.
            - Self Driving: Angry once an obstacle is detected in the path
              (still moving/slowing), Sad once it actually stops for the
              obstacle. While the obstacle scan (servo sweep) runs, the
              face looks left/right/center in sync with the servo.
              Otherwise (path clear): the idle personality cycle below.
            - Still / Manual: the idle personality cycle below.
            - Catch Me: Happy while evading, Surprised once caught.
          IDLE PERSONALITY CYCLE (Self Driving-when-clear, Still, Manual):
          Normal face (with a self-directed look-around eye animation) for
          30s, then a random pick from {Happy, Angry, Love, Surprised, Sad}
          held for 5s, then back to Normal - repeating continuously.
          Receiving any command restarts the cycle fresh at Normal. Each
          expression change plays a short matching chirp/beep on the
          buzzer (see playFaceSound()) - silent for Normal/Blink.
    - 4 modes (Self Driving / Still / Manual selectable from the app's Mode
      row; Catch Me started from the Games panel):
        * SELF DRIVING - autonomous obstacle avoidance, speed 90. Auto-stops
          and drops to Still mode (Sleep face) after 3 continuous minutes.
        * STILL        - motors locked, speed 0
        * MANUAL       - drive with the app's D-pad (forward/back/left/right/stop)
        * CATCH ME     - wanders around evading with basic obstacle avoidance;
          "caught" once something (your hand) stays within CATCH_DISTANCE_CM
          of the front sensor for CATCH_GRACE_MS. The app shows a live timer.
    - Self-hosted web control panel (open http://<robot-ip>/ in Chrome)
    - A dedicated WELCOME/setup screen is shown automatically while the
      robot is in its setup hotspot (no saved WiFi yet) - enter a network
      name/password there to join it.
    - Once on a network, all secondary settings live behind a single
      SETTINGS button in the app to keep the main screen clean.
    - Weather (Open-Meteo via phone GPS) and a radar-animated obstacle Scan
      (5s cooldown between scans, can't interrupt/be interrupted by Weather
      or Temp overlays)
    - TEMP button: shows a large (text size 2) "TEMP - HUM%" overlay for 10s.
      Weather and Temp overlays never interrupt each other or the scan screen -
      if one is already showing, the newer request is politely declined.
    - SETTINGS panel in the app: timezone (UTC offset), default weather
      location (used when the phone doesn't share GPS), and a WiFi section
      to join *any* network at any time (not just during first-time setup) -
      all persisted on the robot, no reflash needed.
    - GAMES panel in the app:
        * Tic Tac Toe - played ON THE OLED. The app is just the controller
          (tap a cell to send /api/ttt/move); the board itself is drawn on
          the robot's screen, X/O and win/draw status included. Starting a
          game forces Still mode (no driving while the board is up).
        * Catch Me - see the Catch Me mode above; the app polls /api/status
          for the live "how long has it evaded you" timer.
    - BATTERY: a resistive voltage sensor module reads the 2S pack voltage
      via GPIO35. Shown ONLY in the web app (percent + voltage) - see
      handleStatus()'s battery_v/battery_pct fields. At/below 20%: OLED
      low-battery icon, forced Sleep face, AND the motors are cut in every
      mode until the pack is charged back up.

  REQUIRED LIBRARIES (Arduino Library Manager):
    - Adafruit GFX Library
    - Adafruit SH110X          (for 1.3" SH1106 OLED - most common)
        -> If your display is actually SSD1306, install "Adafruit SSD1306"
           instead and swap the two lines marked [OLED DRIVER] below.
    - ESP32Servo                (by Kevin Harrington / madhephaestus)
    - DHT sensor library        (by Adafruit) + Adafruit Unified Sensor
    - ArduinoJson                (by Benoit Blanchon, v6.x)
  (WebServer, Preferences, HTTPClient, WiFiClientSecure, ESPmDNS are bundled
   with the ESP32 board package - no separate install needed. tone()/
   noTone() for the buzzer and analogRead() for the battery are built in.)

  WIRING (classic ESP32 DevKitC, matches the labels printed on your board):
    OLED    SDA -> GPIO21   SCL -> GPIO22   VCC -> 3V3   GND -> GND
    HC-SR04 TRIG -> GPIO27  ECHO -> GPIO34 (input-only pin, needs a
             voltage divider - HC-SR04 echo is 5V logic, ESP32 is 3.3V!)
    SG90    Signal -> GPIO13  (external 5V supply for servo, share GND)
    DHT22   Data -> GPIO32 (10k pull-up to 3V3)
    L298N   IN1 -> GPIO25  IN2 -> GPIO26  IN3 -> GPIO17  IN4 -> GPIO16
            ENA -> GPIO18 (PWM)   ENB -> GPIO19 (PWM)
            L298N 5V/12V from battery pack, GND common with ESP32
    Buzzer  +  -> GPIO4     -  -> GND (passive piezo buzzer)
    Battery Voltage sensor module: screw terminal "VCC/GND" -> Batt+/Batt-
            (this is a passive divider, it draws its own sense power from
            the battery itself - no separate power pin needed). Header
            pin "S" -> GPIO35, header pin "-" -> ESP32 GND (common
            ground is required for an accurate reading). Ratio 5.0
            (matches "VCC<25V" modules like the one pictured) - verify
            against a multimeter reading and adjust BATTERY_DIVIDER_RATIO
            if yours differs.
    See the separate circuit diagram for the full 7.4V (2x 3.7V) power
    tree: battery -> switch -> L298N (7.4V) -> L298N's onboard 5V
    regulator -> ESP32 VIN -> ESP32's onboard 3.3V regulator -> sensors.

  Boots in STILL mode by default for safety. Select Self Driving or Manual
  from the app to move.

  FIRST-TIME WIFI SETUP:
    1. Flash this, power the robot on.
    2. On your phone, connect to WiFi "RobotCar-Setup" (password 12345678).
    3. Open http://192.168.4.1/ in Chrome - the app shows a WELCOME screen
       automatically. Enter your home WiFi name/password and tap Connect.
       The robot reboots and joins your home network.
    4. Reconnect your phone to your home WiFi. Check the Arduino Serial
       Monitor for the robot's new IP address (or try http://robotcar.local/).
    To switch to a *different* WiFi network later, no need to reset - just
    open SETTINGS in the app and use the WiFi section there.
*/

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>     // [OLED DRIVER] swap for <Adafruit_SSD1306.h> if using SSD1306
#include <ESP32Servo.h>
#include <DHT.h>
#include "web_ui.h"

// ---------------- WIFI FALLBACK DEFAULTS ----------------
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------------- PIN CONFIG (classic ESP32 DevKitC) ----------------
#define OLED_SDA      21
#define OLED_SCL      22
#define OLED_ADDR     0x3C

#define TRIG_PIN      27
#define ECHO_PIN      34   // input-only pin - fine for ECHO, don't use for outputs

#define SERVO_PIN     13

#define DHT_PIN       32
#define DHT_TYPE      DHT22

#define IN1 25
#define IN2 26
#define IN3 17
#define IN4 16
#define ENA 18
#define ENB 19

#define BATTERY_PIN   35   // ADC1 input-only pin, from the voltage sensor module's signal (S) output

// ---------------- TUNABLE PARAMS ----------------
const int   STOP_DISTANCE_CM   = 20;   // stop & scan below this (self-driving only)
const int   SLOW_DISTANCE_CM   = 40;   // slow down below this (self-driving only)
const int   SELF_DRIVE_SPEED      = 120;  // 0-255
const int   SELF_DRIVE_SLOW_SPEED = 50;  // 0-255
const int   MANUAL_SPEED           = 120; // 0-255, used by the app's D-pad
const unsigned long DHT_READ_INTERVAL = 4000;
const unsigned long WIFI_CHECK_INTERVAL = 5000;
const unsigned long HUD_REFRESH_INTERVAL = 1000; // how often the clock/temp HUD redraws
const unsigned long BATTERY_READ_INTERVAL = 5000;

// 2S Li-ion pack (2x 18650): 3.0V/cell empty, 4.2V/cell full.
// Using an off-the-shelf resistive "voltage sensor module" (S/+/- pins),
// most of which use a 30k/7.5k divider -> ratio exactly 5.0 (Vin = Vout x 5).
// Check your specific module's silkscreen/datasheet and adjust if different.
const float BATTERY_MIN_V = 6.0;
const float BATTERY_MAX_V = 8.4;
const float BATTERY_DIVIDER_RATIO = 5.0;
const float BATTERY_ADC_VREF = 3.3;
const int   BATTERY_ADC_MAX = 4095;
const int   BATTERY_LOW_PERCENT = 20; // OLED low-battery icon + forced Sleep face at/below this

// Idle "look around" eye animation for the Normal face - picks a new small
// eye offset every LOOK_AROUND_MIN..MAX_MS while Normal is on screen.
const unsigned long LOOK_AROUND_MIN_MS = 1500;
const unsigned long LOOK_AROUND_MAX_MS = 3500;

// ---------------- OBJECTS ----------------
Adafruit_SH1106G display(128, 64, &Wire, -1);  // [OLED DRIVER] Adafruit_SSD1306 display(128,64,&Wire,-1);
Servo scanServo;
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);
Preferences prefs;

// ---------------- STATE ----------------
enum RobotState { DRIVING, SLOWING, SCANNING };
RobotState state = DRIVING;

enum OperatingMode { MODE_SELF_DRIVE, MODE_STILL, MODE_MANUAL, MODE_CATCH_ME };
OperatingMode currentMode = MODE_STILL; // boots safe

enum FaceType { FACE_NORMAL, FACE_HAPPY, FACE_ANGRY, FACE_SLEEP, FACE_LOVE, FACE_SAD, FACE_SURPRISED, FACE_BLINK };
FaceType currentFace = FACE_NORMAL; // the face actually on screen right now (auto or override)

// A face picked via the API overrides the automatic mode-based face for
// 10 seconds, then automatic logic resumes. (No longer exposed as buttons
// in the app - faces in Still/Manual are automatic; see below.)
bool manualOverrideActive = false;
FaceType manualOverrideFace = FACE_NORMAL;
unsigned long manualOverrideUntil = 0;
const unsigned long MANUAL_FACE_DURATION = 10000;

// Idle "personality" cycle - active in all 3 driving modes whenever nothing
// more important is happening (no real obstacle event in Self Driving, no
// manual override, no low battery, no game). Mostly Normal (with the
// look-around animation), with a brief random-emotion flash every cycle.
// See computeIdleCycleFace() in the FACE / OLED section.
const unsigned long IDLE_NORMAL_MS = 30000; // 30s showing Normal
const unsigned long IDLE_RANDOM_MS = 5000;  //  5s flashing a random emotion
unsigned long idleCycleStart = 0;
bool idleInRandomPhase = false;
FaceType idleRandomFace = FACE_HAPPY;
unsigned long lastCommandTime = 0; // still used to gate registerActivity()'s cycle reset

// Scan button cooldown - stops rapid double-taps from queuing up
// back-to-back servo sweeps.
unsigned long lastScanTime = 0;
const unsigned long SCAN_COOLDOWN = 5000; // 5s between scans

// Self Driving safety timeout: after this long continuously self-driving,
// stop and drop back to Still mode with a Sleep face until the next command.
unsigned long selfDriveSince = 0;
const unsigned long SELF_DRIVE_AUTO_STOP = 3UL * 60UL * 1000UL; // 3 minutes
bool forceSleepAfterAutoStop = false;

// Catch Me game: how close counts as "caught" and evasive-move cadence
const int   CATCH_DISTANCE_CM   = 8;
const unsigned long CATCH_GRACE_MS    = 400;  // must stay this close briefly, avoids false triggers
const unsigned long EVADE_INTERVAL_MS = 900;  // how often it picks a new evasive move

// ---------------- BUZZER (mood sound effects) ----------------
#define BUZZER_PIN 4
struct SoundStep { int freq; unsigned long durationMs; };
SoundStep soundQueue[4];
int soundQueueLen = 0;
int soundStepIndex = -1; // -1 = not currently playing anything
unsigned long soundStepUntil = 0;

// ---------------- TIC TAC TOE (played on the OLED, app is the controller) ----------------
bool tttActive = false;
char tttBoard[9]; // ' ' empty, 'X' player, 'O' Zuno
bool tttPlayerTurn = true;
String tttResult = ""; // "", "player", "robot", "draw"

// Configurable from the app's Settings panel (persisted in Preferences).
// Defaults match the original hardcoded values (Dhaka, GMT+6).
float gmtOffsetHours   = 6.0;
float defaultWeatherLat = 23.8103;
float defaultWeatherLon = 90.4125;

unsigned long lastDHTRead = 0;
bool wifiConnected = false;
bool isAPMode = false;
String currentSSID = "";
unsigned long lastWifiCheck = 0;
float lastTemp = NAN, lastHum = NAN;
long lastDistance = -1;
long lastScanLeft = -1, lastScanRight = -1;

bool timeSynced = false;
bool weatherSynced = false;
float weatherTemp = NAN;
String weatherDesc = "";

// ---------------- BATTERY ----------------
unsigned long lastBatteryRead = 0;
float lastBatteryVoltage = NAN;
int lastBatteryPercent = -1; // -1 = no reading yet

// ---------------- CATCH ME GAME ----------------
enum CatchState { CATCH_IDLE, CATCH_RUNNING, CATCH_CAUGHT };
CatchState catchState = CATCH_IDLE;
unsigned long catchStartTime = 0;
unsigned long catchCaughtElapsedMs = 0;
unsigned long catchCloseSince = 0;
unsigned long lastEvadeChange = 0;

// ---------------- IDLE LOOK-AROUND (Normal face) ----------------
int normalEyeOffsetX = 0;
int normalEyeOffsetY = 0;
unsigned long nextLookChange = 0;

unsigned long blinkTimer = 0;
unsigned long lastHudRefresh = 0;
bool objectDetected = false;       // true while self-driving has actually stopped for an obstacle (Sad)
bool pathObstacleDetected = false; // true while an obstacle is in path but still moving/slowing (Angry)

// Temporary OLED overlay (used by Show Weather / scan animation+status).
// While active, it takes over the screen and suppresses face/HUD redraws.
bool overlayActive = false;
unsigned long overlayUntil = 0;
String overlayLine1 = "", overlayLine2 = "";

// =========================================================
void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(OLED_ADDR, true);       // [OLED DRIVER] display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  showBootLogo();     // clears + draws + display()s once - no need for a blank flash first
  showBootGreeting(); // brief "Hi!" greeting before the robot settles into its normal face

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(BATTERY_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Little startup chirp (blocking here is fine - nothing else running yet)
  tone(BUZZER_PIN, 1000, 100); delay(120);
  tone(BUZZER_PIN, 1500, 150); delay(150);
  noTone(BUZZER_PIN);

  ESP32PWM::allocateTimer(0);
  scanServo.setPeriodHertz(50);
  scanServo.attach(SERVO_PIN, 500, 2400);
  scanServo.write(90); // facing forward

  dht.begin();

  connectWiFi();
  setupServer();
  randomSeed(esp_random());
  lastCommandTime = millis();
  idleCycleStart = millis();
  selfDriveSince = millis();
  readBattery(); // seed a reading immediately instead of waiting for the first interval
  memset(tttBoard, ' ', 9);

  drawFace(currentFace);
  delay(300);
}

// =========================================================
void loop() {
  server.handleClient();

  if (overlayActive && millis() > overlayUntil) {
    overlayActive = false;
    drawFace(currentFace);
  }

  // Tic Tac Toe owns the OLED entirely while a game is in progress - no
  // driving logic, no face, just wait for moves via /api/ttt/move.
  if (tttActive) {
    stopCar();
    readBattery();
    handleBuzzer();
    delay(30);
    return;
  }

  long distance = getDistanceCM();
  lastDistance = distance;

  bool batteryCritical = (lastBatteryPercent >= 0 && lastBatteryPercent <= BATTERY_LOW_PERCENT);

  if (batteryCritical) {
    // Battery protection: motors stay off in every mode until it's charged
    // back up. Face is forced to Sleep too (see computeAutoFace()).
    stopCar();
    state = DRIVING; // so self-driving resumes cleanly once charged back up
  } else if (currentMode == MODE_SELF_DRIVE && millis() - selfDriveSince >= SELF_DRIVE_AUTO_STOP) {
    // Safety timeout: after 3 continuous minutes of Self Driving, stop and
    // fall back to Still mode with a Sleep face until the next command.
    stopCar();
    currentMode = MODE_STILL;
    forceSleepAfterAutoStop = true;
  } else if (currentMode == MODE_SELF_DRIVE) {
    switch (state) {
      case DRIVING:
        if (distance > 0 && distance < STOP_DISTANCE_CM) {
          stopCar();
          objectDetected = true;       // actually stopped for an obstacle -> Sad
          pathObstacleDetected = true;
          if (!overlayActive) drawFace(FACE_SAD); // immediate feedback
          state = SCANNING;
        } else if (distance > 0 && distance < SLOW_DISTANCE_CM) {
          driveForward(SELF_DRIVE_SLOW_SPEED);
          pathObstacleDetected = true; // obstacle detected in path, still moving -> Angry
          if (!overlayActive) drawFace(FACE_ANGRY); // immediate feedback
          state = SLOWING;
        } else {
          driveForward(SELF_DRIVE_SPEED);
          pathObstacleDetected = false;
        }
        break;

      case SLOWING:
        if (distance > 0 && distance < STOP_DISTANCE_CM) {
          stopCar();
          objectDetected = true;
          if (!overlayActive) drawFace(FACE_SAD);
          state = SCANNING;
        } else if (distance >= SLOW_DISTANCE_CM || distance <= 0) {
          pathObstacleDetected = false;
          state = DRIVING;
        }
        break;

      case SCANNING: {
        stopCar();
        delay(150);

        if (!overlayActive) drawLookAroundFace(-14); // looking left, searching
        int leftDist  = lookAndMeasure(160);
        if (!overlayActive) drawLookAroundFace(14);  // looking right, searching
        int rightDist = lookAndMeasure(20);
        scanServo.write(90);
        if (!overlayActive) drawLookAroundFace(0);   // back to center, still puzzling
        delay(200);

        if (leftDist <= 0) leftDist = 0;
        if (rightDist <= 0) rightDist = 0;

        if (max(leftDist, rightDist) < STOP_DISTANCE_CM) {
          driveBackward(SELF_DRIVE_SLOW_SPEED);
          delay(500);
          stopCar();
        } else if (leftDist > rightDist) {
          turnLeft(SELF_DRIVE_SLOW_SPEED);
          delay(450);
        } else {
          turnRight(SELF_DRIVE_SLOW_SPEED);
          delay(450);
        }
        stopCar();
        objectDetected = false;
        pathObstacleDetected = false;
        state = DRIVING;
        break;
      }

      default:
        state = DRIVING;
        break;
    }
  } else if (currentMode == MODE_STILL) {
    stopCar();
  } else if (currentMode == MODE_CATCH_ME) {
    handleCatchMe(distance);
  }
  // MODE_MANUAL: no autonomous logic here - motors stay in whatever state
  // the last /api/command manual_* call put them in, until changed again.

  // Resolve which face should be showing: a manual override (from the API)
  // wins for 10 seconds, otherwise it's driven by mode/state.
  if (manualOverrideActive && millis() > manualOverrideUntil) {
    manualOverrideActive = false;
  }
  FaceType desiredFace = manualOverrideActive ? manualOverrideFace : computeAutoFace();
  bool faceChanged = (desiredFace != currentFace);
  currentFace = desiredFace;

  handleBlinking();
  handleDHT();
  handleWiFiStatus();
  readBattery();
  handleIdleLookAround();
  handleBuzzer();

  if (!overlayActive && faceChanged) {
    playFaceSound(desiredFace);
  }
  if (!overlayActive && (faceChanged || millis() - lastHudRefresh >= HUD_REFRESH_INTERVAL)) {
    lastHudRefresh = millis();
    drawFace(currentFace); // redraws face + refreshes clock/temp/humidity HUD
  }

  delay(30);
}

// Catch Me game: wanders around evading with basic obstacle avoidance,
// "caught" once something stays within CATCH_DISTANCE_CM for CATCH_GRACE_MS
// (i.e. a hand closing in on the sensor).
void handleCatchMe(long distance) {
  if (catchState != CATCH_RUNNING) { stopCar(); return; }

  if (distance > 0 && distance < CATCH_DISTANCE_CM) {
    if (catchCloseSince == 0) catchCloseSince = millis();
    if (millis() - catchCloseSince >= CATCH_GRACE_MS) {
      stopCar();
      catchState = CATCH_CAUGHT;
      catchCaughtElapsedMs = millis() - catchStartTime;
      setManualOverrideFace(FACE_SURPRISED);
      playFaceSound(FACE_SURPRISED);
    }
    return;
  }
  catchCloseSince = 0;

  if (distance > 0 && distance < STOP_DISTANCE_CM) {
    // Obstacle/wall, not necessarily a hand - quick dodge and re-roll evasive move.
    stopCar();
    turnLeft(SELF_DRIVE_SPEED);
    delay(300);
    stopCar();
    lastEvadeChange = 0; // force a fresh move choice below
  }

  if (millis() - lastEvadeChange >= EVADE_INTERVAL_MS) {
    lastEvadeChange = millis();
    int r = random(0, 4);
    if (r == 0)      driveForward(SELF_DRIVE_SPEED);
    else if (r == 1) turnLeft(SELF_DRIVE_SPEED);
    else if (r == 2) turnRight(SELF_DRIVE_SPEED);
    else             driveBackward(SELF_DRIVE_SLOW_SPEED);
  }
}

// ---------------- MOTOR CONTROL ----------------
void driveForward(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void driveBackward(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnLeft(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnRight(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ---------------- ULTRASONIC ----------------
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout (~4m)
  if (duration == 0) return -1; // no echo / out of range
  return duration * 0.0343 / 2;
}

int lookAndMeasure(int angle) {
  scanServo.write(angle);
  delay(350); // let servo settle
  return getDistanceCM();
}

// Used by the app's "Scan" button: sweeps the servo end-to-end, drawing a
// radar animation on the OLED, then shows a result screen for a moment.
void performScan() {
  stopCar();
  overlayActive = true; // block face/HUD redraws for the whole sweep+result

  lastScanLeft = -1;
  lastScanRight = -1;

  for (int angle = 160; angle >= 20; angle -= 20) {
    scanServo.write(angle);
    delay(150);
    long d = getDistanceCM();
    drawRadarFrame(angle, d);
    if (angle == 160) lastScanLeft = d;
    if (angle == 20)  lastScanRight = d;
  }
  scanServo.write(90);
  delay(150);

  showScanStatusScreen(lastScanLeft, lastScanRight);
  delay(1500);

  overlayActive = false;
  drawFace(currentFace);
}

// ---------------- DHT22 ----------------
void handleDHT() {
  if (millis() - lastDHTRead < DHT_READ_INTERVAL) return;
  lastDHTRead = millis();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    lastTemp = t;
    lastHum = h;
  }
}

// ---------------- BATTERY ----------------
// Shown only in the web app (see /api/status battery_v / battery_pct).
// The OLED stays silent about it except for a small low-battery icon and
// forcing the Sleep face once it drops to/below BATTERY_LOW_PERCENT.
void readBattery() {
  if (millis() - lastBatteryRead < BATTERY_READ_INTERVAL) return;
  lastBatteryRead = millis();

  int raw = analogRead(BATTERY_PIN);
  float adcVoltage = (raw / (float)BATTERY_ADC_MAX) * BATTERY_ADC_VREF;
  lastBatteryVoltage = adcVoltage * BATTERY_DIVIDER_RATIO;

  float pct = (lastBatteryVoltage - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V) * 100.0;
  lastBatteryPercent = constrain((int)pct, 0, 100);
}

// ---------------- BUZZER (mood sound effects) ----------------
// Queues up to 4 short notes to play back-to-back without blocking the
// main loop() - see handleBuzzer(), called every loop iteration.
void queueSound(const SoundStep* steps, int len) {
  if (len > 4) len = 4;
  for (int i = 0; i < len; i++) soundQueue[i] = steps[i];
  soundQueueLen = len;
  soundStepIndex = 0;
  tone(BUZZER_PIN, soundQueue[0].freq);
  soundStepUntil = millis() + soundQueue[0].durationMs;
}

void handleBuzzer() {
  if (soundStepIndex < 0) return; // nothing queued
  if (millis() < soundStepUntil) return;

  soundStepIndex++;
  if (soundStepIndex >= soundQueueLen) {
    noTone(BUZZER_PIN);
    soundStepIndex = -1;
    return;
  }
  tone(BUZZER_PIN, soundQueue[soundStepIndex].freq);
  soundStepUntil = millis() + soundQueue[soundStepIndex].durationMs;
}

// Small chirp/beep per expression, triggered once whenever the face
// actually changes (see loop()) - not on every redraw.
void playFaceSound(FaceType f) {
  static const SoundStep happy[]     = { {1200, 80}, {1600, 100} };
  static const SoundStep sad[]       = { {500, 150}, {300, 200} };
  static const SoundStep angry[]     = { {200, 120}, {150, 120} };
  static const SoundStep love[]      = { {1400, 70}, {1800, 70}, {2200, 100} };
  static const SoundStep surprised[] = { {2000, 60}, {2400, 90} };
  static const SoundStep sleep[]     = { {400, 200} };

  switch (f) {
    case FACE_HAPPY:     queueSound(happy, 2);     break;
    case FACE_SAD:       queueSound(sad, 2);       break;
    case FACE_ANGRY:     queueSound(angry, 2);     break;
    case FACE_LOVE:      queueSound(love, 3);      break;
    case FACE_SURPRISED: queueSound(surprised, 2); break;
    case FACE_SLEEP:     queueSound(sleep, 1);     break;
    default: break; // Normal/Blink stay silent
  }
}

// ---------------- WIFI ----------------
void connectWiFi() {
  prefs.begin("robot", false);
  String ssid = prefs.getString("ssid", WIFI_SSID);
  String pass = prefs.getString("pass", WIFI_PASSWORD);
  gmtOffsetHours   = prefs.getFloat("gmt_offset", 6.0);
  defaultWeatherLat = prefs.getFloat("def_lat", 23.8103);
  defaultWeatherLon = prefs.getFloat("def_lon", 90.4125);

  if (ssid.length() > 0 && ssid != "YOUR_WIFI_NAME") {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("Connecting to WiFi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      Serial.print(".");
      delay(300);
    }
    Serial.println();
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    isAPMode = false;
    currentSSID = ssid;
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
    if (MDNS.begin("robotcar")) {
      Serial.println("mDNS ready: try http://robotcar.local/");
    }
    configTime((long)(gmtOffsetHours * 3600), 0, "pool.ntp.org", "time.google.com"); // configurable via app Settings
  } else {
    Serial.println("Could not join saved WiFi. Starting setup hotspot.");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("RobotCar-Setup", "12345678");
    isAPMode = true;
    wifiConnected = false;
    currentSSID = "";
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  }
}

void handleWiFiStatus() {
  if (millis() - lastWifiCheck < WIFI_CHECK_INTERVAL) return;
  lastWifiCheck = millis();
  if (!isAPMode) {
    wifiConnected = (WiFi.status() == WL_CONNECTED);
  }
}

// ---------------- TIME ----------------
// Full timestamp for the /api/status card.
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 100)) return "Not synced";
  timeSynced = true;
  char buf[24];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

// Short "HH:MM" used in the persistent OLED HUD.
String getHudTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 50)) return "--:--";
  timeSynced = true;
  char buf[6];
  strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
  return String(buf);
}

// ---------------- WEATHER ----------------
String wmoToDesc(int code) {
  if (code == 0) return "Clear sky";
  if (code <= 3) return "Partly cloudy";
  if (code == 45 || code == 48) return "Fog";
  if (code >= 51 && code <= 57) return "Drizzle";
  if (code >= 61 && code <= 67) return "Rain";
  if (code >= 71 && code <= 77) return "Snow";
  if (code >= 80 && code <= 82) return "Rain showers";
  if (code >= 95) return "Thunderstorm";
  return "Unknown";
}

bool fetchWeather(float lat, float lon) {
  if (!wifiConnected) return false;

  WiFiClientSecure client;
  client.setInsecure(); // skip cert validation - fine for a hobby project
  HTTPClient https;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 4) +
               "&longitude=" + String(lon, 4) + "&current_weather=true";

  bool ok = false;
  if (https.begin(client, url)) {
    int code = https.GET();
    if (code == 200) {
      String payload = https.getString();
      StaticJsonDocument<1024> wdoc;
      if (!deserializeJson(wdoc, payload)) {
        weatherTemp = wdoc["current_weather"]["temperature"] | NAN;
        int wcode = wdoc["current_weather"]["weathercode"] | -1;
        weatherDesc = wmoToDesc(wcode);
        weatherSynced = true;
        ok = true;
      }
    }
    https.end();
  }
  return ok;
}

// ---------------- WEB SERVER ----------------
void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.send(200, "text/html", INDEX_HTML);
}

String modeToString(OperatingMode m) {
  if (m == MODE_SELF_DRIVE) return "self_driving";
  if (m == MODE_MANUAL) return "manual";
  if (m == MODE_CATCH_ME) return "catch_me";
  return "still";
}

// Automatic face for the current mode/state, used whenever no manual
// override is active. Low battery takes priority over everything else.
FaceType computeAutoFace() {
  if (lastBatteryPercent >= 0 && lastBatteryPercent <= BATTERY_LOW_PERCENT) {
    return FACE_SLEEP; // battery protection - always wins, regardless of mode
  }

  if (currentMode == MODE_SELF_DRIVE) {
    if (objectDetected) return FACE_SAD;         // actually stopped for an obstacle
    if (pathObstacleDetected) return FACE_ANGRY;  // obstacle detected in path, still moving/slowing
    return computeIdleCycleFace();                // clear path - idle personality cycle
  } else if (currentMode == MODE_CATCH_ME) {
    return (catchState == CATCH_CAUGHT) ? FACE_SURPRISED : FACE_HAPPY;
  }

  if (forceSleepAfterAutoStop) return FACE_SLEEP; // just auto-stopped from a 3-min Self Driving run
  return computeIdleCycleFace(); // MODE_STILL and MODE_MANUAL: same idle personality cycle
}

// Idle personality cycle, shared by Self Driving (when clear), Still, and
// Manual: mostly Normal (with the look-around animation) for
// IDLE_NORMAL_MS, then a brief random emotion for IDLE_RANDOM_MS, on
// repeat. The random face is rolled once per entry into the random phase
// (not every call) so it holds steady for the full 5s window.
FaceType computeIdleCycleFace() {
  unsigned long cycleLen = IDLE_NORMAL_MS + IDLE_RANDOM_MS;
  unsigned long cyclePos = (millis() - idleCycleStart) % cycleLen;
  bool nowRandomPhase = (cyclePos >= IDLE_NORMAL_MS);

  if (nowRandomPhase && !idleInRandomPhase) {
    const FaceType options[] = { FACE_HAPPY, FACE_ANGRY, FACE_LOVE, FACE_SURPRISED, FACE_SAD };
    idleRandomFace = options[random(0, 5)];
  }
  idleInRandomPhase = nowRandomPhase;
  return nowRandomPhase ? idleRandomFace : FACE_NORMAL;
}

// Called whenever the robot receives any command from the app. Restarts
// the idle personality cycle fresh at Normal, and clears the "just
// auto-stopped from Self Driving" Sleep override.
void registerActivity() {
  lastCommandTime = millis();
  idleCycleStart = millis();
  idleInRandomPhase = false;
  forceSleepAfterAutoStop = false;
}

// While the Normal face is idle on screen, periodically nudges the eyes
// to a new small offset (or back to center) so the robot looks like it's
// glancing around by itself instead of just staring blankly. Harmless to
// run even when a different face is showing - drawFace() only applies
// this offset in the FACE_NORMAL case.
void handleIdleLookAround() {
  if (millis() < nextLookChange) return;
  nextLookChange = millis() + random(LOOK_AROUND_MIN_MS, LOOK_AROUND_MAX_MS);

  int r = random(0, 5); // weight one option toward "look straight ahead"
  if (r == 0) {
    normalEyeOffsetX = 0;
    normalEyeOffsetY = 0;
  } else {
    normalEyeOffsetX = random(-4, 5); // -4..4 px
    normalEyeOffsetY = random(-2, 3); // -2..2 px
  }
}

String faceToString(FaceType f) {
  switch (f) {
    case FACE_HAPPY:     return "happy";
    case FACE_ANGRY:     return "angry";
    case FACE_SLEEP:     return "sleep";
    case FACE_LOVE:      return "love";
    case FACE_SAD:       return "sad";
    case FACE_SURPRISED: return "surprised";
    default:             return "normal";
  }
}

void handleStatus() {
  addCORS();
  StaticJsonDocument<768> doc;
  doc["mode"] = modeToString(currentMode);
  doc["face"] = faceToString(currentFace);
  doc["distance_cm"] = lastDistance;
  doc["temp_c"] = lastTemp;
  doc["humidity"] = lastHum;
  doc["wifi_connected"] = wifiConnected;
  doc["ap_mode"] = isAPMode;
  doc["ip"] = isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  doc["ssid"] = currentSSID;
  doc["time"] = getTimeString();
  doc["time_synced"] = timeSynced;
  doc["battery_v"] = lastBatteryVoltage;
  doc["battery_pct"] = lastBatteryPercent;
  JsonObject w = doc.createNestedObject("weather");
  w["synced"] = weatherSynced;
  w["temp_c"] = weatherTemp;
  w["desc"] = weatherDesc;

  JsonObject c = doc.createNestedObject("catch_me");
  if (catchState == CATCH_IDLE) {
    c["state"] = "idle";
  } else if (catchState == CATCH_RUNNING) {
    c["state"] = "running";
    c["elapsed_sec"] = (millis() - catchStartTime) / 1000;
  } else {
    c["state"] = "caught";
    c["elapsed_sec"] = catchCaughtElapsedMs / 1000;
  }

  JsonObject t = doc.createNestedObject("ttt");
  t["active"] = tttActive;
  t["player_turn"] = tttPlayerTurn;
  t["result"] = tttResult;
  String boardStr = "";
  for (int i = 0; i < 9; i++) boardStr += tttBoard[i];
  t["board"] = boardStr;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleCommand() {
  addCORS();
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"No body\"}");
    return;
  }
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Bad JSON\"}");
    return;
  }
  String cmd = doc["cmd"] | "";
  String msg = "OK";
  registerActivity(); // any command counts as activity - resets standby/sleep timers, rerolls idle face

  // ---- Modes ----
  if (cmd == "mode_self_driving") {
    currentMode = MODE_SELF_DRIVE;
    state = DRIVING;
    selfDriveSince = millis();
    catchState = CATCH_IDLE;
    msg = "Self Driving mode";
  } else if (cmd == "mode_still") {
    currentMode = MODE_STILL;
    stopCar();
    catchState = CATCH_IDLE;
    msg = "Still mode";
  } else if (cmd == "mode_manual") {
    currentMode = MODE_MANUAL;
    stopCar();
    catchState = CATCH_IDLE;
    msg = "Manual mode - use the D-pad";
  } else if (cmd == "mode_catch_me") {
    currentMode = MODE_CATCH_ME;
    catchState = CATCH_RUNNING;
    catchStartTime = millis();
    catchCloseSince = 0;
    lastEvadeChange = 0;
    msg = "Catch Me started - go get it!";

  // ---- Manual D-pad (only effective in Manual mode) ----
  } else if (cmd == "manual_forward" || cmd == "manual_backward" ||
             cmd == "manual_left" || cmd == "manual_right" || cmd == "manual_stop") {
    if (currentMode != MODE_MANUAL) {
      server.send(400, "application/json", "{\"ok\":false,\"message\":\"Switch to Manual mode first\"}");
      return;
    }
    if (cmd == "manual_forward")      { driveForward(MANUAL_SPEED);   msg = "Moving forward"; }
    else if (cmd == "manual_backward"){ driveBackward(MANUAL_SPEED);  msg = "Moving backward"; }
    else if (cmd == "manual_left")    { turnLeft(MANUAL_SPEED);       msg = "Turning left"; }
    else if (cmd == "manual_right")   { turnRight(MANUAL_SPEED);      msg = "Turning right"; }
    else                                { stopCar();                   msg = "Stopped"; }

  // ---- Face selection (no longer exposed as buttons in the app, but kept
  // available over the API - overrides the automatic face for 10s) ----
  } else if (cmd == "face_normal")     { setManualOverrideFace(FACE_NORMAL);    msg = "Face: Normal (10s)"; }
  else if (cmd == "face_happy")        { setManualOverrideFace(FACE_HAPPY);     msg = "Face: Happy (10s)"; }
  else if (cmd == "face_angry")        { setManualOverrideFace(FACE_ANGRY);     msg = "Face: Angry (10s)"; }
  else if (cmd == "face_sleep")        { setManualOverrideFace(FACE_SLEEP);     msg = "Face: Sleep (10s)"; }
  else if (cmd == "face_love")         { setManualOverrideFace(FACE_LOVE);      msg = "Face: Love (10s)"; }
  else if (cmd == "face_sad")          { setManualOverrideFace(FACE_SAD);       msg = "Face: Sad (10s)"; }
  else if (cmd == "face_surprised")    { setManualOverrideFace(FACE_SURPRISED); msg = "Face: Surprised (10s)"; }

  // ---- Show temp/humidity overlay (10s, large text) ----
  else if (cmd == "show_temp") {
    if (overlayActive) {
      msg = "Display busy, try again in a moment";
    } else {
      showTempOverlay();
      msg = "Showing temperature (10s)";
    }
  }

  // ---- Scan ----
  else if (cmd == "scan") {
    if (overlayActive) {
      msg = "Display busy, try again in a moment";
    } else if (millis() - lastScanTime < SCAN_COOLDOWN) {
      msg = "Scan cooling down, try again shortly";
    } else {
      lastScanTime = millis();
      performScan();
      msg = "Scan complete: L=" + String(lastScanLeft) + "cm  R=" + String(lastScanRight) + "cm";
    }
  } else {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Unknown command\"}");
    return;
  }

  StaticJsonDocument<256> res;
  res["ok"] = true;
  res["message"] = msg;
  String out;
  serializeJson(res, out);
  server.send(200, "application/json", out);
}

void handleWifiSet() {
  addCORS();
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"No body\"}");
    return;
  }
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Bad JSON\"}");
    return;
  }
  String ssid = doc["ssid"] | "";
  String pass = doc["password"] | "";
  if (ssid.length() == 0) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"SSID required\"}");
    return;
  }

  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);

  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Saved. Robot is rebooting to connect...\"}");
  delay(800);
  ESP.restart();
}

void handleWeatherSync() {
  addCORS();
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"No body\"}");
    return;
  }
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Bad JSON\"}");
    return;
  }
  float lat = doc["lat"] | defaultWeatherLat; // configurable default, used only if the phone doesn't send GPS
  float lon = doc["lon"] | defaultWeatherLon;

  bool ok = fetchWeather(lat, lon);
  String message;
  if (ok) {
    if (overlayActive) {
      // Don't interrupt a Temp overlay (or scan screen) already on screen.
      message = "Weather synced (display busy, try again)";
    } else {
      String tempStr = String(weatherTemp, 1) + " C";
      showOverlay(weatherDesc, tempStr, 6000);
      message = "Weather synced and shown on display";
    }
  } else {
    message = "Weather fetch failed (needs internet)";
  }

  StaticJsonDocument<128> res;
  res["ok"] = ok;
  res["message"] = message;
  String out;
  serializeJson(res, out);
  server.send(200, "application/json", out);
}

void handleWifiReset() {
  addCORS();
  prefs.remove("ssid");
  prefs.remove("pass");
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"WiFi reset. Rebooting into setup hotspot...\"}");
  delay(800);
  ESP.restart();
}

// ---------------- SETTINGS (timezone + default weather location) ----------------
void handleGetSettings() {
  addCORS();
  StaticJsonDocument<128> doc;
  doc["gmt_offset"] = gmtOffsetHours;
  doc["lat"] = defaultWeatherLat;
  doc["lon"] = defaultWeatherLon;
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleSetSettings() {
  addCORS();
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"No body\"}");
    return;
  }
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Bad JSON\"}");
    return;
  }

  if (doc.containsKey("gmt_offset")) {
    gmtOffsetHours = doc["gmt_offset"] | gmtOffsetHours;
    prefs.putFloat("gmt_offset", gmtOffsetHours);
    if (wifiConnected) {
      configTime((long)(gmtOffsetHours * 3600), 0, "pool.ntp.org", "time.google.com");
    }
  }
  if (doc.containsKey("lat")) {
    defaultWeatherLat = doc["lat"] | defaultWeatherLat;
    prefs.putFloat("def_lat", defaultWeatherLat);
  }
  if (doc.containsKey("lon")) {
    defaultWeatherLon = doc["lon"] | defaultWeatherLon;
    prefs.putFloat("def_lon", defaultWeatherLon);
  }

  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Settings saved\"}");
}

// ---------------- TIC TAC TOE (played on the OLED, app is the controller) ----------------
const int TTT_LINES[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};

// Returns 'X', 'O', 'D' (draw), or 0 (game still going).
char tttCheckWinner() {
  for (int i = 0; i < 8; i++) {
    char a = tttBoard[TTT_LINES[i][0]], b = tttBoard[TTT_LINES[i][1]], c = tttBoard[TTT_LINES[i][2]];
    if (a != ' ' && a == b && b == c) return a;
  }
  for (int i = 0; i < 9; i++) if (tttBoard[i] == ' ') return 0;
  return 'D';
}

// Simple heuristic AI for 'O': win if possible, else block 'X', else
// center, else a random corner, else any random empty cell.
int tttAiMove() {
  for (int i = 0; i < 8; i++) {
    int oCount = 0, emptyIdx = -1;
    for (int j = 0; j < 3; j++) {
      int cell = TTT_LINES[i][j];
      if (tttBoard[cell] == 'O') oCount++;
      else if (tttBoard[cell] == ' ') emptyIdx = cell;
    }
    if (oCount == 2 && emptyIdx != -1) return emptyIdx;
  }
  for (int i = 0; i < 8; i++) {
    int xCount = 0, emptyIdx = -1;
    for (int j = 0; j < 3; j++) {
      int cell = TTT_LINES[i][j];
      if (tttBoard[cell] == 'X') xCount++;
      else if (tttBoard[cell] == ' ') emptyIdx = cell;
    }
    if (xCount == 2 && emptyIdx != -1) return emptyIdx;
  }
  if (tttBoard[4] == ' ') return 4;
  int corners[4] = {0, 2, 6, 8}, avail[4], n = 0;
  for (int i = 0; i < 4; i++) if (tttBoard[corners[i]] == ' ') avail[n++] = corners[i];
  if (n > 0) return avail[random(0, n)];
  int empty[9], m = 0;
  for (int i = 0; i < 9; i++) if (tttBoard[i] == ' ') empty[m++] = i;
  return (m > 0) ? empty[random(0, m)] : -1;
}

// Renders the 3x3 board on the OLED - takes over the whole screen while
// a game is active (see tttActive in loop()).
void drawTttBoard() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(2, 0);
  if (tttResult == "player")     display.print("YOU WIN!");
  else if (tttResult == "robot") display.print("ZUNO WINS");
  else if (tttResult == "draw")  display.print("DRAW");
  else display.print(tttPlayerTurn ? "YOUR TURN" : "ZUNO'S TURN");
  drawWifiIcon(wifiConnected);

  int gx = 34, gy = 14, cw = 20, ch = 17;
  display.drawFastVLine(gx + cw, gy, ch * 3, SH110X_WHITE);
  display.drawFastVLine(gx + cw * 2, gy, ch * 3, SH110X_WHITE);
  display.drawFastHLine(gx, gy + ch, cw * 3, SH110X_WHITE);
  display.drawFastHLine(gx, gy + ch * 2, cw * 3, SH110X_WHITE);

  for (int i = 0; i < 9; i++) {
    int row = i / 3, col = i % 3;
    int cx = gx + col * cw + cw / 2;
    int cy = gy + row * ch + ch / 2;
    if (tttBoard[i] == 'X') {
      display.drawLine(cx - 6, cy - 6, cx + 6, cy + 6, SH110X_WHITE);
      display.drawLine(cx - 6, cy + 6, cx + 6, cy - 6, SH110X_WHITE);
    } else if (tttBoard[i] == 'O') {
      display.drawCircle(cx, cy, 7, SH110X_WHITE);
    }
  }
  display.display();
}

void handleTttStart() {
  addCORS();
  memset(tttBoard, ' ', 9);
  tttPlayerTurn = true;
  tttResult = "";
  tttActive = true;
  currentMode = MODE_STILL; // safety: no driving while a game is on screen
  stopCar();
  drawTttBoard();
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Game started - your move\"}");
}

void handleTttMove() {
  addCORS();
  if (!tttActive) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Start a game first\"}");
    return;
  }
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"No body\"}");
    return;
  }
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"message\":\"Bad JSON\"}");
    return;
  }
  int cell = doc["cell"] | -1;

  String msg;
  if (tttResult != "") {
    msg = "Game over - start a new game";
  } else if (!tttPlayerTurn) {
    msg = "Not your turn";
  } else if (cell < 0 || cell > 8 || tttBoard[cell] != ' ') {
    msg = "Invalid move";
  } else {
    tttBoard[cell] = 'X';
    char w = tttCheckWinner();
    if (w == 'X') {
      tttResult = "player";
      playFaceSound(FACE_HAPPY);
    } else if (w == 'D') {
      tttResult = "draw";
    } else {
      tttPlayerTurn = false;
      int aiCell = tttAiMove();
      if (aiCell >= 0) tttBoard[aiCell] = 'O';
      char w2 = tttCheckWinner();
      if (w2 == 'O') {
        tttResult = "robot";
        playFaceSound(FACE_SAD);
      } else if (w2 == 'D') {
        tttResult = "draw";
      }
      tttPlayerTurn = true;
    }
    msg = (tttResult != "") ? "Game over" : "OK";
  }
  drawTttBoard();

  StaticJsonDocument<128> res;
  res["ok"] = true;
  res["message"] = msg;
  res["result"] = tttResult;
  String out;
  serializeJson(res, out);
  server.send(200, "application/json", out);
}

void handleTttStop() {
  addCORS();
  tttActive = false;
  registerActivity();
  drawFace(currentFace);
  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Game closed\"}");
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/command", HTTP_POST, handleCommand);
  server.on("/api/wifi", HTTP_POST, handleWifiSet);
  server.on("/api/wifi/reset", HTTP_POST, handleWifiReset);
  server.on("/api/weather", HTTP_POST, handleWeatherSync);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handleSetSettings);
  server.on("/api/ttt/start", HTTP_POST, handleTttStart);
  server.on("/api/ttt/move", HTTP_POST, handleTttMove);
  server.on("/api/ttt/stop", HTTP_POST, handleTttStop);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
  server.begin();
  Serial.println("Web server started on port 80");
}

// ---------------- FACE / OLED ----------------

// "Zuno" splash screen shown once at boot, before the WiFi connect attempt.
void showBootLogo() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(3);
  display.setCursor(28, 20);   // Centered
  display.print("Zuno");
  display.display();
  delay(1800);
}

// Friendly greeting shown right after the boot logo, before the robot
// settles into WiFi connect + its normal face.
void showBootGreeting() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  drawCenteredText("Hi!", 14, 3);
  drawCenteredText("I'm Zuno", 46, 1);
  display.display();
  delay(1500);
}

// Signal-bar WiFi icon, top-right corner. Filled bars = connected,
// outline-only bars = not connected.
void drawWifiIcon(bool connected) {
  int baseX = 109, baseY = 6;
  int barW = 3, gap = 2;
  int heights[4] = {1, 2, 3, 4};
  for (int i = 0; i < 4; i++) {
    int x = baseX + i * (barW + gap);
    int h = heights[i];
    int y = baseY - h;
    if (connected) {
      display.fillRect(x, y, barW, h, SH110X_WHITE);
    } else {
      display.drawFastHLine(baseX + 3, baseY, 12, SH110X_WHITE);
    }
  }
}

// Draws text horizontally centered at the given y (top of the glyphs),
// using the given GFX text size.
void drawCenteredText(const String &text, int y, int textSize) {
  display.setTextSize(textSize);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (128 - w) / 2;
  display.setCursor(x, y - y1);
  display.print(text);
}

// Persistent status bar: real-time clock top-left, WiFi bars top-right.
// Temp/humidity is no longer shown here continuously - tap TEMP in the
// app for a large 10-second reading instead (see showTempOverlay()).
// Battery % itself is app-only (see /api/status) - the OLED only shows a
// small low-battery icon, left side just below the clock, at/below
// BATTERY_LOW_PERCENT (computeAutoFace() also forces Sleep at that point).
void drawHud() {
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  drawWifiIcon(wifiConnected);

  display.setCursor(0, 0);
  display.print(getHudTimeString());

  if (lastBatteryPercent >= 0 && lastBatteryPercent <= BATTERY_LOW_PERCENT) {
    drawLowBatteryIcon(0, 11);
  }
}

// Small battery outline with a near-empty fill, used only as a low-battery
// warning icon (see drawHud()) - never shown at a normal charge level.
void drawLowBatteryIcon(int x, int y) {
  display.drawRect(x, y, 12, 7, SH110X_WHITE);        // body
  display.fillRect(x + 12, y + 2, 2, 3, SH110X_WHITE); // terminal nub
  display.fillRect(x + 1, y + 1, 2, 5, SH110X_WHITE);  // sliver of charge left
}

// Puts text on screen for durationMs, overriding the face until it expires
// (checked and reverted in loop()). Used by Weather and the Scan result.
void showOverlay(String line1, String line2, unsigned long durationMs) {
  overlayLine1 = line1;
  overlayLine2 = line2;
  overlayActive = true;
  overlayUntil = millis() + durationMs;

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  drawCenteredText(overlayLine1, 22, 1);
  drawCenteredText(overlayLine2, 38, 1);
  drawWifiIcon(wifiConnected);
  display.display();
}

// Big single-line "32.5C - 90%" reading from the DHT22, text size 2,
// shown for 10 seconds. Only ever called when no other overlay is active
// (see the show_temp command handler), so it can't collide with Weather
// or the Scan screen.
void showTempOverlay() {
  char buf[24];
  if (isnan(lastTemp) || isnan(lastHum)) {
    snprintf(buf, sizeof(buf), "--C - --%%");
  } else {
    snprintf(buf, sizeof(buf), "%.1fC - %.0f%%", lastTemp, lastHum);
  }
  overlayLine1 = String(buf);
  overlayLine2 = "";
  overlayActive = true;
  overlayUntil = millis() + 10000; // 10 seconds

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  drawCenteredText(overlayLine1, 24, 1); // vertically centered for a size-2 line
  drawWifiIcon(wifiConnected);
  display.display();
}

// One frame of the radar sweep shown during a manual Scan: a half-circle
// dome with a sweeping line at the current servo angle, and a blip if
// something was detected within range.
void drawRadarFrame(int angleDeg, long distanceCm) {
  display.clearDisplay();

  int cx = 64, cy = 60;
  int maxR = 46;

  for (int r = 15; r <= maxR; r += 15) {
    for (int a = 0; a <= 180; a += 4) {
      float rad = radians((float)a);
      int x = cx + (int)(r * cos(rad));
      int y = cy - (int)(r * sin(rad));
      display.drawPixel(x, y, SH110X_WHITE);
    }
  }
  display.drawLine(cx - maxR, cy, cx + maxR, cy, SH110X_WHITE);

  float rad = radians((float)angleDeg);
  int sx = cx + (int)(maxR * cos(rad));
  int sy = cy - (int)(maxR * sin(rad));
  display.drawLine(cx, cy, sx, sy, SH110X_WHITE);

  if (distanceCm > 0 && distanceCm < 100) {
    int br = map(distanceCm, 0, 100, 6, maxR);
    int bx = cx + (int)(br * cos(rad));
    int by = cy - (int)(br * sin(rad));
    display.fillCircle(bx, by, 2, SH110X_WHITE);
  }

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 0);
  display.print("SCANNING");

  display.display();
}

void showScanStatusScreen(long left, long right) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(14, 8);
  display.print("SCAN COMPLETE");
  display.setCursor(6, 30);
  display.print("Left:  ");
  display.print(left);
  display.print(" cm");
  display.setCursor(6, 46);
  display.print("Right: ");
  display.print(right);
  display.print(" cm");
  drawWifiIcon(wifiConnected);
  display.display();
}

// Wrapper around drawFace() that remembers the selected expression so
// handleBlinking() can restore it after a blink.
// Called by the face_* API commands (no longer exposed as buttons in the
// app): shows the picked face immediately and keeps it for
// MANUAL_FACE_DURATION, after which the automatic face logic resumes.
void setManualOverrideFace(FaceType f) {
  manualOverrideActive = true;
  manualOverrideFace = f;
  manualOverrideUntil = millis() + MANUAL_FACE_DURATION;
  currentFace = f;
  if (!overlayActive) drawFace(f);
}

void handleBlinking() {
  if (overlayActive) return;
  if (millis() - blinkTimer > 3000) {
    blinkTimer = millis();
    drawFace(FACE_BLINK);
    delay(120);
    drawFace(currentFace); // restore the selected expression
  }
}

// Adafruit GFX has no native ellipse fill, so this scans row by row
void fillEllipse(int cx, int cy, int rx, int ry) {
  for (int dy = -ry; dy <= ry; dy++) {
    int dx = (int)(rx * sqrt(1.0 - ((float)(dy * dy) / (float)(ry * ry))));
    display.drawFastHLine(cx - dx, cy + dy, 2 * dx + 1, SH110X_WHITE);
  }
}

// "Happy" half circle-shaped eye
void drawHappyEye(int cx, int cy, int r) {
  for (int y= -r; y <=0; y++) {
    int x = sqrt(r * r - y * y);
    display.drawFastHLine(cx - x, cy + y, 2 * x + 1, SH110X_WHITE);
  }
}

// "Sad" eye: domed bottom + pointed top (teardrop pointing up - drooping look)
void drawSadEye(int cx, int cy, int rx, int ry, bool isLeftEye) {
  // Draw the normal eye
  fillEllipse(cx, cy, rx, ry);

  if (isLeftEye) {
    // Left eye: droops toward the center
    display.fillTriangle(
      cx - rx - 2, cy - ry - 2,
      cx + rx + 2, cy - ry + 6,
      cx + rx + 2, cy - ry - 2,
      SH110X_BLACK
    );
  } else {
    // Right eye: mirror image
    display.fillTriangle(
      cx - rx - 2, cy - ry + 6,
      cx - rx - 2, cy - ry - 2,
      cx + rx + 2, cy - ry - 2,
      SH110X_BLACK
    );
  }
}

// Angry eye: triangle with the inner corner pointing down toward center
void drawAngryEye(int cx, int cy, int r, bool isLeftEye) {
  if (isLeftEye) {
    display.fillTriangle(cx - r, cy - r, cx + r, cy + 2, cx - r, cy + r, SH110X_WHITE);
  } else {
    display.fillTriangle(cx + r, cy - r, cx - r, cy + 2, cx + r, cy + r, SH110X_WHITE);
  }
}

// Small heart shape used for the Love face's eyes
void drawHeart(int cx, int cy, int r) {
  display.fillCircle(cx - r / 2, cy, r / 2, SH110X_WHITE);
  display.fillCircle(cx + r / 2, cy, r / 2, SH110X_WHITE);
  display.fillTriangle(cx - r, cy, cx + r, cy, cx, cy + r, SH110X_WHITE);
}


void drawFace(FaceType face) {
  display.clearDisplay();

  // Face area sits below the HUD (which occupies roughly y=0-33)
  int leftX = 42, rightX = 86, eyeY = 40;

  switch (face) {
    case FACE_NORMAL:
      fillEllipse(leftX + normalEyeOffsetX, eyeY + normalEyeOffsetY, 13, 16);
      fillEllipse(rightX + normalEyeOffsetX, eyeY + normalEyeOffsetY, 13, 16);
      break;

    case FACE_HAPPY:
      drawHappyEye(leftX, eyeY, 14);
      drawHappyEye(rightX, eyeY, 14);
      break;

    case FACE_ANGRY:
      drawAngryEye(leftX, eyeY, 15, true);
      drawAngryEye(rightX, eyeY, 15, false);
      break;

    case FACE_SLEEP:
      display.drawFastHLine(leftX - 14, eyeY, 28, SH110X_WHITE);
      display.drawFastHLine(rightX - 14, eyeY, 28, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(80, 16);
      display.print("z");

      display.setCursor(90, 12);
      display.print("Z");
      break;

    case FACE_LOVE:
      drawHeart(leftX, eyeY, 13);
      drawHeart(rightX, eyeY, 13);
      break;

    case FACE_SAD:
      drawSadEye(leftX, eyeY +15, 13, 16, true);
      drawSadEye(rightX, eyeY +15, 13, 16, false);
      break;

    case FACE_SURPRISED:
      fillEllipse(leftX, eyeY, 18, 18);
      fillEllipse(rightX, eyeY, 18, 18);
      break;

    case FACE_BLINK:
    display.drawFastHLine(leftX - 14, eyeY, 28, SH110X_WHITE);
    display.drawFastHLine(rightX - 14, eyeY, 28, SH110X_WHITE);
    break;
  }

  drawHud();
  display.display();
}

// Deliberate "searching" look, drawn during the SCANNING servo sweep -
// eyes shifted toward the direction the servo is currently pointing, so
// the face visually tracks the physical scan (offsetX<0 = looking left,
// >0 = looking right, 0 = center).
void drawLookAroundFace(int offsetX) {
  display.clearDisplay();
  int leftX = 42, rightX = 86, eyeY = 40;
  fillEllipse(leftX + offsetX, eyeY, 13, 16);
  fillEllipse(rightX + offsetX, eyeY, 13, 16);
  drawHud();
  display.display();
}
