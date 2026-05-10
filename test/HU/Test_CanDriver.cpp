#include "../../HU/CanDriver.h"
#include "../../HU/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_CanDriver(uint16_t loop) {
  CanFrame frame;
  frame.id = CAN_ID_CONTROL_REQUEST;
  frame.dlc = CAN_DLC + 1;

  ASSERT_EQUALS(0, canDriverIsReady(), 0);
  ASSERT_EQUALS(1, canDriverSend(frame), 0);
  ASSERT_EQUALS(2, canDriverBegin(0), 1);
  ASSERT_EQUALS(3, canDriverIsReady(), 1);
  ASSERT_EQUALS(4, canDriverSend(frame), 0);

  frame.dlc = CAN_DLC;
  for (uint8_t i = 0; i < CAN_DLC; i++) {
    frame.data[i] = i;
  }
  ASSERT_EQUALS(5, canDriverSend(frame), 1);

  CanFrame rx;
  ASSERT_EQUALS(6, canDriverReceive(rx), 0);
  return 1;
}
