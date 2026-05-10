#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol();
uint8_t Test_CanHandler();
uint8_t Test_CanDriver();
uint8_t Test_CanMonitor();

int main(void) {
  printf("[TEST] MKBD can unit test start\n");
  ASSERT(Test_CanProtocol());
  ASSERT(Test_CanHandler());
  ASSERT(Test_CanDriver());
  ASSERT(Test_CanMonitor());
  printf("[PASS] MKBD can unit test complete\n");
  return 0;
}
