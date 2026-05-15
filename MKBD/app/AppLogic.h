#ifndef APP_LOGIC_H
#define APP_LOGIC_H

#include <Arduino.h>
#include "../GDS.h"
#include "../state/State.h"
#include "../encoder/EncoderInput.h"

enum AppButton : uint8_t {
  APP_BUTTON_NONE = 0,
  APP_BUTTON_FAN_UP = GDS_PIN_BTN_FAN_UP,
  APP_BUTTON_FAN_DOWN = GDS_PIN_BTN_FAN_DOWN,
  APP_BUTTON_SCREEN = GDS_PIN_BTN_SCREEN,
  APP_BUTTON_WIND_RADIO = GDS_PIN_BTN_WIND_RADIO
};

uint8_t handleButtonAction(SystemState& state, uint8_t button);
uint8_t handleEncoderAction(SystemState& state, uint8_t encoderEvent);
int calculateFanPwm(const SystemState& state);
uint8_t calculateStatusLed(const SystemState& state);
uint8_t shouldRefreshDisplay(unsigned long now, unsigned long lastDisplayTime, unsigned long interval);

#endif
