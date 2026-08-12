#include "zuno_core.h"

Scheduler scheduler;

Scheduler::Scheduler() : count(0) {}

bool Scheduler::add(TaskCallback callback, uint32_t intervalMs) {
  if (count >= MAX_TASKS || callback == nullptr || intervalMs == 0) {
    return false;
  }

  tasks[count].callback = callback;
  tasks[count].intervalMs = intervalMs;
  tasks[count].lastRunMs = millis();
  tasks[count].enabled = true;
  count++;
  return true;
}

void Scheduler::update() {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < count; i++) {
    ScheduledTask &task = tasks[i];
    if (!task.enabled) {
      continue;
    }
    if ((uint32_t)(now - task.lastRunMs) >= task.intervalMs) {
      task.lastRunMs = now;
      task.callback();
    }
  }
}

void Scheduler::reset() {
  count = 0;
}
