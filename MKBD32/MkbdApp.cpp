#include "MkbdApp.h"

#include <Arduino.h>

#include "GDS.h"
#include "app/MkbdHardware.h"
#include "can/MkbdCanService.h"
#include "state/State.h"
#include "task/MkbdRtos.h"

SystemState state;

uint8_t readButtonEvent() {
  return mkbdHardwareReadButtonEvent();
}

uint8_t readEncoderEvent() {
  return mkbdHardwareReadEncoderEvent();
}

int updateFanMotor() {
  return mkbdHardwareUpdateFan(state);
}

uint8_t drawCurrentScreen() {
  return mkbdHardwareDrawDisplay(state);
}

uint8_t printSystemStatus(const SystemState& current) {
  Serial.print("SCREEN:");
  Serial.print(screenModeToText(current.screenMode));
  Serial.print(" | HVAC:");
  Serial.print(hvacModeToText(current.hvacMode));
  Serial.print(" | FAN:");
  Serial.print(current.fanSpeed);
  Serial.print(" | DRV:");
  Serial.print(current.driverTemp);
  Serial.print(" | PSG:");
  Serial.print(current.passengerTemp);
  Serial.print(" | AIR:");
  Serial.print(windModeToText(current.windMode));
  Serial.print(" | VOLUME:");
  Serial.print(current.volume);
  Serial.print(" | MUTE:");
  Serial.print(current.mute ? "ON" : "OFF");
  Serial.print(" | MEDIA:");
  Serial.println(current.mediaMode ? "ON" : "OFF");
  return 1;
}

uint8_t processCanReceive() {
  return mkbdCanServiceProcessReceive(state);
}

uint8_t broadcastButtonMenuEvent(uint8_t button, const SystemState& inputState) {
  return mkbdCanServiceBroadcastButtonMenu(button, inputState);
}

uint8_t broadcastEncoderSwitchEvent(uint8_t encoderEvent, const SystemState& inputState) {
  return mkbdCanServiceBroadcastEncoderSwitch(encoderEvent, inputState);
}

uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after) {
  return mkbdCanServiceBroadcastChanges(before, after);
}

void mkbdAppBegin() {
  Serial.begin(115200);
  initSystemState(state);
  mkbdHardwareBegin();
  mkbdCanServiceBegin();

  Serial.print("TWAI TX:");
  Serial.print(GDS_PIN_TWAI_TX);
  Serial.print(" RX:");
  Serial.println(GDS_PIN_TWAI_RX);

  drawCurrentScreen();
  printSystemStatus(state);
  mkbdRtosStart();
}

void mkbdAppRun() {
  mkbdRtosIdle();
}
