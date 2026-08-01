#include "MkbdCanService.h"

#include "../GDS.h"
#include "../app/AppLogic.h"
#include "../encoder/EncoderInput.h"
#include "CanDriver.h"
#include "CanHandler.h"
#include "CanMonitor.h"
#include "CanProtocol.h"

static uint8_t canTxCounter = 0;

static uint8_t sendPayload(uint16_t id, const CanPayload& payload) {
  CanFrame frame;
  frame.id = id;
  frame.dlc = CAN_DLC;
  canPayloadToBytes(payload, frame.data);
  canMonitorPrintFrame("TX", frame);

  uint8_t sent = canDriverSend(frame);
  if (!sent) {
    Serial.println("CAN ERROR: SEND FAIL");
    canDriverPrintStatus("SEND FAIL");
  }
  return sent;
}

static uint8_t broadcastSignal(const SystemState& state, uint8_t signal) {
  CanPayload payload = canMakeStatusPayload(state, signal, canTxCounter++);
  return sendPayload(CAN_ID_HVAC_STATUS, payload);
}

static uint8_t broadcastIfChanged(const SystemState& before, const SystemState& after, uint8_t signal) {
  uint8_t beforeValue = 0;
  uint8_t afterValue = 0;
  if (!canSignalValueFromState(before, signal, beforeValue) ||
      !canSignalValueFromState(after, signal, afterValue) ||
      beforeValue == afterValue) {
    return 0;
  }
  return broadcastSignal(after, signal);
}

uint8_t mkbdCanServiceBegin() {
  uint8_t ready = canDriverBegin();
  Serial.print("CAN:");
  Serial.println(ready ? "READY" : "FAIL");
  canDriverPrintStatus("BEGIN");
  return ready;
}

uint8_t mkbdCanServiceProcessReceive(SystemState& state) {
  CanFrame rxFrame;
  if (!canDriverReceive(rxFrame)) return 0;

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
  sendPayload(CAN_ID_CONTROL_RESPONSE, response);
  if (changed) mkbdCanServiceBroadcastChanges(before, state);
  return changed;
}

uint8_t mkbdCanServiceBroadcastButtonMenu(uint8_t button, const SystemState& state) {
  if (state.screenMode != SCREEN_INFO) return 0;
  if (button == APP_BUTTON_FAN_UP) return broadcastSignal(state, CAN_SIGNAL_HU_OPEN_MAP);
  if (button == APP_BUTTON_FAN_DOWN) return broadcastSignal(state, CAN_SIGNAL_HU_OPEN_HOME);
  if (button == APP_BUTTON_WIND_MEDIA) return broadcastSignal(state, CAN_SIGNAL_HU_OPEN_MEDIA);
  return 0;
}

uint8_t mkbdCanServiceBroadcastEncoderSwitch(uint8_t event, const SystemState& state) {
  if (state.screenMode == SCREEN_INFO) {
    if (event == ENCODER_EVENT_PASSENGER_CW) return broadcastSignal(state, CAN_SIGNAL_HU_FOCUS_NEXT);
    if (event == ENCODER_EVENT_PASSENGER_CCW) return broadcastSignal(state, CAN_SIGNAL_HU_FOCUS_PREV);
    if (event == ENCODER_EVENT_PASSENGER_SW) return broadcastSignal(state, CAN_SIGNAL_PASSENGER_ENCODER_SW);
    return 0;
  }
  if (event == ENCODER_EVENT_DRIVER_SW) return broadcastSignal(state, CAN_SIGNAL_DRIVER_ENCODER_SW);
  if (event == ENCODER_EVENT_PASSENGER_SW) return broadcastSignal(state, CAN_SIGNAL_PASSENGER_ENCODER_SW);
  return 0;
}

uint8_t mkbdCanServiceBroadcastChanges(const SystemState& before, const SystemState& after) {
  static const uint8_t signals[] = {
      CAN_SIGNAL_POWER, CAN_SIGNAL_FAN_SPEED, CAN_SIGNAL_TEMPERATURE,
      CAN_SIGNAL_PASSENGER_TEMPERATURE, CAN_SIGNAL_MODE, CAN_SIGNAL_AC,
      CAN_SIGNAL_AUTO, CAN_SIGNAL_SCREEN_MODE, CAN_SIGNAL_MEDIA,
      CAN_SIGNAL_VOLUME, CAN_SIGNAL_MAP, CAN_SIGNAL_MUTE, CAN_SIGNAL_HOME,
      CAN_SIGNAL_MEDIA_MODE, CAN_SIGNAL_MEDIA_INDEX};

  uint8_t sentCount = 0;
  for (uint8_t signal : signals) {
    sentCount += broadcastIfChanged(before, after, signal);
  }
  return sentCount;
}
