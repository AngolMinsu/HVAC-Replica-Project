#include "HeadUnitHmi.h"
#include "DynamicImageUi.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/CanDriver.h"
#include "../can/CanMonitor.h"
#include "../driver/DisplayDriver.h"
#include "../ota/OtaManager.h"
#include "WifiManager.h"

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
static uint32_t lastWifiRevision = UINT32_MAX;
static uint32_t lastWifiNetworkRevision = UINT32_MAX;
static uint32_t lastOtaRevision = UINT32_MAX;
static HeadUnitWifiNetwork wifiUiNetworks[16] = {};
static size_t wifiUiNetworkCount = 0;

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

static void setDisabled(lv_obj_t* object, bool disabled) {
  if (object == NULL) return;
  if (disabled) {
    lv_obj_add_state(object, LV_STATE_DISABLED);
  } else {
    lv_obj_clear_state(object, LV_STATE_DISABLED);
  }
}

static void refreshWifiOptions() {
  uint32_t networkRevision = wifiManagerGetNetworkRevision();
  if (networkRevision == lastWifiNetworkRevision || ui_Dropdown1 == NULL) return;
  lastWifiNetworkRevision = networkRevision;
  wifiUiNetworkCount = 0;

  char options[1024];
  size_t used = snprintf(options, sizeof(options), "Select network");
  for (size_t index = 0; index < wifiManagerGetNetworkCount() &&
                         wifiUiNetworkCount < 16 && used < sizeof(options); index++) {
    HeadUnitWifiNetwork network = {};
    if (!wifiManagerGetNetwork(index, &network)) continue;
    wifiUiNetworks[wifiUiNetworkCount++] = network;
    int written = snprintf(options + used, sizeof(options) - used, "\n%s  (%ld dBm)%s",
                           network.ssid, static_cast<long>(network.rssi), network.secured ? " *" : "");
    if (written < 0) break;
    size_t added = static_cast<size_t>(written);
    if (added >= sizeof(options) - used) {
      used = sizeof(options) - 1;
      break;
    }
    used += added;
  }

  lv_dropdown_set_options(ui_Dropdown1, options);
  lv_dropdown_set_selected(ui_Dropdown1, 0);
}

static void refreshWifiUi() {
  HeadUnitWifiState wifiState = wifiManagerGetState();
  bool enabled = wifiState != HEAD_UNIT_WIFI_OFF;
  bool scanning = wifiState == HEAD_UNIT_WIFI_SCANNING;
  bool connecting = wifiState == HEAD_UNIT_WIFI_CONNECTING;
  bool connected = wifiState == HEAD_UNIT_WIFI_CONNECTED;
  bool hasNetworks = wifiManagerGetNetworkCount() > 0;

  refreshWifiOptions();

  if (ui_SwitchWiFiOnOff != NULL) {
    if (enabled) {
      lv_obj_add_state(ui_SwitchWiFiOnOff, LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(ui_SwitchWiFiOnOff, LV_STATE_CHECKED);
    }
  }

  setDisabled(ui_BtnSearchAP, !enabled || scanning || connecting || connected);
  setDisabled(ui_Dropdown1, !enabled || !hasNetworks || connecting || connected);
  setDisabled(ui_TAPW, !enabled || connecting || connected);
  setDisabled(ui_BtnShow, !enabled || connecting || connected);
  setDisabled(ui_BtnConnect, !enabled || (!hasNetworks && !connected) || connecting);

  if (ui_TextConnect1 != NULL && ui_TextDiscon != NULL) {
    if (connected) {
      lv_obj_add_flag(ui_TextConnect1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_TextDiscon, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(ui_TextConnect1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_TextDiscon, LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (connected && ui_TAPW != NULL) {
    lv_textarea_set_text(ui_TAPW, "");
  }

  char statusText[64];
  switch (wifiState) {
    case HEAD_UNIT_WIFI_OFF:
      snprintf(statusText, sizeof(statusText), "Wi-Fi Off");
      break;
    case HEAD_UNIT_WIFI_IDLE:
      snprintf(statusText, sizeof(statusText), hasNetworks ? "Select network" : "No networks found");
      break;
    case HEAD_UNIT_WIFI_SCANNING:
      snprintf(statusText, sizeof(statusText), "Scanning...");
      break;
    case HEAD_UNIT_WIFI_CONNECTING:
      snprintf(statusText, sizeof(statusText), "Connecting...");
      break;
    case HEAD_UNIT_WIFI_CONNECTED: {
      char ssid[33];
      wifiManagerGetConnectedSsid(ssid, sizeof(ssid));
      snprintf(statusText, sizeof(statusText), "Connected: %.32s", ssid);
      break;
    }
    case HEAD_UNIT_WIFI_FAILED:
    default:
      snprintf(statusText, sizeof(statusText), "Connection failed");
      break;
  }
  setTextIfReady(ui_TextSearching, statusText);

  if ((!enabled || connected) && ui_Keyboard2 != NULL) {
    lv_obj_add_flag(ui_Keyboard2, LV_OBJ_FLAG_HIDDEN);
  }
}

static void onWifiSwitch(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;
  wifiManagerRequestEnabled(lv_obj_has_state(ui_SwitchWiFiOnOff, LV_STATE_CHECKED));
}

static void onWifiSearch(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  wifiManagerRequestScan();
}

static void onWifiSelection(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;
  uint16_t selected = lv_dropdown_get_selected(ui_Dropdown1);
  if (selected == 0) {
    setTextIfReady(ui_TextSearching, "Select network");
    return;
  }

  HeadUnitWifiNetwork network = {};
  if (selected <= wifiUiNetworkCount) {
    network = wifiUiNetworks[selected - 1];
    lv_textarea_set_text(ui_TAPW, "");
    setTextIfReady(ui_TextSearching, network.secured ? "Password required" : "Open network");
  }
}

static void onWifiPassword(lv_event_t* event) {
  lv_event_code_t code = lv_event_get_code(event);
  if (code != LV_EVENT_CLICKED && code != LV_EVENT_FOCUSED) return;
  lv_keyboard_set_textarea(ui_Keyboard2, ui_TAPW);
  lv_obj_clear_flag(ui_Keyboard2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(ui_Keyboard2);
}

static void onWifiKeyboard(lv_event_t* event) {
  lv_event_code_t code = lv_event_get_code(event);
  if (code != LV_EVENT_READY && code != LV_EVENT_CANCEL) return;
  lv_obj_add_flag(ui_Keyboard2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_state(ui_TAPW, LV_STATE_FOCUSED);
}

static void onWifiShowPassword(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  bool hidden = lv_textarea_get_password_mode(ui_TAPW);
  lv_textarea_set_password_mode(ui_TAPW, !hidden);
  setTextIfReady(ui_TextShow, hidden ? "Hide" : "Show");
}

static void onWifiConnect(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (ui_TextDiscon != NULL && !lv_obj_has_flag(ui_TextDiscon, LV_OBJ_FLAG_HIDDEN)) {
    wifiManagerRequestDisconnect();
    return;
  }

  uint16_t selected = lv_dropdown_get_selected(ui_Dropdown1);
  HeadUnitWifiNetwork network = {};
  if (selected == 0 || selected > wifiUiNetworkCount) {
    setTextIfReady(ui_TextSearching, "Select network");
    return;
  }
  network = wifiUiNetworks[selected - 1];

  wifiManagerRequestConnect(network.ssid, network.secured ? lv_textarea_get_text(ui_TAPW) : "");
}

static void bindWifiEvents() {
  lv_obj_add_event_cb(ui_SwitchWiFiOnOff, onWifiSwitch, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_BtnSearchAP, onWifiSearch, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_Dropdown1, onWifiSelection, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_TAPW, onWifiPassword, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_Keyboard2, onWifiKeyboard, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_BtnShow, onWifiShowPassword, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnConnect, onWifiConnect, LV_EVENT_CLICKED, NULL);
}

static void addButtonSymbol(lv_obj_t* button, const char* symbol) {
  if (button == NULL || symbol == NULL || lv_obj_get_child_cnt(button) != 0) return;
  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, symbol);
  lv_obj_center(label);
}

static void bindNavigationSymbols() {
  lv_obj_t* backButtons[] = {ui_BackBtn, ui_BackBtn1, ui_BackBtn2, ui_BackBtn3, ui_BackBtn4, ui_BackBtn5};
  lv_obj_t* homeButtons[] = {ui_HomeBtn, ui_HomeBtn1, ui_HomeBtn2, ui_HomeBtn3, ui_HomeBtn4, ui_HomeBtn5};
  for (lv_obj_t* button : backButtons) addButtonSymbol(button, LV_SYMBOL_LEFT);
  for (lv_obj_t* button : homeButtons) addButtonSymbol(button, LV_SYMBOL_HOME);
}

static const char* otaStateText(hu7::ota::OtaState state) {
  switch (state) {
    case hu7::ota::OtaState::Connecting: return "Connecting";
    case hu7::ota::OtaState::CheckingVersion: return "Checking";
    case hu7::ota::OtaState::UpdateAvailable: return "Available";
    case hu7::ota::OtaState::Downloading: return "Downloading";
    case hu7::ota::OtaState::Verifying: return "Verifying";
    case hu7::ota::OtaState::Rebooting: return "Rebooting";
    case hu7::ota::OtaState::Failed: return "Failed";
    default: return "Idle";
  }
}

static void refreshOtaUi(const hu7::ota::Snapshot& snapshot) {
  char valueText[32];
  setTextIfReady(ui_TextCurVer2, snapshot.currentVersion[0] ? snapshot.currentVersion : "-");
  setTextIfReady(ui_TextLatVer2, snapshot.latestVersion[0] ? snapshot.latestVersion : "-");
  setTextIfReady(ui_TextOTAServer2, snapshot.serverStatus);
  setTextIfReady(ui_TextStatus2, snapshot.targetStatus);
  setTextIfReady(ui_TextPackTarget, snapshot.packageTarget[0] ? snapshot.packageTarget : "-");
  setTextIfReady(ui_ValueUpdateStatus, otaStateText(snapshot.state));
  setTextIfReady(ui_TextErr, snapshot.message);

  if (snapshot.firmwareSize == 0) {
    setTextIfReady(ui_TextFirmSize2, "-");
  } else {
    snprintf(valueText, sizeof(valueText), "%lu KB",
             static_cast<unsigned long>((snapshot.firmwareSize + 1023) / 1024));
    setTextIfReady(ui_TextFirmSize2, valueText);
  }

  snprintf(valueText, sizeof(valueText), "%u %%", snapshot.progress);
  setTextIfReady(ui_TextUpdatePercent, valueText);
  if (ui_BarUpdateProgress != NULL) {
    lv_bar_set_value(ui_BarUpdateProgress, snapshot.progress, LV_ANIM_OFF);
  }

  const bool busy = snapshot.state == hu7::ota::OtaState::Connecting ||
                    snapshot.state == hu7::ota::OtaState::CheckingVersion ||
                    snapshot.state == hu7::ota::OtaState::Downloading ||
                    snapshot.state == hu7::ota::OtaState::Verifying ||
                    snapshot.state == hu7::ota::OtaState::Rebooting;
  setDisabled(ui_DropdownTarget, busy);
  setDisabled(ui_ButtonRefresh, busy || snapshot.selectedTarget != hu7::ota::UpdateTarget::HU7);
  setDisabled(ui_ButtonUpdate, busy || !snapshot.updateAvailable);
  if (ui_ButtonCancel != NULL) lv_obj_add_flag(ui_ButtonCancel, LV_OBJ_FLAG_HIDDEN);
}

static void onOtaTargetSelection(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;
  const uint16_t selected = lv_dropdown_get_selected(ui_DropdownTarget);
  hu7::ota::UpdateTarget target = hu7::ota::UpdateTarget::None;
  if (selected <= static_cast<uint16_t>(hu7::ota::UpdateTarget::BMS)) {
    target = static_cast<hu7::ota::UpdateTarget>(selected);
  }
  hu7::ota::otaManagerSelectTarget(target);
}

static void onOtaCheck(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (!hu7::ota::otaManagerRequestCheck()) {
    setTextIfReady(ui_TextErr, "OTA request queue is busy");
  }
}

static void onOtaUpdate(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  if (!hu7::ota::otaManagerRequestUpdate()) {
    setTextIfReady(ui_TextErr, "Check update again");
  }
}

static void onOpenSettingConnect(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  _ui_screen_change(&ui_SettingConnect, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_SettingConnect_screen_init);
  headUnitHmiUpdateClock();
  hu7::ota::Snapshot snapshot{};
  if (hu7::ota::otaManagerGetSnapshot(snapshot)) refreshOtaUi(snapshot);
}

static void bindSettingConnectEvents() {
  lv_obj_add_event_cb(ui_CardConnect, onOpenSettingConnect, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_DropdownTarget, onOtaTargetSelection, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_ButtonRefresh, onOtaCheck, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_ButtonUpdate, onOtaUpdate, LV_EVENT_CLICKED, NULL);
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

static void onFanSpeedPrevious(lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
  fanSpeed = (fanSpeed == 0) ? GDS_FAN_SPEED_MAX : fanSpeed - 1;
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

}

static void bindMkbdEvents() {
  lv_obj_add_event_cb(ui_BtnPsgTpDn2, onDriverTempDown, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnPsgTpUp2, onDriverTempUp, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnPsgTpDn, onPassengerTempDown, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnPsgTpUp, onPassengerTempUp, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnSpdDn, onFanSpeedPrevious, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnSpdUp, onFanSpeedNext, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_BtnChMd, onWindModeNext, LV_EVENT_CLICKED, NULL);
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
    dynamicImageUiBegin();
    lv_disp_load_scr(ui_Main);
    bindMkbdEvents();
    bindWifiEvents();
    bindNavigationSymbols();
    bindSettingConnectEvents();
    refreshWifiUi();
    hu7::ota::Snapshot otaSnapshot{};
    if (hu7::ota::otaManagerGetSnapshot(otaSnapshot)) refreshOtaUi(otaSnapshot);
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
  if (ui_Time3 != NULL) lv_label_set_text(ui_Time3, timeText);
  if (ui_Time5 != NULL) lv_label_set_text(ui_Time5, timeText);
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
  loadScreen(&ui_SettingMKBD, LV_SCR_LOAD_ANIM_NONE, ui_SettingMKBD_screen_init);
}

void headUnitHmiTick() {
  wifiManagerTick();

  uint32_t now = millis();
  bool updateClock = now - lastClockUpdateMs >= 1000;
  bool updateWifi = wifiManagerGetRevision() != lastWifiRevision;
  hu7::ota::Snapshot otaSnapshot{};
  bool updateOta = hu7::ota::otaManagerGetSnapshot(otaSnapshot) &&
                   otaSnapshot.revision != lastOtaRevision;
  if (!updateClock && !updateWifi && !updateOta) return;

  if (displayDriverLock(20)) {
    if (updateClock) {
      lastClockUpdateMs = now;
      headUnitHmiUpdateClock();
    }
    if (updateWifi) {
      lastWifiRevision = wifiManagerGetRevision();
      refreshWifiUi();
    }
    if (updateOta) {
      lastOtaRevision = otaSnapshot.revision;
      refreshOtaUi(otaSnapshot);
    }
    displayDriverUnlock();
  }
}
