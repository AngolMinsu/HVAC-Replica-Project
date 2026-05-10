#include "../../../MKBD/can/CanMonitor.h"
#include "../../../MKBD/can/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_CanMonitor() {
  CanFrame frame;
  frame.id = CAN_ID_CONTROL_REQUEST;
  frame.dlc = 2;
  frame.data[0] = 0x20;
  frame.data[1] = 0x00;
  canMonitorPrintFrame("RX", frame);
  ASSERT_EQUALS(0, frame.dlc, 2);
  return 1;
}
