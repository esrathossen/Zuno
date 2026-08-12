#include <WiFi.h>
#include <LittleFS.h>
#include "zuno_core.h"

WebServer webServer(80);

static const char missingFsPage[] PROGMEM =
  "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>Zuno v1.2</title><style>body{font-family:Arial,sans-serif;background:#0d1117;color:#f3f6fb;"
  "padding:24px;line-height:1.5}.box{max-width:520px;border:1px solid #303946;border-radius:8px;"
  "padding:18px;background:#151b23}code{color:#49d0a9}</style></head><body><div class='box'>"
  "<h2>Zuno Web UI not uploaded</h2><p>ESP32 web server is running, but LittleFS cannot find "
  "<code>/index.html</code>.</p><p>Upload the sketch <code>data</code> folder to LittleFS, reboot Zuno, "
  "then open <code>http://192.168.4.1/</code>.</p><p>Status API: <code>/api/status</code></p>"
  "</div></body></html>";

static void sendJsonStatus() {
  char json[420];
  snprintf(json, sizeof(json),
           "{\"mode\":\"%s\",\"face\":\"%s\",\"motor\":\"%s\",\"selfDrive\":\"%s\","
           "\"temperature\":%.1f,\"humidity\":%.1f,\"dhtValid\":%s,"
           "\"battery\":%u,\"voltage\":%.2f,\"distance\":%u,\"obstacle\":%s,"
           "\"wifi\":%s,\"game\":\"%s\",\"gameStatus\":\"%s\",\"uptime\":%lu}",
           robot.mode == MODE_SELF_DRIVE ? "Self Driving" :
           robot.mode == MODE_MANUAL ? "Manual" :
           robot.mode == MODE_GAME ? "Game" : "Still",
           faceName(robot.face),
           motorCommandName(),
           selfDriveStateName(),
           sensors.temperatureC,
           sensors.humidity,
           sensors.dhtValid ? "true" : "false",
           sensors.batteryPercent,
           sensors.batteryVoltage,
           sensors.distanceCm,
           sensors.obstacle ? "true" : "false",
           robot.wifiReady ? "true" : "false",
           gameName(robot.activeGame),
           gameStatus(),
           (unsigned long)(millis() / 1000UL));
  webServer.send(200, "application/json", json);
}

static void sendIndex() {
  if (!LittleFS.exists("/index.html")) {
    webServer.send_P(200, "text/html", missingFsPage);
    return;
  }

  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    webServer.send_P(200, "text/html", missingFsPage);
    return;
  }
  webServer.streamFile(file, "text/html");
  file.close();
}

static void setMode(RobotMode mode) {
  if (mode != MODE_GAME) {
    gamesStop();
  }
  robot.mode = mode;
  if (mode == MODE_STILL) {
    motorStop();
    faceSetExpression(FACE_LOOK_AROUND, 0);
  } else if (mode == MODE_MANUAL || mode == MODE_SELF_DRIVE) {
    faceSetExpression(FACE_NORMAL, 0);
  }
  webServer.send(200, "application/json", "{\"ok\":true}");
}

static void handleManual(MotorCommand cmd) {
  gamesStop();
  robot.mode = MODE_MANUAL;
  if (cmd == MOTOR_STOP) {
    motorStop();
  } else {
    motorSet(cmd, MOTOR_DEFAULT_SPEED);
  }
  webServer.send(200, "application/json", "{\"ok\":true}");
}

static void handleFace(FaceExpression expression) {
  faceSetExpression(expression, FACE_RANDOM_DURATION_MS);
  webServer.send(200, "application/json", "{\"ok\":true}");
}

static void handleGame(GameId game) {
  gamesStart(game);
  webServer.send(200, "application/json", "{\"ok\":true}");
}

static void notFound() {
  webServer.send(404, "application/json", "{\"error\":\"not found\"}");
}

void webUIBegin() {
  WiFi.mode(WIFI_AP);
  robot.wifiReady = WiFi.softAP(ZUNO_AP_SSID, ZUNO_AP_PASS);

  webServer.on("/", HTTP_GET, sendIndex);
  webServer.on("/index.html", HTTP_GET, sendIndex);
  webServer.serveStatic("/app.css", LittleFS, "/app.css", "max-age=600");
  webServer.serveStatic("/app.js", LittleFS, "/app.js", "max-age=600");

  webServer.on("/api/status", HTTP_GET, sendJsonStatus);
  webServer.on("/api/mode/still", HTTP_POST, []() { setMode(MODE_STILL); });
  webServer.on("/api/mode/manual", HTTP_POST, []() { setMode(MODE_MANUAL); });
  webServer.on("/api/mode/self", HTTP_POST, []() { setMode(MODE_SELF_DRIVE); });

  webServer.on("/api/manual/stop", HTTP_POST, []() { handleManual(MOTOR_STOP); });
  webServer.on("/api/manual/forward", HTTP_POST, []() { handleManual(MOTOR_FORWARD); });
  webServer.on("/api/manual/backward", HTTP_POST, []() { handleManual(MOTOR_BACKWARD); });
  webServer.on("/api/manual/left", HTTP_POST, []() { handleManual(MOTOR_LEFT); });
  webServer.on("/api/manual/right", HTTP_POST, []() { handleManual(MOTOR_RIGHT); });

  webServer.on("/api/face/normal", HTTP_POST, []() { handleFace(FACE_NORMAL); });
  webServer.on("/api/face/happy", HTTP_POST, []() { handleFace(FACE_HAPPY); });
  webServer.on("/api/face/angry", HTTP_POST, []() { handleFace(FACE_ANGRY); });
  webServer.on("/api/face/sad", HTTP_POST, []() { handleFace(FACE_SAD); });
  webServer.on("/api/face/love", HTTP_POST, []() { handleFace(FACE_LOVE); });
  webServer.on("/api/face/surprise", HTTP_POST, []() { handleFace(FACE_SURPRISE); });
  webServer.on("/api/face/sleep", HTTP_POST, []() { handleFace(FACE_SLEEP); });

  webServer.on("/api/game/stop", HTTP_POST, []() { gamesStop(); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/tictactoe", HTTP_POST, []() { handleGame(GAME_TICTACTOE); });
  webServer.on("/api/game/catch", HTTP_POST, []() { handleGame(GAME_CATCH_ME); });

  webServer.on("/api/game/ttt/0", HTTP_POST, []() { gamesAction(0); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/1", HTTP_POST, []() { gamesAction(1); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/2", HTTP_POST, []() { gamesAction(2); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/3", HTTP_POST, []() { gamesAction(3); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/4", HTTP_POST, []() { gamesAction(4); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/5", HTTP_POST, []() { gamesAction(5); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/6", HTTP_POST, []() { gamesAction(6); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/7", HTTP_POST, []() { gamesAction(7); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/8", HTTP_POST, []() { gamesAction(8); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/play", HTTP_POST, []() { gamesAction(20); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/ttt/reset", HTTP_POST, []() { gamesAction(21); webServer.send(200, "application/json", "{\"ok\":true}"); });

  webServer.on("/api/game/catch/up", HTTP_POST, []() { gamesAction(1); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/catch/down", HTTP_POST, []() { gamesAction(2); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/catch/left", HTTP_POST, []() { gamesAction(3); webServer.send(200, "application/json", "{\"ok\":true}"); });
  webServer.on("/api/game/catch/right", HTTP_POST, []() { gamesAction(4); webServer.send(200, "application/json", "{\"ok\":true}"); });

  webServer.onNotFound(notFound);
  webServer.begin();
}

void webUIUpdate() {
  webServer.handleClient();
}
