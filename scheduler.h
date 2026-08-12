#pragma once

#include <Arduino.h>

typedef void (*ZunoTaskCallback)();

struct ZunoTask {
  ZunoTaskCallback callback;
  uint32_t intervalMs;
  uint32_t lastRunMs;
  bool enabled;
};

class ZunoScheduler {
 public:
  ZunoScheduler();

  bool addTask(ZunoTaskCallback callback, uint32_t intervalMs, bool runImmediately = false);
  void update(uint32_t nowMs);
  void clear();
  uint8_t taskCount() const;

 private:
  static constexpr uint8_t maxTasks = 10;
  ZunoTask tasks_[maxTasks];
  uint8_t count_;
};