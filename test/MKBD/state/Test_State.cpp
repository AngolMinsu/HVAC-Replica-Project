#include "../../../MKBD/state/State.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_State(uint16_t loop) {
  SystemState state;
  initSystemState(state);

  ASSERT_EQUALS(0, state.screenMode, SCREEN_DATC);
  ASSERT_EQUALS(1, state.hvacMode, HVAC_OFF);
  ASSERT_EQUALS(2, state.fanSpeed, GDS_FAN_SPEED_MIN);
  ASSERT_EQUALS(3, state.driverTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(4, state.passengerTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(5, state.windMode, WIND_FACE);
  ASSERT_EQUALS(6, state.volume, GDS_VOLUME_DEFAULT);
  ASSERT_EQUALS(7, state.mute, 0);
  ASSERT_EQUALS(8, state.homeReady, 0);
  ASSERT_EQUALS(9, state.mediaMode, 0);
  ASSERT_EQUALS(10, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT);

  toggleScreenMode(state);
  ASSERT_EQUALS(11, state.screenMode, SCREEN_INFO);
  toggleScreenMode(state);
  ASSERT_EQUALS(12, state.screenMode, SCREEN_DATC);

  ASSERT_EQUALS(13, screenModeToText(SCREEN_INFO)[0], 'I');
  ASSERT_EQUALS(14, hvacModeToText(HVAC_AUTO)[0], 'A');
  ASSERT_EQUALS(15, windModeToText(WIND_DEF)[0], 'D');
  ASSERT_EQUALS(16, windModeToText(WIND_OFF)[0], 'O');
  ASSERT_EQUALS(17, screenModeToText((ScreenMode)0xFF)[0], 'U');
  ASSERT_EQUALS(18, hvacModeToText((HvacMode)0xFF)[0], 'U');
  ASSERT_EQUALS(19, windModeToText((WindMode)0xFF)[0], 'U');
  return 1;
}
