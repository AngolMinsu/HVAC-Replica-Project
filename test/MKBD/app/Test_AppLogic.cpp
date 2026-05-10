#include "../../../MKBD/app/AppLogic.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

static uint8_t expectButton(
    uint16_t loop,
    uint8_t button,
    ScreenMode inputScreenMode,
    HvacMode inputHvacMode,
    int inputFanSpeed,
    int inputSetTemp,
    WindMode inputWindMode,
    int inputVolume,
    uint8_t inputMediaReady,
    uint8_t inputMapReady,
    uint8_t expChanged,
    ScreenMode expScreenMode,
    HvacMode expHvacMode,
    int expFanSpeed,
    int expSetTemp,
    WindMode expWindMode,
    int expVolume,
    uint8_t expMediaReady,
    uint8_t expMapReady) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    SystemState actual;
    actual.screenMode = inputScreenMode;
    actual.hvacMode = inputHvacMode;
    actual.fanSpeed = inputFanSpeed;
    actual.setTemp = inputSetTemp;
    actual.windMode = inputWindMode;
    actual.volume = inputVolume;
    actual.mediaReady = inputMediaReady != 0;
    actual.mapReady = inputMapReady != 0;

    uint8_t changed = handleButtonAction(actual, button);

    ASSERT_EQUALS(0, changed, expChanged);
    ASSERT_EQUALS(1, actual.screenMode, expScreenMode);
    ASSERT_EQUALS(2, actual.hvacMode, expHvacMode);
    ASSERT_EQUALS(3, actual.fanSpeed, expFanSpeed);
    ASSERT_EQUALS(4, actual.setTemp, expSetTemp);
    ASSERT_EQUALS(5, actual.windMode, expWindMode);
    ASSERT_EQUALS(6, actual.volume, expVolume);
    ASSERT_EQUALS(7, actual.mediaReady, expMediaReady);
    ASSERT_EQUALS(8, actual.mapReady, expMapReady);
  }

  return 1;
}

uint8_t Test_AppLogic(uint16_t loop) {
  if (loop == 0) loop = 1;

  ASSERT(expectButton(loop, APP_BUTTON_2, SCREEN_DATC, HVAC_OFF, 0, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AC, 1, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_2, SCREEN_DATC, HVAC_AUTO, 1, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_OFF, 0, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_3, SCREEN_DATC, HVAC_AC, 8, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AC, 0, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_3, SCREEN_DATC, HVAC_AC, GDS_FAN_SPEED_MIN, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AC, GDS_FAN_SPEED_MIN + 1, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_4, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MAX, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MIN, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_4, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MIN, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MIN + 1, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_5, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_MIX, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_SCREEN, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_2, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 1, 0));
  ASSERT(expectButton(1, APP_BUTTON_3, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MAX - 1, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MAX, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_3, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MAX, 0, 0,
      0, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MAX, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_4, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MIN + 1, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MIN, 0, 0));
  ASSERT(expectButton(1, APP_BUTTON_4, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MIN, 0, 0,
      0, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_MIN, 0, 0));

  SystemState pwmState;
  pwmState.hvacMode = HVAC_OFF;
  pwmState.fanSpeed = 5;
  ASSERT_EQUALS(9, calculateFanPwm(pwmState), 0);
  pwmState.hvacMode = HVAC_AUTO;
  pwmState.fanSpeed = GDS_FAN_SPEED_MAX;
  ASSERT_EQUALS(10, calculateFanPwm(pwmState), GDS_FAN_PWM_MAX);
  pwmState.fanSpeed = (GDS_FAN_SPEED_MIN + GDS_FAN_SPEED_MAX) / 2;
  ASSERT_EQUALS(11, calculateFanPwm(pwmState) > GDS_FAN_PWM_MIN, 1);
  ASSERT_EQUALS(12, shouldRefreshDisplay(150, 50, 100), 0);
  ASSERT_EQUALS(13, shouldRefreshDisplay(151, 50, 100), 1);
  ASSERT_EQUALS(14, shouldRefreshDisplay(200, 50, 100), 1);
  ASSERT_EQUALS(15, shouldRefreshDisplay(120, 50, 100), 0);
  return 1;
}
