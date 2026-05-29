#include "HeadUnitHmi.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../driver/DisplayDriver.h"

extern "C" {
#include "../generated/squareline/ui.h"
}

static uint32_t lastClockUpdateMs = 0;
static uint8_t driverTemp = 24;
static uint8_t passengerTemp = 24;
static uint8_t fanSpeed = 0;
static uint8_t windMode = 0;

static const char* windModeToText(uint8_t value) {
  switch (value) {
    case 0: return "FACE";
    case 1: return "FOOT";
    case 2: return "DEF";
    case 3: return "MIX";
    default: return "-";
  }
}

static void setTextIfReady(lv_obj_t* label, const char* text) {
  if (label != NULL) {
    lv_label_set_text(label, text);
  }
}

static void refreshHvacInfoLabels() {
  char valueText[12];

  snprintf(valueText, sizeof(valueText), "%u", driverTemp);
  setTextIfReady(ui_TxtCurDrvTmp1, valueText);

  snprintf(valueText, sizeof(valueText), "%u", passengerTemp);
  setTextIfReady(ui_TxtCurPsgTmp2, valueText);

  snprintf(valueText, sizeof(valueText), "%u", fanSpeed);
  setTextIfReady(ui_TxtCurPsgTmp3, valueText);

  setTextIfReady(ui_TextCurMode1, windModeToText(windMode));
}

static void loadScreen(lv_obj_t** screen, lv_scr_load_anim_t anim, void (*init)(void)) {
  if (displayDriverLock(50)) {
    _ui_screen_change(screen, anim, 0, 0, init);
    headUnitHmiUpdateClock();
    refreshHvacInfoLabels();
    displayDriverUnlock();
  }
}

void headUnitHmiBegin() {
  if (displayDriverLock(-1)) {
    ui_init();
    headUnitHmiUpdateClock();
    refreshHvacInfoLabels();
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
  if (ui_Time1 != NULL) lv_label_set_text(ui_Time1, timeText);
  if (ui_Time2 != NULL) lv_label_set_text(ui_Time2, timeText);
  if (ui_CurTime != NULL) lv_label_set_text(ui_CurTime, timeText);
}

void headUnitHmiApplyCanPayload(const CanPayload& payload) {
  if (!canValidateChecksum(payload) || payload.result == GDS_CAN_RESULT_FAIL) {
    return;
  }

  uint8_t signal = payload.signal;
  uint8_t value = payload.value;

  switch (signal) {
    case GDS_CAN_SIGNAL_TEMPERATURE:
      driverTemp = value;
      break;
    case GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE:
      passengerTemp = value;
      break;
    case GDS_CAN_SIGNAL_FAN_SPEED:
      fanSpeed = value;
      break;
    case GDS_CAN_SIGNAL_MODE:
      windMode = value;
      break;
    default:
      return;
  }

  if (displayDriverLock(20)) {
    refreshHvacInfoLabels();
    displayDriverUnlock();
  }
}

void headUnitHmiOpenHome() {
  loadScreen(&ui_Main, LV_SCR_LOAD_ANIM_NONE, ui_Main_screen_init);
}

void headUnitHmiOpenMap() {
  loadScreen(&ui_MapScreen, LV_SCR_LOAD_ANIM_NONE, ui_MapScreen_screen_init);
}

void headUnitHmiOpenSetting() {
  loadScreen(&ui_Setting, LV_SCR_LOAD_ANIM_NONE, ui_Setting_screen_init);
}

void headUnitHmiOpenSettingInfo() {
  loadScreen(&ui_Setting1, LV_SCR_LOAD_ANIM_NONE, ui_Setting1_screen_init);
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
