#include "zuno_core.h"

// Motor driver
static MotorCommand currentCommand = MOTOR_STOP;
static uint8_t currentSpeed = MOTOR_DEFAULT_SPEED;

static void writeMotor(int8_t leftDir, int8_t rightDir, uint8_t speed) {
  digitalWrite(PIN_MOTOR_L_IN1, leftDir > 0 ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_L_IN2, leftDir < 0 ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_R_IN1, rightDir > 0 ? HIGH : LOW);
  digitalWrite(PIN_MOTOR_R_IN2, rightDir < 0 ? HIGH : LOW);
  zunoPwmWrite(PIN_MOTOR_L_EN, MOTOR_L_PWM_CH, leftDir == 0 ? 0 : speed);
  zunoPwmWrite(PIN_MOTOR_R_EN, MOTOR_R_PWM_CH, rightDir == 0 ? 0 : speed);
}

void motorBegin() {
  pinMode(PIN_MOTOR_L_IN1, OUTPUT);
  pinMode(PIN_MOTOR_L_IN2, OUTPUT);
  pinMode(PIN_MOTOR_R_IN1, OUTPUT);
  pinMode(PIN_MOTOR_R_IN2, OUTPUT);
  zunoPwmAttach(PIN_MOTOR_L_EN, MOTOR_L_PWM_CH, MOTOR_PWM_FREQ, MOTOR_PWM_RES_BITS);
  zunoPwmAttach(PIN_MOTOR_R_EN, MOTOR_R_PWM_CH, MOTOR_PWM_FREQ, MOTOR_PWM_RES_BITS);
  robot.motorsEnabled = true;
  motorStop();
}

void motorSet(MotorCommand command, uint8_t speed) {
  if (sensors.batteryLow) command = MOTOR_STOP;
  currentCommand = command;
  currentSpeed = speed;
}

void motorStop() {
  currentCommand = MOTOR_STOP;
  writeMotor(0, 0, 0);
}

void motorUpdate() {
  if (!robot.motorsEnabled || sensors.batteryLow) {
    writeMotor(0, 0, 0);
    return;
  }
  switch (currentCommand) {
    case MOTOR_FORWARD: writeMotor(1, 1, currentSpeed); break;
    case MOTOR_BACKWARD: writeMotor(-1, -1, currentSpeed); break;
    case MOTOR_LEFT: writeMotor(-1, 1, currentSpeed); break;
    case MOTOR_RIGHT: writeMotor(1, -1, currentSpeed); break;
    default: writeMotor(0, 0, 0); break;
  }
}

const char *motorCommandName() {
  switch (currentCommand) {
    case MOTOR_FORWARD: return "Forward";
    case MOTOR_BACKWARD: return "Backward";
    case MOTOR_LEFT: return "Left";
    case MOTOR_RIGHT: return "Right";
    default: return "Stop";
  }
}

// Servo driver
static uint8_t currentAngle = 90;
static uint8_t targetAngle = 90;
static uint32_t lastStepMs = 0;

static uint32_t angleToDuty(uint8_t angle) {
  const uint32_t minUs = 500;
  const uint32_t maxUs = 2400;
  const uint32_t us = map(constrain(angle, 0, 180), 0, 180, minUs, maxUs);
  return (us * ((1UL << SERVO_PWM_RES_BITS) - 1UL)) / 20000UL;
}

void servoBegin() {
  zunoPwmAttach(PIN_SERVO, SERVO_PWM_CH, SERVO_PWM_FREQ, SERVO_PWM_RES_BITS);
  servoSetAngle(90);
  zunoPwmWrite(PIN_SERVO, SERVO_PWM_CH, angleToDuty(currentAngle));
}

void servoSetAngle(uint8_t angle) {
  targetAngle = constrain(angle, 0, 180);
}

uint8_t servoGetAngle() {
  return currentAngle;
}

void servoUpdate() {
  const uint32_t now = millis();
  if ((uint32_t)(now - lastStepMs) < 15UL) return;
  lastStepMs = now;
  if (currentAngle < targetAngle) currentAngle++;
  if (currentAngle > targetAngle) currentAngle--;
  zunoPwmWrite(PIN_SERVO, SERVO_PWM_CH, angleToDuty(currentAngle));
}

// Ultrasonic driver
static uint16_t lastDistanceCm = 250;

void ultrasonicBegin() {
  pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
}

uint16_t ultrasonicReadCm() {
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);

  const uint32_t duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH, 25000UL);
  if (duration == 0) return 250;
  uint16_t cm = (uint16_t)(duration / 58UL);
  if (cm > 250) cm = 250;
  return cm;
}

void ultrasonicUpdate() {
  lastDistanceCm = ultrasonicReadCm();
  sensors.distanceCm = lastDistanceCm;
  sensors.obstacle = lastDistanceCm <= OBSTACLE_CM;
}

// Battery manager
static bool lowSoundPlayed = false;

void batteryBegin() {
  pinMode(PIN_BATTERY_ADC, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
}

uint8_t batteryVoltageToPercent(float voltage) {
  if (voltage <= BATTERY_EMPTY_V) return 0;
  if (voltage >= BATTERY_FULL_V) return 100;
  return (uint8_t)(((voltage - BATTERY_EMPTY_V) * 100.0f) / (BATTERY_FULL_V - BATTERY_EMPTY_V));
}

void batteryUpdate() {
  uint32_t total = 0;
  for (uint8_t i = 0; i < 8; i++) {
    total += analogRead(PIN_BATTERY_ADC);
  }
  const float adc = (float)total / 8.0f;
  const float pinVoltage = (adc * ADC_REF_VOLTAGE) / ADC_MAX_COUNT;
  sensors.batteryVoltage = pinVoltage * VOLTAGE_DIVIDER_RATIO;
  sensors.batteryPercent = batteryVoltageToPercent(sensors.batteryVoltage);
  sensors.batteryLow = sensors.batteryPercent <= BATTERY_LOW_PERCENT;

  if (sensors.batteryLow) {
    motorStop();
    faceSetExpression(FACE_SLEEP, 0);
    if (!lowSoundPlayed) {
      buzzerPlay(BUZZER_BATTERY_LOW);
      lowSoundPlayed = true;
    }
  } else {
    lowSoundPlayed = false;
  }
}

// Buzzer driver
struct Note {
  uint16_t freq;
  uint16_t durationMs;
};

static const Note startupTune[] PROGMEM = { {880, 70}, {1175, 80}, {1568, 100}, {0, 40} };
static const Note happyTune[] PROGMEM = { {1319, 55}, {1568, 70}, {0, 20} };
static const Note angryTune[] PROGMEM = { {220, 90}, {165, 120}, {0, 30} };
static const Note obstacleTune[] PROGMEM = { {988, 45}, {0, 30}, {988, 45}, {0, 30} };
static const Note batteryTune[] PROGMEM = { {392, 120}, {330, 140}, {262, 160}, {0, 40} };
static const Note winTune[] PROGMEM = { {1047, 70}, {1319, 70}, {1568, 120}, {0, 30} };
static const Note loseTune[] PROGMEM = { {330, 120}, {262, 160}, {0, 40} };

static const Note *activeTune = nullptr;
static uint8_t activeLen = 0;
static uint8_t noteIndex = 0;
static uint32_t nextNoteMs = 0;

static void selectTune(BuzzerSound sound, const Note **tune, uint8_t *len) {
  switch (sound) {
    case BUZZER_HAPPY: *tune = happyTune; *len = 3; break;
    case BUZZER_ANGRY: *tune = angryTune; *len = 3; break;
    case BUZZER_OBSTACLE: *tune = obstacleTune; *len = 4; break;
    case BUZZER_BATTERY_LOW: *tune = batteryTune; *len = 4; break;
    case BUZZER_GAME_WIN: *tune = winTune; *len = 4; break;
    case BUZZER_GAME_LOSE: *tune = loseTune; *len = 3; break;
    default: *tune = startupTune; *len = 4; break;
  }
}

void buzzerBegin() {
  zunoPwmAttach(PIN_BUZZER, BUZZER_PWM_CH, 2000, 10);
  zunoPwmTone(PIN_BUZZER, BUZZER_PWM_CH, 0);
}

void buzzerPlay(BuzzerSound sound) {
  selectTune(sound, &activeTune, &activeLen);
  noteIndex = 0;
  nextNoteMs = 0;
}

void buzzerUpdate() {
  const uint32_t now = millis();
  if (activeTune == nullptr) {
    zunoPwmTone(PIN_BUZZER, BUZZER_PWM_CH, 0);
    return;
  }
  if (nextNoteMs && (int32_t)(now - nextNoteMs) < 0) return;
  if (noteIndex >= activeLen) {
    zunoPwmTone(PIN_BUZZER, BUZZER_PWM_CH, 0);
    activeTune = nullptr;
    return;
  }

  Note note;
  memcpy_P(&note, &activeTune[noteIndex], sizeof(Note));
  zunoPwmTone(PIN_BUZZER, BUZZER_PWM_CH, note.freq);
  nextNoteMs = now + note.durationMs;
  noteIndex++;
}
