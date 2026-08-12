#include "zuno_core.h"

static uint8_t tttBoard[9];
static uint8_t tttCursor = 0;
static char statusText[20] = "Idle";

static int8_t catchX = 62;
static int8_t catchY = 30;
static int8_t targetX = 20;
static int8_t targetY = 20;
static uint8_t catchScore = 0;
static uint32_t lastCatchMoveMs = 0;

static uint8_t checkWinner() {
  static const uint8_t lines[8][3] PROGMEM = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
  };

  for (uint8_t i = 0; i < 8; i++) {
    uint8_t a = pgm_read_byte(&lines[i][0]);
    uint8_t b = pgm_read_byte(&lines[i][1]);
    uint8_t c = pgm_read_byte(&lines[i][2]);
    if (tttBoard[a] && tttBoard[a] == tttBoard[b] && tttBoard[a] == tttBoard[c]) {
      return tttBoard[a];
    }
  }
  for (uint8_t i = 0; i < 9; i++) {
    if (tttBoard[i] == 0) {
      return 0;
    }
  }
  return 3;
}

static void aiMove() {
  for (uint8_t i = 0; i < 9; i++) {
    if (tttBoard[i] == 0) {
      tttBoard[i] = 2;
      if (checkWinner() == 2) return;
      tttBoard[i] = 0;
    }
  }
  for (uint8_t i = 0; i < 9; i++) {
    if (tttBoard[i] == 0) {
      tttBoard[i] = 1;
      if (checkWinner() == 1) {
        tttBoard[i] = 2;
        return;
      }
      tttBoard[i] = 0;
    }
  }
  const uint8_t order[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
  for (uint8_t i = 0; i < 9; i++) {
    uint8_t p = order[i];
    if (tttBoard[p] == 0) {
      tttBoard[p] = 2;
      return;
    }
  }
}

static void resetTicTacToe() {
  memset(tttBoard, 0, sizeof(tttBoard));
  tttCursor = 0;
  strncpy(statusText, "Your turn", sizeof(statusText));
}

static void resetCatchMe() {
  catchX = 62;
  catchY = 30;
  targetX = random(8, 120);
  targetY = random(12, 56);
  catchScore = 0;
  strncpy(statusText, "Catch target", sizeof(statusText));
}

static void renderTicTacToe() {
  display.clearDisplay();
  display.drawLine(49, 8, 49, 62, SH110X_WHITE);
  display.drawLine(78, 8, 78, 62, SH110X_WHITE);
  display.drawLine(21, 26, 106, 26, SH110X_WHITE);
  display.drawLine(21, 44, 106, 44, SH110X_WHITE);

  for (uint8_t i = 0; i < 9; i++) {
    const int16_t col = i % 3;
    const int16_t row = i / 3;
    const int16_t cx = 35 + (col * 29);
    const int16_t cy = 17 + (row * 18);
    if (i == tttCursor) {
      display.drawRect(cx - 10, cy - 7, 20, 14, SH110X_WHITE);
    }
    if (tttBoard[i] == 1) {
      display.drawLine(cx - 5, cy - 5, cx + 5, cy + 5, SH110X_WHITE);
      display.drawLine(cx + 5, cy - 5, cx - 5, cy + 5, SH110X_WHITE);
    } else if (tttBoard[i] == 2) {
      display.drawCircle(cx, cy, 6, SH110X_WHITE);
    }
  }

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(statusText);
  display.display();
}

static void renderCatchMe() {
  display.clearDisplay();
  display.drawCircle(targetX, targetY, 4, SH110X_WHITE);
  display.fillRect(catchX - 3, catchY - 3, 7, 7, SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Score:"));
  display.print(catchScore);
  display.display();
}

void gamesBegin() {
  robot.activeGame = GAME_NONE;
}

void gamesStart(GameId game) {
  robot.activeGame = game;
  robot.mode = MODE_GAME;
  if (game == GAME_TICTACTOE) {
    resetTicTacToe();
  } else if (game == GAME_CATCH_ME) {
    resetCatchMe();
  }
}

void gamesStop() {
  robot.activeGame = GAME_NONE;
  robot.mode = MODE_STILL;
  strncpy(statusText, "Idle", sizeof(statusText));
}

void gamesAction(uint8_t action) {
  if (robot.activeGame == GAME_TICTACTOE) {
    if (action < 9) {
      tttCursor = action;
      return;
    }
    if (action == 20 && tttBoard[tttCursor] == 0 && checkWinner() == 0) {
      tttBoard[tttCursor] = 1;
      uint8_t winner = checkWinner();
      if (winner == 1) {
        strncpy(statusText, "You win", sizeof(statusText));
        buzzerPlay(BUZZER_GAME_WIN);
      } else if (winner == 3) {
        strncpy(statusText, "Draw", sizeof(statusText));
      } else {
        aiMove();
        winner = checkWinner();
        if (winner == 2) {
          strncpy(statusText, "Zuno wins", sizeof(statusText));
          buzzerPlay(BUZZER_GAME_LOSE);
        } else if (winner == 3) {
          strncpy(statusText, "Draw", sizeof(statusText));
        }
      }
    } else if (action == 21) {
      resetTicTacToe();
    }
  } else if (robot.activeGame == GAME_CATCH_ME) {
    if (action == 1 && catchY > 10) catchY -= 4;
    if (action == 2 && catchY < 60) catchY += 4;
    if (action == 3 && catchX > 4) catchX -= 4;
    if (action == 4 && catchX < 124) catchX += 4;
  }
}

void gamesUpdate() {
  if (robot.activeGame == GAME_NONE) {
    return;
  }

  if (robot.activeGame == GAME_CATCH_ME) {
    const uint32_t now = millis();
    if ((uint32_t)(now - lastCatchMoveMs) > 900UL) {
      lastCatchMoveMs = now;
      targetX = constrain(targetX + (int8_t)random(-8, 9), 6, 122);
      targetY = constrain(targetY + (int8_t)random(-6, 7), 12, 58);
    }
    if (abs(catchX - targetX) < 7 && abs(catchY - targetY) < 7) {
      catchScore++;
      targetX = random(8, 120);
      targetY = random(12, 56);
      buzzerPlay(BUZZER_HAPPY);
    }
    renderCatchMe();
  } else if (robot.activeGame == GAME_TICTACTOE) {
    renderTicTacToe();
  }
}

const char *gameName(GameId game) {
  switch (game) {
    case GAME_TICTACTOE: return "Tic Tac Toe";
    case GAME_CATCH_ME: return "Catch Me";
    default: return "None";
  }
}

const char *gameStatus() {
  return statusText;
}
