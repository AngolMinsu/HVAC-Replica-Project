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
      case APP_BUTTON_FAN_UP: return datcIncreaseFanSpeed(state);
      case APP_BUTTON_FAN_DOWN: return datcDecreaseFanSpeed(state);
      case APP_BUTTON_WIND_MEDIA: return datcCycleWindMode(state);
      default: return 0;
    }
  }

  switch (button) {
    case APP_BUTTON_FAN_UP: return infoHandleMap(state);
    case APP_BUTTON_FAN_DOWN: return infoHandleHome(state);
    case APP_BUTTON_WIND_MEDIA: return infoHandleMedia(state);
    default: return 0;
  }
}

uint8_t handleEncoderAction(SystemState& state, uint8_t encoderEvent) {
  if (encoderEvent == ENCODER_EVENT_NONE) {
    return 0;
  }

  if (state.screenMode == SCREEN_DATC) {
    switch (encoderEvent) {
      case ENCODER_EVENT_DRIVER_CW: return datcIncreaseDriverTemp(state);
      case ENCODER_EVENT_DRIVER_CCW: return datcDecreaseDriverTemp(state);
      case ENCODER_EVENT_DRIVER_SW: return 0;
      case ENCODER_EVENT_PASSENGER_CW: return datcIncreasePassengerTemp(state);
      case ENCODER_EVENT_PASSENGER_CCW: return datcDecreasePassengerTemp(state);
      case ENCODER_EVENT_PASSENGER_SW: return 0;
      default: return 0;
    }
  }

  switch (encoderEvent) {
    case ENCODER_EVENT_DRIVER_CW: return infoIncreaseVolume(state);
    case ENCODER_EVENT_DRIVER_CCW: return infoDecreaseVolume(state);
    case ENCODER_EVENT_DRIVER_SW: return infoToggleMute(state);
    case ENCODER_EVENT_PASSENGER_CW: return infoMediaIndexUp(state);
    case ENCODER_EVENT_PASSENGER_CCW: return infoMediaIndexDown(state);
    case ENCODER_EVENT_PASSENGER_SW: return infoSelect(state);
    default: return 0;
  }
}

int calculateFanPwm(const SystemState& state) {
  if (state.windMode == WIND_OFF || state.fanSpeed <= 0) {
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
  return calculateFanPwm(state) == 0 ? LOW : HIGH;
}

uint8_t shouldRefreshDisplay(unsigned long now, unsigned long lastDisplayTime, unsigned long interval) {
  return now - lastDisplayTime > interval;
}
