#include "../../../MKBD/app/AppLogic.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

static void initAppTestState(SystemState& state) {
  initSystemState(state);
  state.hvacMode = HVAC_AUTO;
  state.fanSpeed = 3;
  state.driverTemp = GDS_TEMP_DEFAULT;
  state.passengerTemp = GDS_TEMP_DEFAULT;
  state.windMode = WIND_FACE;
  state.volume = GDS_VOLUME_DEFAULT;
  state.mute = false;
  state.mediaReady = false;
  state.mapReady = false;
  state.homeReady = false;
  state.mediaMode = false;
  state.mediaIndex = GDS_MEDIA_INDEX_DEFAULT;
}

uint8_t Test_AppLogic(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    SystemState state;
    initAppTestState(state);

    ASSERT_EQUALS(0, handleEncoderAction(state, ENCODER_EVENT_DRIVER_CW), 1);
    ASSERT_EQUALS(1, state.driverTemp, GDS_TEMP_DEFAULT + 1);
    ASSERT_EQUALS(2, handleEncoderAction(state, ENCODER_EVENT_DRIVER_CCW), 1);
    ASSERT_EQUALS(3, state.driverTemp, GDS_TEMP_DEFAULT);
    ASSERT_EQUALS(4, handleEncoderAction(state, ENCODER_EVENT_PASSENGER_CCW), 1);
    ASSERT_EQUALS(5, state.passengerTemp, GDS_TEMP_DEFAULT - 1);
    ASSERT_EQUALS(6, handleButtonAction(state, APP_BUTTON_FAN_UP), 1);
    ASSERT_EQUALS(7, state.fanSpeed, 4);
    ASSERT_EQUALS(8, handleButtonAction(state, APP_BUTTON_FAN_DOWN), 1);
    ASSERT_EQUALS(9, state.fanSpeed, 3);
    ASSERT_EQUALS(10, handleButtonAction(state, APP_BUTTON_WIND_MEDIA), 1);
    ASSERT_EQUALS(11, state.windMode, WIND_FOOT);
    ASSERT_EQUALS(12, handleButtonAction(state, APP_BUTTON_SCREEN), 1);
    ASSERT_EQUALS(13, state.screenMode, SCREEN_INFO);

    ASSERT_EQUALS(14, handleEncoderAction(state, ENCODER_EVENT_DRIVER_CW), 1);
    ASSERT_EQUALS(15, state.volume, GDS_VOLUME_DEFAULT + 1);
    ASSERT_EQUALS(16, handleEncoderAction(state, ENCODER_EVENT_DRIVER_CCW), 1);
    ASSERT_EQUALS(17, state.volume, GDS_VOLUME_DEFAULT);
    ASSERT_EQUALS(18, handleEncoderAction(state, ENCODER_EVENT_DRIVER_SW), 1);
    ASSERT_EQUALS(19, state.mute, 1);
    ASSERT_EQUALS(20, handleEncoderAction(state, ENCODER_EVENT_PASSENGER_CW), 0);
    ASSERT_EQUALS(21, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT);
    ASSERT_EQUALS(22, handleButtonAction(state, APP_BUTTON_WIND_MEDIA), 1);
    ASSERT_EQUALS(23, state.mediaMode, 1);
    ASSERT_EQUALS(24, handleEncoderAction(state, ENCODER_EVENT_PASSENGER_CW), 1);
    ASSERT_EQUALS(25, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT + 1);
    ASSERT_EQUALS(26, handleEncoderAction(state, ENCODER_EVENT_PASSENGER_SW), 1);
    ASSERT_EQUALS(27, state.mediaReady, 1);
    ASSERT_EQUALS(28, handleButtonAction(state, APP_BUTTON_FAN_UP), 1);
    ASSERT_EQUALS(29, state.mapReady, 1);
    ASSERT_EQUALS(30, handleButtonAction(state, APP_BUTTON_FAN_DOWN), 1);
    ASSERT_EQUALS(31, state.homeReady, 1);
    ASSERT_EQUALS(32, handleButtonAction(state, APP_BUTTON_NONE), 0);
    ASSERT_EQUALS(33, handleEncoderAction(state, ENCODER_EVENT_NONE), 0);
  }

  SystemState pwmState;
  initAppTestState(pwmState);
  pwmState.fanSpeed = 5;
  pwmState.windMode = WIND_OFF;
  ASSERT_EQUALS(34, calculateFanPwm(pwmState), 0);
  pwmState.windMode = WIND_FACE;
  pwmState.fanSpeed = GDS_FAN_SPEED_MAX;
  ASSERT_EQUALS(35, calculateFanPwm(pwmState), GDS_FAN_PWM_MAX);
  pwmState.fanSpeed = (GDS_FAN_SPEED_MIN + GDS_FAN_SPEED_MAX) / 2;
  ASSERT_EQUALS(36, calculateFanPwm(pwmState) > GDS_FAN_PWM_MIN, 1);
  ASSERT_EQUALS(37, calculateStatusLed(pwmState), HIGH);
  pwmState.fanSpeed = 0;
  ASSERT_EQUALS(38, calculateStatusLed(pwmState), LOW);
  ASSERT_EQUALS(39, shouldRefreshDisplay(150, 50, 100), 0);
  ASSERT_EQUALS(40, shouldRefreshDisplay(151, 50, 100), 1);
  ASSERT_EQUALS(41, shouldRefreshDisplay(200, 50, 100), 1);
  ASSERT_EQUALS(42, shouldRefreshDisplay(120, 50, 100), 0);

  return 1;
}
