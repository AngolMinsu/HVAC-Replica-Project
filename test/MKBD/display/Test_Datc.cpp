#include "../../../MKBD/display/Datc.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Datc(uint16_t loop) {
  SystemState state;
  state.hvacMode = HVAC_OFF;
  state.fanSpeed = 0;
  state.setTemp = GDS_TEMP_MAX;
  state.windMode = WIND_MIX;

  ASSERT_EQUALS(0, datcHandleButton2(state), 1);
  ASSERT_EQUALS(1, state.hvacMode, HVAC_AC);
  ASSERT_EQUALS(2, state.fanSpeed, 1);

  ASSERT_EQUALS(3, datcHandleButton3(state), 1);
  ASSERT_EQUALS(4, state.fanSpeed, 2);

  ASSERT_EQUALS(5, datcHandleButton4(state), 1);
  ASSERT_EQUALS(6, state.setTemp, GDS_TEMP_MIN);

  ASSERT_EQUALS(7, datcHandleButton5(state), 1);
  ASSERT_EQUALS(8, state.windMode, WIND_FACE);

  state.hvacMode = HVAC_AUTO;
  state.fanSpeed = GDS_FAN_SPEED_MAX;
  ASSERT_EQUALS(9, datcHandleButton3(state), 1);
  ASSERT_EQUALS(10, state.fanSpeed, GDS_FAN_SPEED_MIN);

  state.setTemp = (GDS_TEMP_MIN + GDS_TEMP_MAX) / 2;
  ASSERT_EQUALS(11, datcHandleButton4(state), 1);
  ASSERT_EQUALS(12, state.setTemp, ((GDS_TEMP_MIN + GDS_TEMP_MAX) / 2) + 1);
  return 1;
}
