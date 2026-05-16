#include <Wire.h>
#include <U8g2lib.h>

#include "GDS.h"
#include "state/State.h"
#include "display/Datc.h"
#include "display/Info.h"
#include "app/AppLogic.h"
#include "button/ButtonInput.h"
#include "encoder/EncoderInput.h"
#include "can/CanDriver.h"
#include "can/CanHandler.h"
#include "can/CanMonitor.h"
#include "can/CanProtocol.h"

// =====================================================
// SH1106 OLED + DATC/INFO MODE DEMO
// Board : Arduino Uno R4
// OLED  : SH1106 I2C
// Fan   : 5V 2Pin DC Fan + 2N2222A Transistor
// =====================================================

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// ===== 핀 맵 =====
#define DRIVER_ENC_A     GDS_PIN_DRIVER_ENC_A
#define DRIVER_ENC_B     GDS_PIN_DRIVER_ENC_B
#define DRIVER_ENC_SW    GDS_PIN_DRIVER_ENC_SW
#define BTN_FAN_UP       GDS_PIN_BTN_FAN_UP
#define BTN_FAN_DOWN     GDS_PIN_BTN_FAN_DOWN
#define BTN_SCREEN       GDS_PIN_BTN_SCREEN
#define PSG_ENC_A        GDS_PIN_PASSENGER_ENC_A
#define PSG_ENC_B        GDS_PIN_PASSENGER_ENC_B
#define PSG_ENC_SW       GDS_PIN_PASSENGER_ENC_SW
#define BTN_WIND_MEDIA   GDS_PIN_BTN_WIND_MEDIA

#define FAN_MOTOR   GDS_PIN_FAN_MOTOR
#define CAN_CS      GDS_PIN_CAN_CS

// ===== 시스템 상태 =====
SystemState state;

// ===== 버튼 이전 상태 =====
ButtonHistory buttonHistory;
EncoderHistory encoderHistory;

// ===== 디바운싱 =====
const unsigned long debounceDelay = GDS_DEBOUNCE_DELAY_MS;
const unsigned long encoderDebounceDelay = GDS_ENCODER_DEBOUNCE_DELAY_MS;

// ===== 화면 갱신 =====
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = GDS_DISPLAY_INTERVAL_MS;

uint8_t canTxCounter = 0;

ButtonLevels readButtonLevels();
uint8_t readButtonEvent();
EncoderLevels readEncoderLevels();
uint8_t readEncoderEvent();
int updateFanMotor();
uint8_t drawCurrentScreen();
uint8_t printSystemStatus(const SystemState& s);
uint8_t setupCan();
uint8_t processCanReceive();
uint8_t sendCanPayload(uint16_t id, const CanPayload& payload);
uint8_t broadcastHvacStatus(uint8_t signal);
uint8_t broadcastEncoderSwitchEvent(uint8_t encoderEvent);
uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after);
uint8_t broadcastIfSignalChanged(const SystemState& before, const SystemState& after, uint8_t signal);

void setup() {
  pinMode(DRIVER_ENC_A, INPUT_PULLUP);
  pinMode(DRIVER_ENC_B, INPUT_PULLUP);
  pinMode(DRIVER_ENC_SW, INPUT_PULLUP);
  pinMode(BTN_FAN_UP, INPUT_PULLUP);
  pinMode(BTN_FAN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SCREEN, INPUT_PULLUP);
  pinMode(PSG_ENC_A, INPUT_PULLUP);
  pinMode(PSG_ENC_B, INPUT_PULLUP);
  pinMode(PSG_ENC_SW, INPUT_PULLUP);
  pinMode(BTN_WIND_MEDIA, INPUT_PULLUP);

  pinMode(FAN_MOTOR, OUTPUT);

  analogWrite(FAN_MOTOR, 0);

  Serial.begin(9600);

  Wire.begin();
  Wire.setClock(100000);
  u8g2.begin();
  u8g2.setBusClock(100000);

  setupCan();

  initSystemState(state);
  initButtonHistory(buttonHistory);
  initEncoderHistory(encoderHistory);

  drawCurrentScreen();
  printSystemStatus(state);
}

void loop() {
  uint8_t button = readButtonEvent();
  if (button != APP_BUTTON_NONE) {
    SystemState before = state;
    uint8_t changed = handleButtonAction(state, button);
    if (changed) {
      printSystemStatus(state);
      broadcastChangedHvacStatus(before, state);
    }
  }

  uint8_t encoderEvent = readEncoderEvent();
  if (encoderEvent != ENCODER_EVENT_NONE) {
    SystemState before = state;
    uint8_t changed = handleEncoderAction(state, encoderEvent);
    if (changed) {
      printSystemStatus(state);
      broadcastChangedHvacStatus(before, state);
    }
    broadcastEncoderSwitchEvent(encoderEvent);
  }

  if (processCanReceive()) {
    printSystemStatus(state);
  }

  // 화면이 INFO여도 공조 장치는 계속 동작해야 하므로 항상 실행
  updateFanMotor();

  if (shouldRefreshDisplay(millis(), lastDisplayTime, displayInterval)) {
    drawCurrentScreen();
    lastDisplayTime = millis();
  }
}

ButtonLevels readButtonLevels() {
  ButtonLevels levels;
  levels.fanUp = digitalRead(BTN_FAN_UP);
  levels.fanDown = digitalRead(BTN_FAN_DOWN);
  levels.screen = digitalRead(BTN_SCREEN);
  levels.windMedia = digitalRead(BTN_WIND_MEDIA);

  return levels;
}

uint8_t readButtonEvent() {
  ButtonLevels levels = readButtonLevels();
  return detectButtonEvent(buttonHistory, levels, millis(), debounceDelay);
}

EncoderLevels readEncoderLevels() {
  EncoderLevels levels;
  levels.driverA = digitalRead(DRIVER_ENC_A);
  levels.driverB = digitalRead(DRIVER_ENC_B);
  levels.driverSw = digitalRead(DRIVER_ENC_SW);
  levels.passengerA = digitalRead(PSG_ENC_A);
  levels.passengerB = digitalRead(PSG_ENC_B);
  levels.passengerSw = digitalRead(PSG_ENC_SW);

  return levels;
}

uint8_t readEncoderEvent() {
  EncoderLevels levels = readEncoderLevels();
  return detectEncoderEvent(encoderHistory, levels, millis(), encoderDebounceDelay);
}

int updateFanMotor() {
  int pwmValue = calculateFanPwm(state);

  analogWrite(FAN_MOTOR, pwmValue);

  return pwmValue;
}

uint8_t drawCurrentScreen() {
  if (state.screenMode == SCREEN_DATC) {
    datcDrawScreen(u8g2, state);
  } else {
    infoDrawScreen(u8g2, state);
  }

  return state.screenMode;
}

uint8_t printSystemStatus(const SystemState& s) {
  Serial.print("SCREEN:");
  Serial.print(screenModeToText(s.screenMode));
  Serial.print(" | HVAC:");
  Serial.print(hvacModeToText(s.hvacMode));
  Serial.print(" | FAN:");
  Serial.print(s.fanSpeed);
  Serial.print(" | DRV:");
  Serial.print(s.driverTemp);
  Serial.print(" | PSG:");
  Serial.print(s.passengerTemp);
  Serial.print(" | AIR:");
  Serial.print(windModeToText(s.windMode));
  Serial.print(" | VOLUME:");
  Serial.print(s.volume);
  Serial.print(" | MUTE:");
  Serial.print(s.mute ? "ON" : "OFF");
  Serial.print(" | MEDIA:");
  Serial.println(s.mediaMode ? "ON" : "OFF");

  return 1;
}

uint8_t setupCan() {
  uint8_t ready = canDriverBegin(CAN_CS);

  Serial.print("CAN:");
  Serial.println(ready ? "READY" : "FAIL");

  return ready;
}

uint8_t processCanReceive() {
  CanFrame rxFrame;
  if (!canDriverReceive(rxFrame)) {
    return 0;
  }

  canMonitorPrintFrame("RX", rxFrame);

  if (rxFrame.id != CAN_ID_CONTROL_REQUEST) {
    Serial.println("CAN ERROR: UNKNOWN ID");
    return 0;
  }

  if (rxFrame.dlc != CAN_DLC) {
    Serial.println("CAN ERROR: INVALID DLC");
    return 0;
  }

  CanPayload request = canPayloadFromBytes(rxFrame.data);
  CanPayload response;
  SystemState before = state;
  uint8_t changed = canProcessControlRequest(state, request, response);

  if (!sendCanPayload(CAN_ID_CONTROL_RESPONSE, response)) {
    Serial.println("CAN ERROR: SEND FAIL");
  }

  if (changed) {
    broadcastChangedHvacStatus(before, state);
  }

  return changed;
}

uint8_t sendCanPayload(uint16_t id, const CanPayload& payload) {
  CanFrame frame;
  frame.id = id;
  frame.dlc = CAN_DLC;
  canPayloadToBytes(payload, frame.data);

  canMonitorPrintFrame("TX", frame);

  return canDriverSend(frame);
}

uint8_t broadcastHvacStatus(uint8_t signal) {
  CanPayload payload = canMakeStatusPayload(state, signal, canTxCounter++);

  return sendCanPayload(CAN_ID_HVAC_STATUS, payload);
}

uint8_t broadcastEncoderSwitchEvent(uint8_t encoderEvent) {
  if (encoderEvent == ENCODER_EVENT_DRIVER_SW) {
    return broadcastHvacStatus(CAN_SIGNAL_DRIVER_ENCODER_SW);
  }

  if (encoderEvent == ENCODER_EVENT_PASSENGER_SW) {
    return broadcastHvacStatus(CAN_SIGNAL_PASSENGER_ENCODER_SW);
  }

  return 0;
}

uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after) {
  uint8_t sentCount = 0;

  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_POWER);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_FAN_SPEED);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_TEMPERATURE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_PASSENGER_TEMPERATURE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MODE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_AC);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_AUTO);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_SCREEN_MODE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MEDIA);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_VOLUME);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MAP);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MUTE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_HOME);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MEDIA_MODE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MEDIA_INDEX);

  return sentCount;
}

uint8_t broadcastIfSignalChanged(const SystemState& before, const SystemState& after, uint8_t signal) {
  uint8_t beforeValue = 0;
  uint8_t afterValue = 0;

  if (!canSignalValueFromState(before, signal, beforeValue) ||
      !canSignalValueFromState(after, signal, afterValue)) {
    return 0;
  }

  if (beforeValue == afterValue) {
    return 0;
  }

  return broadcastHvacStatus(signal);
}

#include "state/State.cpp"
#include "display/Datc.cpp"
#include "display/Info.cpp"
#include "app/AppLogic.cpp"
#include "button/ButtonInput.cpp"
#include "encoder/EncoderInput.cpp"
#include "can/CanProtocol.cpp"
#include "can/CanHandler.cpp"
#include "can/CanMonitor.cpp"
#include "can/CanDriver.cpp"
