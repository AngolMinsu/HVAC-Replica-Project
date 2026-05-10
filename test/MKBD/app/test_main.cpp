#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_AppLogic(uint16_t loop);

int main(void) {
  printf("[TEST] MKBD app unit test start\n");
  ASSERT(Test_AppLogic(1));
  printf("[PASS] MKBD app unit test complete\n");
  return 0;
}
