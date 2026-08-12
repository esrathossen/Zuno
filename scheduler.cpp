#include "scheduler.h"

ZunoScheduler::ZunoScheduler() : count_(0) {
  clear();
}

bool ZunoScheduler::addTask(ZunoTaskCallback callback, uint32_t intervalMs, bool runImmediately) {
  if (callback == nullptr || intervalMs == 0 || count_ >= maxTasks) {
    return false;
  }

  ZunoTask &task = tasks_[count_++];
  task.callback = callback;
  task.intervalMs = intervalMs;
  task.lastRunMs = runImmediately ? millis() - intervalMs : millis();
  task.enabled = true;
  return true;
}

void ZunoScheduler::update(uint32_t nowMs) {
  for (uint8_t i = 0; i < count_; ++i) {
    ZunoTask &task = tasks_[i];
    if (!task.enabled || task.callback == nullptr) {
      continue;
    }

    if ((uint32_t)(nowMs - task.lastRunMs) >= task.intervalMs) {
      task.lastRunMs = nowMs;
      task.callback();
    }
  }
}

void ZunoScheduler::clear() {
  for (uint8_t i = 0; i < maxTasks; ++i) {
    tasks_[i].callback = nullptr;
    tasks_[i].intervalMs = 0;
    tasks_[i].lastRunMs = 0;
    tasks_[i].enabled = false;
  }
  count_ = 0;
}

uint8_t ZunoScheduler::taskCount() const {
  return count_;
}