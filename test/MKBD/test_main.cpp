#include <stdio.h>

#define TEST_ASSERT_RETURN 1
#include "../TestSupport/Test_Assert.h"
#include "../../MKBD/GDS.h"

int main(void) {
  printf("[TEST] MKBD root unit test start\n");

  ASSERT_EQUALS(0, GDS_CAN_DLC, 8);
  ASSERT_EQUALS(1, GDS_CAN_ID_CONTROL_REQUEST, 0x100);
  ASSERT_EQUALS(2, GDS_CAN_ID_CONTROL_RESPONSE, 0x101);
  ASSERT_EQUALS(3, GDS_CAN_ID_HVAC_STATUS, 0x300);
  ASSERT_EQUALS(4, GDS_TEMP_MIN <= GDS_TEMP_DEFAULT, 1);
  ASSERT_EQUALS(5, GDS_TEMP_DEFAULT <= GDS_TEMP_MAX, 1);

  printf("[PASS] MKBD root unit test complete\n");
  return 0;
}
