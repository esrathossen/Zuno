#pragma once

#include <Arduino.h>
#include <Adafruit_SH110X.h>

enum ZunoFaceType : uint8_t {
  ZUNO_FACE_NORMAL = 0,
  ZUNO_FACE_HAPPY,
  ZUNO_FACE_ANGRY,
  ZUNO_FACE_SAD,
  ZUNO_FACE_LOVE,
  ZUNO_FACE_SURPRISE,
  ZUNO_FACE_SLEEP,
  ZUNO_FACE_BLINK,
  ZUNO_FACE_LOOK_AROUND
};

class ZunoFaceEngine {
 public:
  explicit ZunoFaceEngine(Adafruit_SH1106G &display);

  void begin();
  void showBootLogo(uint16_t holdMs);
  void setFace(ZunoFaceType face, uint32_t nowMs);
  void setLowBattery(bool lowBattery);
  void setScanning(bool scanning);
  void update(uint32_t nowMs);
  ZunoFaceType currentFace() const;

 private:
  Adafruit_SH1106G &display_;
  ZunoFaceType face_;
  uint32_t lastFrameMs_;
  uint32_t nextBlinkMs_;
  uint32_t blinkStartMs_;
  uint32_t nextLookMs_;
  uint32_t randomFaceAtMs_;
  uint32_t randomFaceUntilMs_;
  bool blinkActive_;
  bool lowBattery_;
  bool scanning_;
  int8_t eyeOffsetX_;
  int8_t scanDir_;

  void drawFrame(uint32_t nowMs);
  void drawHud();
  void drawEyes(ZunoFaceType face, uint8_t blinkClose);
  void drawMouth(ZunoFaceType face);
  void drawLowBatteryIcon(uint8_t x, uint8_t y);
  void fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t color);
  void drawHeart(int16_t cx, int16_t cy, int16_t r, uint16_t color);
  ZunoFaceType pickRandomFace();
};