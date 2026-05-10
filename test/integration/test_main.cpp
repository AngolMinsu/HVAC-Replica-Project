#include <stdio.h>

#include "../TestSupport/Arduino.h"
#define TEST_ASSERT_RETURN 1
#include "../TestSupport/Test_Assert.h"

uint8_t Test_EndToEndControlInput(uint16_t loop);

int main(void) {
  printf("[TEST] Integration module test start\n");

  ASSERT(Test_EndToEndControlInput(1));

  printf("[PASS] Integration module test complete\n");
  return 0;
}
