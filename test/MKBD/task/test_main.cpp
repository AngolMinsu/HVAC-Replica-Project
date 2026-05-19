#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_MkbdScheduler(uint16_t loop);

int main(void) {
  printf("[TEST] MKBD task unit test start\n");
  ASSERT(Test_MkbdScheduler(1));
  printf("[PASS] MKBD task unit test complete\n");
  return 0;
}
