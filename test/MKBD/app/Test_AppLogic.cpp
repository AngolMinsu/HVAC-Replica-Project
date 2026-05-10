#include "../../../MKBD/app/AppLogic.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

static uint8_t expectButton(
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
  return 1;
}

uint8_t Test_AppLogic() {
  ASSERT(expectButton(APP_BUTTON_2, SCREEN_DATC, HVAC_OFF, 0, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AC, 1, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(APP_BUTTON_3, SCREEN_DATC, HVAC_AC, 8, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AC, 0, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(APP_BUTTON_4, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MAX, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_MIN, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(APP_BUTTON_SCREEN, SCREEN_DATC, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(expectButton(APP_BUTTON_2, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0,
      1, SCREEN_INFO, HVAC_AUTO, 3, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 1, 0));

  SystemState pwmState;
  pwmState.hvacMode = HVAC_OFF;
  pwmState.fanSpeed = 5;
  ASSERT_EQUALS(9, calculateFanPwm(pwmState), 0);
  pwmState.hvacMode = HVAC_AUTO;
  pwmState.fanSpeed = GDS_FAN_SPEED_MAX;
  ASSERT_EQUALS(10, calculateFanPwm(pwmState), GDS_FAN_PWM_MAX);
  ASSERT_EQUALS(11, shouldRefreshDisplay(200, 50, 100), 1);
  ASSERT_EQUALS(12, shouldRefreshDisplay(120, 50, 100), 0);
  return 1;
}
