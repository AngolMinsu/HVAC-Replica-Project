#include "Test_Assert.h"

#include <stdint.h>
#include <stdlib.h>

enum TestResult {
  TEST_RESULT_OK = 0,
  TEST_RESULT_NOK = 1
};

static uint8_t Test_AllUnitTests() {
  fflush(stdout);
  int result = system("powershell -ExecutionPolicy Bypass -File .\\test\\run_tests.ps1");
  if (result != 0) {
    printf("[NOK] HVAC Replica all unit test failed\n");
    return 0;
  }

  return 1;
}

int main(void) {
  printf("[TEST] HVAC Replica all unit test start\n");

  if (!Test_AllUnitTests()) {
    /*
      ============================================================
       NOK : return 1
      ============================================================
    */
    return TEST_RESULT_NOK;
  }

  printf("[ OK ] HVAC Replica all unit test complete\n");

  /*
    ============================================================
     OK : return 0
    ============================================================
  */
  return TEST_RESULT_OK;
}
