#include "MkbdScheduler.h"

#include "../GDS.h"
#include "task10ms/can/CanRxTask.h"
#include "task10ms/input/InputTask.h"
#include "task10ms/output/OutputTask.h"
#include "task100ms/display/DisplayTask.h"

static unsigned long lastTask10ms = 0;
static unsigned long lastTask100ms = 0;

static uint8_t isDue(unsigned long now, unsigned long& lastRun, unsigned long periodMs) {
  if (now - lastRun < periodMs) {
    return 0;
  }

  lastRun = now;
  return 1;
}

void mkbdSchedulerRun() {
  unsigned long now = millis();

  if (isDue(now, lastTask10ms, 10)) {
    mkbdTask10msInputRun();
    mkbdTask10msCanRxRun();
    mkbdTask10msOutputRun();
  }

  if (isDue(now, lastTask100ms, GDS_DISPLAY_INTERVAL_MS)) {
    mkbdTask100msDisplayRun();
  }
}
