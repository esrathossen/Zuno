#include "motor.h"

#include "pins.h"

ZunoMotorDriver::ZunoMotorDriver() : direction_(ZUNO_MOTOR_STOP), speed_(0) {}

void ZunoMotorDriver::begin() {
  pinMode(ZunoPins::motorIn1, OUTPUT);
  pinMode(ZunoPins::motorIn2, OUTPUT);
  pinMode(ZunoPins::motorIn3, OUTPUT);
  pinMode(ZunoPins::motorIn4, OUTPUT);
  pinMode(ZunoPins::motorEna, OUTPUT);
  pinMode(ZunoPins::motorEnb, OUTPUT);
  stop();
}

void ZunoMotorDriver::forward(uint8_t speed) {
  setDirection(ZUNO_MOTOR_FORWARD, speed);
}

void ZunoMotorDriver::backward(uint8_t speed) {
  setDirection(ZUNO_MOTOR_BACKWARD, speed);
}

void ZunoMotorDriver::left(uint8_t speed) {
  setDirection(ZUNO_MOTOR_LEFT, speed);
}

void ZunoMotorDriver::right(uint8_t speed) {
  setDirection(ZUNO_MOTOR_RIGHT, speed);
}

void ZunoMotorDriver::stop() {
  direction_ = ZUNO_MOTOR_STOP;
  speed_ = 0;
  digitalWrite(ZunoPins::motorIn1, LOW);
  digitalWrite(ZunoPins::motorIn2, LOW);
  digitalWrite(ZunoPins::motorIn3, LOW);
  digitalWrite(ZunoPins::motorIn4, LOW);
  applySpeed(0);
}

ZunoMotorDirection ZunoMotorDriver::direction() const {
  return direction_;
}

uint8_t ZunoMotorDriver::speed() const {
  return speed_;
}

bool ZunoMotorDriver::isMoving() const {
  return direction_ != ZUNO_MOTOR_STOP && speed_ > 0;
}

void ZunoMotorDriver::setLeftMotor(bool forward, bool enabled) {
  if (!enabled) {
    digitalWrite(ZunoPins::motorIn1, LOW);
    digitalWrite(ZunoPins::motorIn2, LOW);
    return;
  }
  digitalWrite(ZunoPins::motorIn1, forward ? HIGH : LOW);
  digitalWrite(ZunoPins::motorIn2, forward ? LOW : HIGH);
}

void ZunoMotorDriver::setRightMotor(bool forward, bool enabled) {
  if (!enabled) {
    digitalWrite(ZunoPins::motorIn3, LOW);
    digitalWrite(ZunoPins::motorIn4, LOW);
    return;
  }
  digitalWrite(ZunoPins::motorIn3, forward ? HIGH : LOW);
  digitalWrite(ZunoPins::motorIn4, forward ? LOW : HIGH);
}

void ZunoMotorDriver::applySpeed(uint8_t speed) {
  analogWrite(ZunoPins::motorEna, speed);
  analogWrite(ZunoPins::motorEnb, speed);
}

void ZunoMotorDriver::setDirection(ZunoMotorDirection direction, uint8_t speed) {
  if (speed == 0) {
    stop();
    return;
  }

  direction_ = direction;
  speed_ = speed;

  switch (direction) {
    case ZUNO_MOTOR_FORWARD:
      setLeftMotor(true, true);
      setRightMotor(true, true);
      break;
    case ZUNO_MOTOR_BACKWARD:
      setLeftMotor(false, true);
      setRightMotor(false, true);
      break;
    case ZUNO_MOTOR_LEFT:
      setLeftMotor(false, true);
      setRightMotor(true, true);
      break;
    case ZUNO_MOTOR_RIGHT:
      setLeftMotor(true, true);
      setRightMotor(false, true);
      break;
    default:
      stop();
      return;
  }

  applySpeed(speed_);
}