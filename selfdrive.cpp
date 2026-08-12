#include "zuno_core.h"

enum DriveState : uint8_t {
  SD_FORWARD = 0,
  SD_STOP,
  SD_SCAN_LEFT,
  SD_SCAN_RIGHT,
  SD_DECIDE,
  SD_RECOVER_BACK,
  SD_RECOVER_TURN
};

static DriveState state = SD_FORWARD;
static uint32_t stateStartedMs = 0;
static uint16_t leftCm = 0;
static uint16_t rightCm = 0;
static bool turnRight = true;

static void changeState(DriveState next) {
  state = next;
  stateStartedMs = millis();
}

void selfDriveBegin() {
  changeState(SD_FORWARD);
}

void selfDriveUpdate() {
  if (robot.mode != MODE_SELF_DRIVE || sensors.batteryLow) {
    robot.selfDriveScanning = false;
    return;
  }

  const uint32_t now = millis();
  switch (state) {
    case SD_FORWARD:
      robot.selfDriveScanning = false;
      servoSetAngle(90);
      faceSetExpression(FACE_NORMAL, 0);
      motorSet(MOTOR_FORWARD, MOTOR_DEFAULT_SPEED);
      if (sensors.obstacle) {
        motorStop();
        faceSetExpression(FACE_ANGRY, 900);
        buzzerPlay(BUZZER_OBSTACLE);
        changeState(SD_STOP);
      }
      break;

    case SD_STOP:
      motorStop();
      robot.selfDriveScanning = true;
      faceSetExpression(FACE_LOOK_AROUND, 0);
      servoSetAngle(35);
      if ((uint32_t)(now - stateStartedMs) >= SERVO_SETTLE_MS) {
        changeState(SD_SCAN_LEFT);
      }
      break;

    case SD_SCAN_LEFT:
      leftCm = ultrasonicReadCm();
      servoSetAngle(145);
      if ((uint32_t)(now - stateStartedMs) >= SERVO_SETTLE_MS) {
        changeState(SD_SCAN_RIGHT);
      }
      break;

    case SD_SCAN_RIGHT:
      rightCm = ultrasonicReadCm();
      servoSetAngle(90);
      changeState(SD_DECIDE);
      break;

    case SD_DECIDE:
      robot.selfDriveScanning = false;
      turnRight = rightCm >= leftCm;
      if (leftCm < CLEAR_PATH_CM && rightCm < CLEAR_PATH_CM) {
        motorSet(MOTOR_BACKWARD, MOTOR_DEFAULT_SPEED);
        changeState(SD_RECOVER_BACK);
      } else {
        faceSetExpression(FACE_HAPPY, 700);
        motorSet(turnRight ? MOTOR_RIGHT : MOTOR_LEFT, MOTOR_DEFAULT_SPEED);
        changeState(SD_RECOVER_TURN);
      }
      break;

    case SD_RECOVER_BACK:
      if ((uint32_t)(now - stateStartedMs) >= RECOVERY_REVERSE_MS) {
        motorSet(turnRight ? MOTOR_RIGHT : MOTOR_LEFT, MOTOR_DEFAULT_SPEED);
        changeState(SD_RECOVER_TURN);
      }
      break;

    case SD_RECOVER_TURN:
      if ((uint32_t)(now - stateStartedMs) >= RECOVERY_TURN_MS) {
        changeState(SD_FORWARD);
      }
      break;
  }
}

const char *selfDriveStateName() {
  switch (state) {
    case SD_STOP: return "Stop";
    case SD_SCAN_LEFT: return "Scan Left";
    case SD_SCAN_RIGHT: return "Scan Right";
    case SD_DECIDE: return "Decide";
    case SD_RECOVER_BACK: return "Recover Back";
    case SD_RECOVER_TURN: return "Recover Turn";
    default: return "Forward";
  }
}
