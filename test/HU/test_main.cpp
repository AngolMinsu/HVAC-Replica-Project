#include <stdio.h>

#include "../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol();
uint8_t Test_CommandParser();
uint8_t Test_CanDriver();
uint8_t Test_CanMonitor();
uint8_t Test_WebUi();

int main(void) {
  printf("[TEST] HU unit test start\n");

  ASSERT(Test_CanProtocol());
  ASSERT(Test_CommandParser());
  ASSERT(Test_CanDriver());
  ASSERT(Test_CanMonitor());
  ASSERT(Test_WebUi());

  printf("[PASS] HU unit test complete\n");
  return 0;
}
