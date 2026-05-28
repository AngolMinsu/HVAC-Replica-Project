#include "HeadUnitHmi.h"

#include <Arduino.h>

#include "../driver/DisplayDriver.h"

extern "C" {
#include "../generated/squareline/ui.h"
}

static uint32_t lastClockUpdateMs = 0;

void headUnitHmiBegin() {
  if (displayDriverLock(-1)) {
    ui_init();
    headUnitHmiUpdateClock();
    lv_refr_now(NULL);
    displayDriverUnlock();
  }
}

void headUnitHmiUpdateClock() {
  uint32_t now = millis();
  uint32_t totalSeconds = now / 1000;
  uint32_t minutes = (totalSeconds / 60) % 60;
  uint32_t hours = (totalSeconds / 3600) % 24;

  char timeText[8];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu", (unsigned long)hours, (unsigned long)minutes);

  if (ui_Time != NULL) lv_label_set_text(ui_Time, timeText);
  if (ui_CurTime != NULL) lv_label_set_text(ui_CurTime, timeText);
}

void headUnitHmiTick() {
  uint32_t now = millis();
  if (now - lastClockUpdateMs < 1000) return;
  lastClockUpdateMs = now;

  if (displayDriverLock(20)) {
    headUnitHmiUpdateClock();
    displayDriverUnlock();
  }
}
