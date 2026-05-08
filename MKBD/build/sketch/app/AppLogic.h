#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\app\\AppLogic.h"
#ifndef APP_LOGIC_H
#define APP_LOGIC_H

#include <Arduino.h>
#include "../GDS.h"
#include "../state/State.h"

enum AppButton : uint8_t {
  APP_BUTTON_NONE = 0,
  APP_BUTTON_2 = GDS_PIN_BTN_2,
  APP_BUTTON_3 = GDS_PIN_BTN_3,
  APP_BUTTON_4 = GDS_PIN_BTN_4,
  APP_BUTTON_5 = GDS_PIN_BTN_5,
  APP_BUTTON_SCREEN = GDS_PIN_BTN_SCREEN
};

uint8_t handleButtonAction(SystemState& state, uint8_t button);
int calculateFanPwm(const SystemState& state);
uint8_t calculateStatusLed(const SystemState& state);
uint8_t shouldRefreshDisplay(unsigned long now, unsigned long lastDisplayTime, unsigned long interval);

#endif
