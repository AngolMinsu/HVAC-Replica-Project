#include <stdint.h>
#include <stdio.h>

#include "../MkbdApp.h"
#include "../GDS.h"
#include "../state/State.h"
#include "../task/MkbdTaskHooks.h"
#include <freertos/task.h>

#define TEST_ASSERT_RETURN 1
#include "TestSupport/Test_Assert.h"

int main() {
  puts("[TEST] MKBD application integration start");
  testTaskCreateCount = 0;
  mkbdAppBegin();
  ASSERT_EQUALS(0, state.screenMode, SCREEN_DATC);
  ASSERT_EQUALS(1, state.fanSpeed, GDS_FAN_SPEED_MIN);
  ASSERT_EQUALS(2, state.driverTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(3, state.passengerTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(4, testTaskCreateCount, 4);
  ASSERT_EQUALS(5, drawCurrentScreen(), SCREEN_DATC);
  ASSERT_EQUALS(6, printSystemStatus(state), 1);
  puts("[PASS] MkbdApp assembly module");
  puts("[PASS] MKBD full integrated module test");
  return 0;
}
