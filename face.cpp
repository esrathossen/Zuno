#include "zuno_core.h"

struct FaceEngine {
  FaceExpression expression;
  FaceExpression previous;
  uint32_t expressionUntilMs;
  uint32_t lastBlinkMs;
  uint32_t lastLookMs;
  uint32_t lastRandomMs;
  int8_t gazeX;
  int8_t gazeY;
  int8_t targetGazeX;
  int8_t targetGazeY;
  uint8_t blinkLevel;
  int8_t blinkDir;
};

static FaceEngine face;

static void drawEye(int16_t cx, int16_t cy, uint8_t w, uint8_t h, int8_t gx, int8_t gy) {
  const int16_t x = cx - (w / 2) + gx;
  const int16_t y = cy - (h / 2) + gy;
  display.fillRoundRect(x, y, w, h, 8, SH110X_WHITE);
}

static void drawHeart(int16_t x, int16_t y) {
  display.fillCircle(x - 3, y - 2, 3, SH110X_WHITE);
  display.fillCircle(x + 3, y - 2, 3, SH110X_WHITE);
  display.fillTriangle(x - 7, y, x + 7, y, x, y + 9, SH110X_WHITE);
}

static void drawMouth(FaceExpression expression) {
  switch (expression) {
    case FACE_HAPPY:
      display.drawCircle(64, 42, 15, SH110X_WHITE);
      display.fillRect(44, 24, 40, 18, SH110X_BLACK);
      break;
    case FACE_ANGRY:
      display.drawLine(50, 49, 78, 49, SH110X_WHITE);
      display.drawLine(50, 50, 78, 50, SH110X_WHITE);
      break;
    case FACE_SAD:
      display.drawCircle(64, 58, 15, SH110X_WHITE);
      display.fillRect(44, 58, 40, 12, SH110X_BLACK);
      break;
    case FACE_SURPRISE:
      display.drawCircle(64, 45, 8, SH110X_WHITE);
      break;
    case FACE_SLEEP:
      display.drawLine(53, 48, 75, 48, SH110X_WHITE);
      display.setTextSize(1);
      display.setCursor(92, 16);
      display.print(F("Zz"));
      break;
    default:
      display.drawLine(54, 48, 74, 48, SH110X_WHITE);
      break;
  }
}

static void drawFace() {
  const FaceExpression e = face.expression;
  uint8_t eyeW = 28;
  uint8_t eyeH = 18;
  int8_t gy = face.gazeY;

  if (e == FACE_SLEEP) {
    display.drawLine(32, 31, 56, 31, SH110X_WHITE);
    display.drawLine(72, 31, 96, 31, SH110X_WHITE);
    drawMouth(e);
    return;
  }

  if (e == FACE_BLINK || face.blinkLevel > 0) {
    eyeH = (uint8_t)max(2, 18 - (face.blinkLevel * 3));
  }

  if (e == FACE_SURPRISE) {
    eyeW = 20;
    eyeH = 22;
  }

  if (e == FACE_LOVE) {
    drawHeart(43 + face.gazeX, 29 + gy);
    drawHeart(85 + face.gazeX, 29 + gy);
  } else {
    drawEye(43, 30, eyeW, eyeH, face.gazeX, gy);
    drawEye(85, 30, eyeW, eyeH, face.gazeX, gy);
  }

  if (e == FACE_ANGRY) {
    display.drawLine(28, 16, 55, 24, SH110X_BLACK);
    display.drawLine(100, 16, 73, 24, SH110X_BLACK);
    display.drawLine(28, 15, 55, 23, SH110X_BLACK);
    display.drawLine(100, 15, 73, 23, SH110X_BLACK);
  }

  drawMouth(e);
}

void faceBegin() {
  face.expression = FACE_NORMAL;
  face.previous = FACE_NORMAL;
  face.expressionUntilMs = 0;
  face.lastBlinkMs = millis();
  face.lastLookMs = millis();
  face.lastRandomMs = millis();
  face.gazeX = 0;
  face.gazeY = 0;
  face.targetGazeX = 0;
  face.targetGazeY = 0;
  face.blinkLevel = 0;
  face.blinkDir = 0;
  robot.face = FACE_NORMAL;
}

void faceSetExpression(FaceExpression expression, uint32_t durationMs) {
  if (face.expression != expression) {
    face.previous = face.expression;
    face.expression = expression;
    robot.face = expression;
    if (expression == FACE_HAPPY) {
      buzzerPlay(BUZZER_HAPPY);
    } else if (expression == FACE_ANGRY) {
      buzzerPlay(BUZZER_ANGRY);
    }
  }
  face.expressionUntilMs = durationMs ? millis() + durationMs : 0;
}

void faceUpdate() {
  const uint32_t now = millis();

  if (sensors.batteryLow) {
    faceSetExpression(FACE_SLEEP, 0);
    return;
  }

  if (face.expressionUntilMs && (int32_t)(now - face.expressionUntilMs) >= 0) {
    face.expressionUntilMs = 0;
    face.expression = (robot.mode == MODE_STILL) ? FACE_LOOK_AROUND : FACE_NORMAL;
    robot.face = face.expression;
  }

  if ((uint32_t)(now - face.lastLookMs) > 1200UL) {
    face.lastLookMs = now;
    if (robot.mode == MODE_STILL || face.expression == FACE_LOOK_AROUND || robot.selfDriveScanning) {
      face.targetGazeX = (int8_t)random(-6, 7);
      face.targetGazeY = (int8_t)random(-2, 3);
    } else {
      face.targetGazeX = 0;
      face.targetGazeY = 0;
    }
  }

  if (face.gazeX < face.targetGazeX) face.gazeX++;
  if (face.gazeX > face.targetGazeX) face.gazeX--;
  if (face.gazeY < face.targetGazeY) face.gazeY++;
  if (face.gazeY > face.targetGazeY) face.gazeY--;

  if (face.blinkDir == 0 && (uint32_t)(now - face.lastBlinkMs) > (uint32_t)random(2600, 5200)) {
    face.blinkDir = 1;
  }
  if (face.blinkDir != 0) {
    const int16_t next = (int16_t)face.blinkLevel + face.blinkDir;
    face.blinkLevel = (uint8_t)constrain(next, 0, 6);
    if (face.blinkLevel >= 6) {
      face.blinkDir = -1;
    } else if (face.blinkLevel == 0) {
      face.blinkDir = 0;
      face.lastBlinkMs = now;
    }
  }

  if (robot.mode == MODE_STILL && (uint32_t)(now - face.lastRandomMs) >= FACE_RANDOM_PERIOD_MS) {
    face.lastRandomMs = now;
    const FaceExpression options[] = { FACE_HAPPY, FACE_SAD, FACE_LOVE, FACE_SURPRISE, FACE_LOOK_AROUND };
    faceSetExpression(options[random(0, 5)], FACE_RANDOM_DURATION_MS);
  }
}

void faceRender() {
  if (robot.activeGame != GAME_NONE) {
    return;
  }

  display.clearDisplay();
  drawFace();
  drawBatteryIcon(display, 3, 3, sensors.batteryPercent);
  drawWifiIcon(display, 111, 2, robot.wifiReady);
  display.display();
}

const char *faceName(FaceExpression expression) {
  switch (expression) {
    case FACE_HAPPY: return "Happy";
    case FACE_ANGRY: return "Angry";
    case FACE_SAD: return "Sad";
    case FACE_LOVE: return "Love";
    case FACE_SURPRISE: return "Surprise";
    case FACE_SLEEP: return "Sleep";
    case FACE_BLINK: return "Blink";
    case FACE_LOOK_AROUND: return "Look Around";
    default: return "Normal";
  }
}
