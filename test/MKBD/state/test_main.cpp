#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_State();

int main(void) {
  printf("[TEST] MKBD state unit test start\n");
  ASSERT(Test_State());
  printf("[PASS] MKBD state unit test complete\n");
  return 0;
}
