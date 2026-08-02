#include "HeadUnitHmi.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/CanDriver.h"
#include "../can/CanMonitor.h"
#include "../driver/DisplayDriver.h"

extern "C" {
#include "../generated/squareline/ui.h"
}

static uint32_t lastClockUpdateMs = 0;
static uint8_t driverTemp = 24;
static uint8_t passengerTemp = 24;
static uint8_t fanSpeed = 0;
static uint8_t windMode = 0;
static uint8_t volume = 10;
static uint8_t mute = 0;
static uint8_t mediaMode = 0;
static uint8_t mediaIndex = 0;
static uint8_t mediaReady = 0;
static uint8_t mapReady = 0;
static uint8_t huTxCounter = 0;
static const char* lastPsgControl = "-";

static lv_obj_t* infoPanelOwner = NULL;
static lv_obj_t* textVolume = NULL;
static lv_obj_t* valueVolume = NULL;
static lv_obj_t* textMute = NULL;
static lv_obj_t* valueMute = NULL;
static lv_obj_t* textMediaMode = NULL;
static lv_obj_t* valueMediaMode = NULL;
static lv_obj_t* textMediaIndex = NULL;
static lv_obj_t* valueMediaIndex = NULL;
static lv_obj_t* textMediaMap = NULL;
static lv_obj_t* valueMediaMap = NULL;
static lv_obj_t* textPsgControl = NULL;
static lv_obj_t* valuePsgControl = NULL;
static lv_obj_t* btnDriverTempDown = NULL;
static lv_obj_t* btnDriverTempUp = NULL;
static lv_obj_t* btnPassengerTempDown = NULL;
static lv_obj_t* btnPassengerTempUp = NULL;
static lv_obj_t* btnFanSpeed = NULL;
static lv_obj_t* btnWindMode = NULL;

static void refreshHvacInfoLabels();
static void refreshInfoModeLabels();

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
  if (label != NULL && text != NULL) {
    lv_label_set_text(label, text);
  }
}

static lv_obj_t* createInfoLabel(const char* text, int16_t x, int16_t y) {
  lv_obj_t* label = lv_label_create(ui_PanelInfo);
  lv_obj_set_width(label, LV_SIZE_CONTENT);
  lv_obj_set_height(label, LV_SIZE_CONTENT);
  lv_obj_set_x(label, x);
  lv_obj_set_y(label, y);
  lv_obj_set_align(label, LV_ALIGN_CENTER);
  lv_label_set_text(label, text);
  return label;
}

static lv_obj_t* createInfoButton(const char* text, int16_t x, int16_t y, lv_event_cb_t callback) {
  lv_obj_t* button = lv_btn_create(ui_PanelInfo);
  lv_obj_set_size(button, 92, 34);
  lv_obj_set_x(button, x);
  lv_obj_set_y(button, y);
  lv_obj_set_align(button, LV_ALIGN_CENTER);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_center(label);

  return button;
}

static uint8_t sendHuControlRequest(uint8_t signal, uint8_t value) {
  if (!canDriverIsReady()) {
    Serial.println("HU TX FAIL CAN:NOT_READY");
    return 0;
  }

  CanPayload payload;
  payload.service = GDS_CAN_SERVICE_WRITE_REQUEST;
  payload.result = GDS_CAN_RESULT_NORMAL;
  payload.signal = signal;
  payload.value = value;
  payload.option = 0;
  payload.reserved = 0;
  payload.counter = ++huTxCounter;
  payload.checksum = canCalculateChecksum(payload);

  CanFrame frame;
  frame.id = GDS_CAN_ID_CONTROL_REQUEST;
  frame.dlc = GDS_CAN_DLC;
  canPayloadToBytes(payload, frame.data);

  canMonitorPrintFrame("HU TX", frame);
  canMonitorPrintPayloadSummary(payload);

  uint8_t sent = canDriverSend(frame);
  Serial.println(sent ? "HU TX RESULT:OK" : "HU TX RESULT:FAIL");
  if (!sent) {
    canDriverPrintStatus("HU_TX_FAIL");
  }
  return sent;
}

static void sendAndRefresh(uint8_t signal, uint8_t value) {
  sendHuControlRequest(signal, value);
  refreshHvacInfoLabels();
  refreshInfoModeLabels();
}

static void onDriverTempDown(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (driverTemp > GDS_TEMP_MIN) driverTemp--;
  sendAndRefresh(GDS_CAN_SIGNAL_TEMPERATURE, driverTemp);
}

static void onDriverTempUp(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (driverTemp < GDS_TEMP_MAX) driverTemp++;
  sendAndRefresh(GDS_CAN_SIGNAL_TEMPERATURE, driverTemp);
}

static void onPassengerTempDown(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (passengerTemp > GDS_TEMP_MIN) passengerTemp--;
  sendAndRefresh(GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE, passengerTemp);
}

static void onPassengerTempUp(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (passengerTemp < GDS_TEMP_MAX) passengerTemp++;
  sendAndRefresh(GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE, passengerTemp);
}

static void onFanSpeedNext(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  fanSpeed = (fanSpeed >= GDS_FAN_SPEED_MAX) ? 0 : fanSpeed + 1;
  sendAndRefresh(GDS_CAN_SIGNAL_FAN_SPEED, fanSpeed);
}

static void onWindModeNext(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  windMode = (windMode >= 3) ? 0 : windMode + 1;
  sendAndRefresh(GDS_CAN_SIGNAL_MODE, windMode);
}

static void ensureInfoModeLabels() {
  if (ui_PanelInfo == NULL || infoPanelOwner == ui_PanelInfo) {
    return;
  }

  infoPanelOwner = ui_PanelInfo;
  textVolume = createInfoLabel("Volume", 20, -150);
  valueVolume = createInfoLabel("10", 170, -150);
  textMute = createInfoLabel("Mute", 20, -120);
  valueMute = createInfoLabel("OFF", 170, -120);
  textMediaMode = createInfoLabel("MediaMode", 20, -90);
  valueMediaMode = createInfoLabel("OFF", 170, -90);
  textMediaIndex = createInfoLabel("MediaIndex", 20, -60);
  valueMediaIndex = createInfoLabel("0", 170, -60);
  textMediaMap = createInfoLabel("Media / Map", 20, -30);
  valueMediaMap = createInfoLabel("0 / 0", 170, -30);
  textPsgControl = createInfoLabel("PSG Control", 20, 0);
  valuePsgControl = createInfoLabel("-", 170, 0);

  btnDriverTempDown = createInfoButton("DRV -", -310, 72, onDriverTempDown);
  btnDriverTempUp = createInfoButton("DRV +", -210, 72, onDriverTempUp);
  btnPassengerTempDown = createInfoButton("PSG -", -110, 72, onPassengerTempDown);
  btnPassengerTempUp = createInfoButton("PSG +", -10, 72, onPassengerTempUp);
  btnFanSpeed = createInfoButton("FAN", -310, 116, onFanSpeedNext);
  btnWindMode = createInfoButton("MODE", -210, 116, onWindModeNext);
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

static const char* onOffToText(uint8_t value) {
  return value ? "ON" : "OFF";
}

static void refreshInfoModeLabels() {
  ensureInfoModeLabels();

  char valueText[16];

  snprintf(valueText, sizeof(valueText), "%u", volume);
  setTextIfReady(valueVolume, valueText);

  setTextIfReady(valueMute, onOffToText(mute));
  setTextIfReady(valueMediaMode, onOffToText(mediaMode));

  snprintf(valueText, sizeof(valueText), "%u", mediaIndex);
  setTextIfReady(valueMediaIndex, valueText);

  snprintf(valueText, sizeof(valueText), "%u / %u", mediaReady, mapReady);
  setTextIfReady(valueMediaMap, valueText);

  setTextIfReady(valuePsgControl, lastPsgControl);
}

static void loadScreen(lv_obj_t** screen, lv_scr_load_anim_t anim, void (*init)(void)) {
  if (screen == NULL || init == NULL) {
    return;
  }

  if (displayDriverLock(50)) {
    _ui_screen_change(screen, anim, 0, 0, init);
    headUnitHmiUpdateClock();
    refreshHvacInfoLabels();
    refreshInfoModeLabels();
    displayDriverUnlock();
  }
}

void headUnitHmiBegin() {
  if (displayDriverLock(-1)) {
    ui_init();
    headUnitHmiUpdateClock();
    refreshHvacInfoLabels();
    refreshInfoModeLabels();
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
    case GDS_CAN_SIGNAL_VOLUME:
      volume = value;
      break;
    case GDS_CAN_SIGNAL_MUTE:
      mute = value;
      break;
    case GDS_CAN_SIGNAL_MEDIA_MODE:
      mediaMode = value;
      break;
    case GDS_CAN_SIGNAL_MEDIA_INDEX:
      mediaIndex = value;
      break;
    case GDS_CAN_SIGNAL_MEDIA:
      mediaReady = value;
      break;
    case GDS_CAN_SIGNAL_MAP:
      mapReady = value;
      break;
    case GDS_CAN_SIGNAL_HU_FOCUS_PREV:
      lastPsgControl = "PREV";
      break;
    case GDS_CAN_SIGNAL_HU_FOCUS_NEXT:
      lastPsgControl = "NEXT";
      break;
    case GDS_CAN_SIGNAL_PASSENGER_ENCODER_SW:
      lastPsgControl = "SELECT";
      break;
    default:
      return;
  }

  if (displayDriverLock(20)) {
    refreshHvacInfoLabels();
    refreshInfoModeLabels();
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
