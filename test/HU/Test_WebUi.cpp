#include "../../HU/WebUi.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_WebUi() {
  ASSERT_EQUALS(0, sizeof(HuRequestSender) > 0, 1);
  ASSERT_EQUALS(1, sizeof(CanPayload), 8);
  return 1;
}
