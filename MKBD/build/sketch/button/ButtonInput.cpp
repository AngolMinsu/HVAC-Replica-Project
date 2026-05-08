#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\button\\ButtonInput.cpp"
#include "ButtonInput.h"

static uint8_t isFallingEdge(uint8_t previousLevel, uint8_t currentLevel) {
  return previousLevel == HIGH && currentLevel == LOW;
}

void initButtonHistory(ButtonHistory& history) {
  history.prevBtn2 = HIGH;
  history.prevBtn3 = HIGH;
  history.prevBtn4 = HIGH;
  history.prevBtn5 = HIGH;
  history.prevScreen = HIGH;
  history.lastEventTime = 0;
}

uint8_t detectButtonEvent(ButtonHistory& history, const ButtonLevels& levels, unsigned long now, unsigned long debounceDelay) {
  uint8_t button = APP_BUTTON_NONE;

  if (now - history.lastEventTime > debounceDelay) {
    if (isFallingEdge(history.prevScreen, levels.screen)) {
      button = APP_BUTTON_SCREEN;
    } else if (isFallingEdge(history.prevBtn2, levels.btn2)) {
      button = APP_BUTTON_2;
    } else if (isFallingEdge(history.prevBtn3, levels.btn3)) {
      button = APP_BUTTON_3;
    } else if (isFallingEdge(history.prevBtn4, levels.btn4)) {
      button = APP_BUTTON_4;
    } else if (isFallingEdge(history.prevBtn5, levels.btn5)) {
      button = APP_BUTTON_5;
    }

    if (button != APP_BUTTON_NONE) {
      history.lastEventTime = now;
    }
  }

  history.prevBtn2 = levels.btn2;
  history.prevBtn3 = levels.btn3;
  history.prevBtn4 = levels.btn4;
  history.prevBtn5 = levels.btn5;
  history.prevScreen = levels.screen;

  return button;
}
