#ifndef ENCODER_INPUT_H
#define ENCODER_INPUT_H

#include <Arduino.h>

enum EncoderEvent : uint8_t {
  ENCODER_EVENT_NONE = 0,
  ENCODER_EVENT_DRIVER_CW,
  ENCODER_EVENT_DRIVER_CCW,
  ENCODER_EVENT_DRIVER_SW,
  ENCODER_EVENT_PASSENGER_CW,
  ENCODER_EVENT_PASSENGER_CCW,
  ENCODER_EVENT_PASSENGER_SW
};

struct EncoderLevels {
  uint8_t driverA;
  uint8_t driverB;
  uint8_t driverSw;
  uint8_t passengerA;
  uint8_t passengerB;
  uint8_t passengerSw;
};

struct EncoderHistory {
  uint8_t prevDriverA;
  uint8_t prevDriverB;
  uint8_t prevDriverSw;
  uint8_t prevPassengerA;
  uint8_t prevPassengerB;
  uint8_t prevPassengerSw;
  unsigned long lastEventTime;
};

void initEncoderHistory(EncoderHistory& history);
uint8_t detectEncoderEvent(EncoderHistory& history, const EncoderLevels& levels, unsigned long now, unsigned long debounceDelay);

#endif
