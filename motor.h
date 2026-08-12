#pragma once

#include <Arduino.h>

enum ZunoMotorDirection : uint8_t {
  ZUNO_MOTOR_STOP = 0,
  ZUNO_MOTOR_FORWARD,
  ZUNO_MOTOR_BACKWARD,
  ZUNO_MOTOR_LEFT,
  ZUNO_MOTOR_RIGHT
};

class ZunoMotorDriver {
 public:
  ZunoMotorDriver();

  void begin();
  void forward(uint8_t speed);
  void backward(uint8_t speed);
  void left(uint8_t speed);
  void right(uint8_t speed);
  void stop();

  ZunoMotorDirection direction() const;
  uint8_t speed() const;
  bool isMoving() const;

 private:
  ZunoMotorDirection direction_;
  uint8_t speed_;

  void setLeftMotor(bool forward, bool enabled);
  void setRightMotor(bool forward, bool enabled);
  void applySpeed(uint8_t speed);
  void setDirection(ZunoMotorDirection direction, uint8_t speed);
};