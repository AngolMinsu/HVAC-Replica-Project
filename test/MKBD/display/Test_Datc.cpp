#include "../../../MKBD/display/Datc.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Datc(uint16_t loop) {
  SystemState state;
  state.hvacMode = HVAC_OFF;
  state.fanSpeed = 0;
  state.driverTemp = GDS_TEMP_MAX;
  state.passengerTemp = GDS_TEMP_MIN;
  state.windMode = WIND_MIX;

  ASSERT_EQUALS(0, datcIncreaseDriverTemp(state), 0);
  ASSERT_EQUALS(1, state.driverTemp, GDS_TEMP_MAX);
  ASSERT_EQUALS(2, datcDecreasePassengerTemp(state), 0);
  ASSERT_EQUALS(3, state.passengerTemp, GDS_TEMP_MIN);

  ASSERT_EQUALS(4, datcIncreaseFanSpeed(state), 1);
  ASSERT_EQUALS(5, state.fanSpeed, 1);
  ASSERT_EQUALS(6, datcDecreaseFanSpeed(state), 1);
  ASSERT_EQUALS(7, state.fanSpeed, 0);
  ASSERT_EQUALS(8, datcDecreaseFanSpeed(state), 0);
  ASSERT_EQUALS(9, state.fanSpeed, GDS_FAN_SPEED_MIN);

  ASSERT_EQUALS(10, datcCycleWindMode(state), 1);
  ASSERT_EQUALS(11, state.windMode, WIND_OFF);
  ASSERT_EQUALS(12, datcCycleWindMode(state), 1);
  ASSERT_EQUALS(13, state.windMode, WIND_FACE);

  state.driverTemp = (GDS_TEMP_MIN + GDS_TEMP_MAX) / 2;
  ASSERT_EQUALS(14, datcIncreaseDriverTemp(state), 1);
  ASSERT_EQUALS(15, state.driverTemp, ((GDS_TEMP_MIN + GDS_TEMP_MAX) / 2) + 1);
  ASSERT_EQUALS(16, datcDecreaseDriverTemp(state), 1);
  ASSERT_EQUALS(17, state.driverTemp, (GDS_TEMP_MIN + GDS_TEMP_MAX) / 2);
  state.passengerTemp = (GDS_TEMP_MIN + GDS_TEMP_MAX) / 2;
  ASSERT_EQUALS(18, datcIncreasePassengerTemp(state), 1);
  ASSERT_EQUALS(19, state.passengerTemp, ((GDS_TEMP_MIN + GDS_TEMP_MAX) / 2) + 1);
  return 1;
}
