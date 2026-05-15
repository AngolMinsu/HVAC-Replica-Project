#include "ButtonInput.h"

static uint8_t isFallingEdge(uint8_t previousLevel, uint8_t currentLevel) {
  return previousLevel == HIGH && currentLevel == LOW;
}

void initButtonHistory(ButtonHistory& history) {
  history.prevFanUp = HIGH;
  history.prevFanDown = HIGH;
  history.prevScreen = HIGH;
  history.prevWindMedia = HIGH;
  history.lastEventTime = 0;
}

uint8_t detectButtonEvent(ButtonHistory& history, const ButtonLevels& levels, unsigned long now, unsigned long debounceDelay) {
  uint8_t button = APP_BUTTON_NONE;

  if (now - history.lastEventTime > debounceDelay) {
    if (isFallingEdge(history.prevScreen, levels.screen)) {
      button = APP_BUTTON_SCREEN;
    } else if (isFallingEdge(history.prevFanUp, levels.fanUp)) {
      button = APP_BUTTON_FAN_UP;
    } else if (isFallingEdge(history.prevFanDown, levels.fanDown)) {
      button = APP_BUTTON_FAN_DOWN;
    } else if (isFallingEdge(history.prevWindMedia, levels.windMedia)) {
      button = APP_BUTTON_WIND_MEDIA;
    }

    if (button != APP_BUTTON_NONE) {
      history.lastEventTime = now;
    }
  }

  history.prevFanUp = levels.fanUp;
  history.prevFanDown = levels.fanDown;
  history.prevScreen = levels.screen;
  history.prevWindMedia = levels.windMedia;

  return button;
}
