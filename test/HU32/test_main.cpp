#include <stdio.h>

#include "../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../TestSupport/Test_Assert.h"

uint8_t Test_Hu32CanProtocol(uint16_t loop);
uint8_t Test_Hu32Types(uint16_t loop);

int main(void) {
  printf("[TEST] HU32 unit test start\n");
  ASSERT(Test_Hu32CanProtocol(1));
  ASSERT(Test_Hu32Types(1));
  printf("[PASS] HU32 unit test complete\n");
  return 0;
}
