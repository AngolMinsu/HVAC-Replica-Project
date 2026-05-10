#include "../../HU/CommandParser.h"
#include "../../HU/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

static uint8_t expectCommand(const char* inputLine, uint8_t expValid, uint8_t expService, uint8_t expSignal, uint8_t expValue) {
  HuCommand actual = parseHuCommand(String(inputLine));

  ASSERT_EQUALS(0, actual.valid, expValid);
  ASSERT_EQUALS(1, actual.service, expService);
  ASSERT_EQUALS(2, actual.signal, expSignal);
  ASSERT_EQUALS(3, actual.value, expValue);
  return 1;
}

uint8_t Test_CommandParser() {
  ASSERT(expectCommand("fan 3", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_FAN_SPEED, 3));
  ASSERT(expectCommand("temp 30", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_TEMPERATURE, 30));
  ASSERT(expectCommand("volume 0", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_VOLUME, 0));
  ASSERT(expectCommand("screen info", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_SCREEN_MODE, 1));
  ASSERT(expectCommand("read map", 1, CAN_SERVICE_READ_REQUEST, CAN_SIGNAL_MAP, 0));
  ASSERT(expectCommand("fan 9", 0, 0, 0, 0));
  ASSERT(expectCommand("temp cold", 0, 0, 0, 0));
  return 1;
}
