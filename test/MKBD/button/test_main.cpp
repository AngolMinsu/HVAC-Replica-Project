#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_ButtonInput();

int main(void) {
  printf("[TEST] MKBD button unit test start\n");
  ASSERT(Test_ButtonInput());
  printf("[PASS] MKBD button unit test complete\n");
  return 0;
}
