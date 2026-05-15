#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>
#include "../app/AppLogic.h"

struct ButtonLevels {
  uint8_t fanUp;
  uint8_t fanDown;
  uint8_t screen;
  uint8_t windRadio;
};

struct ButtonHistory {
  uint8_t prevFanUp;
  uint8_t prevFanDown;
  uint8_t prevScreen;
  uint8_t prevWindRadio;
  unsigned long lastEventTime;
};

void initButtonHistory(ButtonHistory& history);
uint8_t detectButtonEvent(ButtonHistory& history, const ButtonLevels& levels, unsigned long now, unsigned long debounceDelay);

#endif
