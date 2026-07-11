#include "DisplayTask.h"

#include "../../MkbdTaskHooks.h"

void mkbdTask100msDisplayRun() {
  drawCurrentScreen();
  lastDisplayTime = millis();
}
