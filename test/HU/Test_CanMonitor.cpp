#include "../../HU/CanMonitor.h"
#include "../../HU/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_CanMonitor(uint16_t loop) {
  CanFrame frame;
  frame.id = CAN_ID_CONTROL_REQUEST;
  frame.dlc = 2;
  frame.data[0] = 0x20;
  frame.data[1] = 0x00;
  canMonitorPrintFrame("TX", frame);

  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 3, 0, 1);
  canMonitorPrintPayloadSummary(payload);
  ASSERT_EQUALS(0, canValidateChecksum(payload), 1);
  return 1;
}
