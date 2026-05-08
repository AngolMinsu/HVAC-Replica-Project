#include "AppLogic.h"
#include "../GDS.h"
#include "../display/Datc.h"
#include "../display/Info.h"

uint8_t handleButtonAction(SystemState& state, uint8_t button) {
  if (button == APP_BUTTON_SCREEN) {
    toggleScreenMode(state);
    return 1;
  }

  if (state.screenMode == SCREEN_DATC) {
    switch (button) {
      case APP_BUTTON_2: return datcHandleButton2(state);
      case APP_BUTTON_3: return datcHandleButton3(state);
      case APP_BUTTON_4: return datcHandleButton4(state);
      case APP_BUTTON_5: return datcHandleButton5(state);
      default: return 0;
    }
  }

  switch (button) {
    case APP_BUTTON_2: return infoHandleButton2(state);
    case APP_BUTTON_3: return infoHandleButton3(state);
    case APP_BUTTON_4: return infoHandleButton4(state);
    case APP_BUTTON_5: return infoHandleButton5(state);
    default: return 0;
  }
}

int calculateFanPwm(const SystemState& state) {
  if (state.hvacMode == HVAC_OFF || state.fanSpeed <= 0) {
    return 0;
  }

  if (state.fanSpeed >= GDS_FAN_SPEED_MAX) {
    return GDS_FAN_PWM_MAX;
  }

  return GDS_FAN_PWM_MIN +
         ((state.fanSpeed - GDS_FAN_SPEED_MIN - 1) * (GDS_FAN_PWM_MAX - GDS_FAN_PWM_MIN)) /
         (GDS_FAN_SPEED_MAX - GDS_FAN_SPEED_MIN - 1);
}

uint8_t calculateStatusLed(const SystemState& state) {
  return state.hvacMode == HVAC_OFF ? LOW : HIGH;
}

uint8_t shouldRefreshDisplay(unsigned long now, unsigned long lastDisplayTime, unsigned long interval) {
  return now - lastDisplayTime > interval;
}
