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
  ASSERT_EQUALS(3, state.setTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(4, state.windMode, WIND_FACE);
  ASSERT_EQUALS(5, state.volume, GDS_VOLUME_DEFAULT);

  toggleScreenMode(state);
  ASSERT_EQUALS(6, state.screenMode, SCREEN_INFO);
  toggleScreenMode(state);
  ASSERT_EQUALS(7, state.screenMode, SCREEN_DATC);

  ASSERT_EQUALS(8, screenModeToText(SCREEN_INFO)[0], 'I');
  ASSERT_EQUALS(9, hvacModeToText(HVAC_AUTO)[0], 'A');
  ASSERT_EQUALS(10, windModeToText(WIND_DEF)[0], 'D');
  return 1;
}
