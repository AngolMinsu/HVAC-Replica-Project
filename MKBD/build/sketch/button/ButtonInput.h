#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\button\\ButtonInput.h"
#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <Arduino.h>
#include "../app/AppLogic.h"

struct ButtonLevels {
  uint8_t btn2;
  uint8_t btn3;
  uint8_t btn4;
  uint8_t btn5;
  uint8_t screen;
};

struct ButtonHistory {
  uint8_t prevBtn2;
  uint8_t prevBtn3;
  uint8_t prevBtn4;
  uint8_t prevBtn5;
  uint8_t prevScreen;
  unsigned long lastEventTime;
};

void initButtonHistory(ButtonHistory& history);
uint8_t detectButtonEvent(ButtonHistory& history, const ButtonLevels& levels, unsigned long now, unsigned long debounceDelay);

#endif
