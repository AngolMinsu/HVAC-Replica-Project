#include "EncoderInput.h"

static uint8_t isEncoderFallingEdge(uint8_t previousLevel, uint8_t currentLevel) {
  return previousLevel == HIGH && currentLevel == LOW;
}

static uint8_t makeEncoderState(uint8_t a, uint8_t b) {
  return ((a == LOW) ? 0x02 : 0x00) | ((b == LOW) ? 0x01 : 0x00);
}

static int8_t decodeEncoderTransition(uint8_t previousState, uint8_t currentState) {
  static const int8_t table[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
  };

  return table[(previousState << 2) | currentState];
}

void initEncoderHistory(EncoderHistory& history) {
  history.prevDriverA = HIGH;
  history.prevDriverB = HIGH;
  history.prevDriverSw = HIGH;
  history.prevPassengerA = HIGH;
  history.prevPassengerB = HIGH;
  history.prevPassengerSw = HIGH;
  history.driverStep = 0;
  history.passengerStep = 0;
  history.lastEventTime = 0;
}

uint8_t detectEncoderEvent(EncoderHistory& history, const EncoderLevels& levels, unsigned long now, unsigned long debounceDelay) {
  uint8_t event = ENCODER_EVENT_NONE;

  if (now - history.lastEventTime > debounceDelay) {
    if (isEncoderFallingEdge(history.prevDriverSw, levels.driverSw)) {
      event = ENCODER_EVENT_DRIVER_SW;
    } else if (isEncoderFallingEdge(history.prevPassengerSw, levels.passengerSw)) {
      event = ENCODER_EVENT_PASSENGER_SW;
    } else {
      uint8_t prevDriverState = makeEncoderState(history.prevDriverA, history.prevDriverB);
      uint8_t currDriverState = makeEncoderState(levels.driverA, levels.driverB);
      uint8_t prevPassengerState = makeEncoderState(history.prevPassengerA, history.prevPassengerB);
      uint8_t currPassengerState = makeEncoderState(levels.passengerA, levels.passengerB);

      history.driverStep += decodeEncoderTransition(prevDriverState, currDriverState);
      history.passengerStep += decodeEncoderTransition(prevPassengerState, currPassengerState);

      if (history.driverStep >= 4) {
        history.driverStep = 0;
        event = ENCODER_EVENT_DRIVER_CW;
      } else if (history.driverStep <= -4) {
        history.driverStep = 0;
        event = ENCODER_EVENT_DRIVER_CCW;
      } else if (history.passengerStep >= 4) {
        history.passengerStep = 0;
        event = ENCODER_EVENT_PASSENGER_CW;
      } else if (history.passengerStep <= -4) {
        history.passengerStep = 0;
        event = ENCODER_EVENT_PASSENGER_CCW;
      }
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
