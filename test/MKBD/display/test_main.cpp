#include <stdio.h>

#include "../../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Datc(uint16_t loop);
uint8_t Test_Info(uint16_t loop);

int main(void) {
  printf("[TEST] MKBD display unit test start\n");
  ASSERT(Test_Datc(1));
  ASSERT(Test_Info(1));
  printf("[PASS] MKBD display unit test complete\n");
  return 0;
}
