#include "EncoderInput.h"

static uint8_t isEncoderFallingEdge(uint8_t previousLevel, uint8_t currentLevel) {
  return previousLevel == HIGH && currentLevel == LOW;
}

void initEncoderHistory(EncoderHistory& history) {
  history.prevDriverA = HIGH;
  history.prevDriverB = HIGH;
  history.prevDriverSw = HIGH;
  history.prevPassengerA = HIGH;
  history.prevPassengerB = HIGH;
  history.prevPassengerSw = HIGH;
  history.lastEventTime = 0;
}

uint8_t detectEncoderEvent(EncoderHistory& history, const EncoderLevels& levels, unsigned long now, unsigned long debounceDelay) {
  uint8_t event = ENCODER_EVENT_NONE;

  if (now - history.lastEventTime > debounceDelay) {
    if (isEncoderFallingEdge(history.prevDriverSw, levels.driverSw)) {
      event = ENCODER_EVENT_DRIVER_SW;
    } else if (isEncoderFallingEdge(history.prevPassengerSw, levels.passengerSw)) {
      event = ENCODER_EVENT_PASSENGER_SW;
    } else if (isEncoderFallingEdge(history.prevDriverA, levels.driverA)) {
      event = (levels.driverB == HIGH) ? ENCODER_EVENT_DRIVER_CW : ENCODER_EVENT_DRIVER_CCW;
    } else if (isEncoderFallingEdge(history.prevPassengerA, levels.passengerA)) {
      event = (levels.passengerB == HIGH) ? ENCODER_EVENT_PASSENGER_CW : ENCODER_EVENT_PASSENGER_CCW;
    }

    if (event != ENCODER_EVENT_NONE) {
      history.lastEventTime = now;
    }
  }

  history.prevDriverA = levels.driverA;
  history.prevDriverB = levels.driverB;
  history.prevDriverSw = levels.driverSw;
  history.prevPassengerA = levels.passengerA;
  history.prevPassengerB = levels.passengerB;
  history.prevPassengerSw = levels.passengerSw;

  return event;
}
