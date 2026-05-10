#include <stdio.h>

#include "../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol(uint16_t loop);
uint8_t Test_CommandParser(uint16_t loop);
uint8_t Test_CanDriver(uint16_t loop);
uint8_t Test_CanMonitor(uint16_t loop);
uint8_t Test_WebUi(uint16_t loop);

int main(void) {
  printf("[TEST] HU unit test start\n");

  ASSERT(Test_CanProtocol(1));
  ASSERT(Test_CommandParser(1));
  ASSERT(Test_CanDriver(1));
  ASSERT(Test_CanMonitor(1));
  ASSERT(Test_WebUi(1));

  printf("[PASS] HU unit test complete\n");
  return 0;
}
