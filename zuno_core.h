#ifndef ZUNO_CORE_H
#define ZUNO_CORE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WebServer.h>
#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

#define ZUNO_FW_VERSION "1.2.0"
#define ZUNO_DEVICE_NAME "Zuno-v1.2"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_I2C_ADDR 0x3C

#define ZUNO_AP_SSID "Zuno-v1.2"
#define ZUNO_AP_PASS "zuno12345"

#define OBSTACLE_CM 22U
#define CLEAR_PATH_CM 32U
#define RECOVERY_REVERSE_MS 350UL
#define RECOVERY_TURN_MS 450UL
#define SERVO_SETTLE_MS 220UL
#define BATTERY_LOW_PERCENT 20U
#define FACE_RANDOM_PERIOD_MS 30000UL
#define FACE_RANDOM_DURATION_MS 5000UL

#define ADC_REF_VOLTAGE 3.30f
#define ADC_MAX_COUNT 4095.0f
#define VOLTAGE_DIVIDER_RATIO 5.00f
#define BATTERY_EMPTY_V 6.40f
#define BATTERY_FULL_V 8.40f

#define TASK_FACE_MS 33UL
#define TASK_OLED_MS 33UL
#define TASK_MOTOR_MS 20UL
#define TASK_SERVO_MS 20UL
#define TASK_ULTRASONIC_MS 80UL
#define TASK_BATTERY_MS 1000UL
#define TASK_DHT_MS 2500UL
#define TASK_WEB_MS 5UL
#define TASK_GAME_MS 50UL
#define TASK_SELFDRIVE_MS 60UL
#define TASK_BUZZER_MS 10UL

#define MOTOR_PWM_FREQ 18000U
#define MOTOR_PWM_RES_BITS 8U
#define MOTOR_DEFAULT_SPEED 185U
#define SERVO_PWM_FREQ 50U
#define SERVO_PWM_RES_BITS 16U

static const uint8_t PIN_MOTOR_L_IN1 = 26;
static const uint8_t PIN_MOTOR_L_IN2 = 27;
static const uint8_t PIN_MOTOR_L_EN  = 25;
static const uint8_t PIN_MOTOR_R_IN1 = 14;
static const uint8_t PIN_MOTOR_R_IN2 = 12;
static const uint8_t PIN_MOTOR_R_EN  = 13;
static const uint8_t PIN_ULTRASONIC_TRIG = 5;
static const uint8_t PIN_ULTRASONIC_ECHO = 18;
static const uint8_t PIN_DHT22 = 4;
static const uint8_t PIN_BATTERY_ADC = 34;
static const uint8_t PIN_SERVO = 15;
static const uint8_t PIN_BUZZER = 23;

static const uint8_t MOTOR_L_PWM_CH = 0;
static const uint8_t MOTOR_R_PWM_CH = 1;
static const uint8_t SERVO_PWM_CH = 2;
static const uint8_t BUZZER_PWM_CH = 3;

enum RobotMode : uint8_t {
  MODE_STILL = 0,
  MODE_MANUAL,
  MODE_SELF_DRIVE,
  MODE_GAME
};

enum FaceExpression : uint8_t {
  FACE_NORMAL = 0,
  FACE_HAPPY,
  FACE_ANGRY,
  FACE_SAD,
  FACE_LOVE,
  FACE_SURPRISE,
  FACE_SLEEP,
  FACE_BLINK,
  FACE_LOOK_AROUND
};

enum GameId : uint8_t {
  GAME_NONE = 0,
  GAME_TICTACTOE,
  GAME_CATCH_ME
};

enum MotorCommand : uint8_t {
  MOTOR_STOP = 0,
  MOTOR_FORWARD,
  MOTOR_BACKWARD,
  MOTOR_LEFT,
  MOTOR_RIGHT
};

enum BuzzerSound : uint8_t {
  BUZZER_STARTUP = 0,
  BUZZER_HAPPY,
  BUZZER_ANGRY,
  BUZZER_OBSTACLE,
  BUZZER_BATTERY_LOW,
  BUZZER_GAME_WIN,
  BUZZER_GAME_LOSE
};

struct SensorState {
  float temperatureC;
  float humidity;
  float batteryVoltage;
  uint8_t batteryPercent;
  uint16_t distanceCm;
  bool obstacle;
  bool batteryLow;
  bool dhtValid;
};

struct RobotState {
  RobotMode mode;
  FaceExpression face;
  GameId activeGame;
  bool wifiReady;
  bool motorsEnabled;
  bool selfDriveScanning;
  uint32_t uptimeMs;
};

typedef void (*TaskCallback)();

struct ScheduledTask {
  TaskCallback callback;
  uint32_t intervalMs;
  uint32_t lastRunMs;
  bool enabled;
};

class Scheduler {
public:
  Scheduler();
  bool add(TaskCallback callback, uint32_t intervalMs);
  void update();
  void reset();

private:
  static const uint8_t MAX_TASKS = 12;
  ScheduledTask tasks[MAX_TASKS];
  uint8_t count;
};

extern Adafruit_SH1106G display;
extern WebServer webServer;
extern SensorState sensors;
extern RobotState robot;
extern Scheduler scheduler;

static inline void zunoPwmAttach(uint8_t pin, uint8_t channel, uint32_t freq, uint8_t resolutionBits) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttachChannel(pin, freq, resolutionBits, channel);
#else
  ledcSetup(channel, freq, resolutionBits);
  ledcAttachPin(pin, channel);
#endif
}

static inline void zunoPwmWrite(uint8_t pin, uint8_t channel, uint32_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  (void)channel;
  ledcWrite(pin, duty);
#else
  (void)pin;
  ledcWrite(channel, duty);
#endif
}

static inline void zunoPwmTone(uint8_t pin, uint8_t channel, uint32_t freq) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  (void)channel;
  ledcWriteTone(pin, freq);
#else
  (void)pin;
  ledcWriteTone(channel, freq);
#endif
}

static inline void drawBatteryIcon(Adafruit_GFX &gfx, int16_t x, int16_t y, uint8_t pct) {
  gfx.drawRect(x, y, 18, 9, SH110X_WHITE);
  gfx.fillRect(x + 18, y + 3, 2, 3, SH110X_WHITE);
  uint8_t bars = pct / 20;
  if (bars > 5) bars = 5;
  for (uint8_t i = 0; i < bars; i++) {
    gfx.fillRect(x + 2 + (i * 3), y + 2, 2, 5, SH110X_WHITE);
  }
}

static inline void drawWifiIcon(Adafruit_GFX &gfx, int16_t x, int16_t y, bool on) {
  if (!on) {
    gfx.drawLine(x, y, x + 10, y + 8, SH110X_WHITE);
    gfx.drawLine(x + 10, y, x, y + 8, SH110X_WHITE);
    return;
  }
  gfx.drawCircle(x + 5, y + 9, 1, SH110X_WHITE);
  gfx.drawCircle(x + 5, y + 9, 5, SH110X_WHITE);
  gfx.drawCircle(x + 5, y + 9, 9, SH110X_WHITE);
  gfx.fillRect(x - 5, y + 9, 22, 10, SH110X_BLACK);
}

void faceBegin();
void faceSetExpression(FaceExpression expression, uint32_t durationMs);
void faceUpdate();
void faceRender();
const char *faceName(FaceExpression expression);

void motorBegin();
void motorSet(MotorCommand command, uint8_t speed);
void motorStop();
void motorUpdate();
const char *motorCommandName();

void servoBegin();
void servoSetAngle(uint8_t angle);
uint8_t servoGetAngle();
void servoUpdate();

void ultrasonicBegin();
void ultrasonicUpdate();
uint16_t ultrasonicReadCm();

void batteryBegin();
void batteryUpdate();
uint8_t batteryVoltageToPercent(float voltage);

void buzzerBegin();
void buzzerPlay(BuzzerSound sound);
void buzzerUpdate();

void selfDriveBegin();
void selfDriveUpdate();
const char *selfDriveStateName();

void gamesBegin();
void gamesStart(GameId game);
void gamesStop();
void gamesAction(uint8_t action);
void gamesUpdate();
const char *gameName(GameId game);
const char *gameStatus();

void webUIBegin();
void webUIUpdate();

#endif
