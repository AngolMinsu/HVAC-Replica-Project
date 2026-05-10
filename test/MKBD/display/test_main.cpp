#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Datc();
uint8_t Test_Info();

int main(void) {
  printf("[TEST] MKBD display unit test start\n");
  ASSERT(Test_Datc());
  ASSERT(Test_Info());
  printf("[PASS] MKBD display unit test complete\n");
  return 0;
}
