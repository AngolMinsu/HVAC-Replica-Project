#include "../../HU/CommandParser.h"
#include "../../HU/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

static uint8_t expectCommand(uint16_t loop, const char* inputLine, uint8_t expValid, uint8_t expService, uint8_t expSignal, uint8_t expValue) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    HuCommand actual = parseHuCommand(String(inputLine));

    ASSERT_EQUALS(0, actual.valid, expValid);
    ASSERT_EQUALS(1, actual.service, expService);
    ASSERT_EQUALS(2, actual.signal, expSignal);
    ASSERT_EQUALS(3, actual.value, expValue);
  }

  return 1;
}

uint8_t Test_CommandParser(uint16_t loop) {
  if (loop == 0) loop = 1;

  ASSERT(expectCommand(loop, "fan 0", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_FAN_SPEED, 0));
  ASSERT(expectCommand(1, "fan 4", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_FAN_SPEED, 4));
  ASSERT(expectCommand(1, "fan 8", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_FAN_SPEED, 8));
  ASSERT(expectCommand(1, "temp 18", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_TEMPERATURE, 18));
  ASSERT(expectCommand(1, "temp 24", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_TEMPERATURE, 24));
  ASSERT(expectCommand(1, "temp 30", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_TEMPERATURE, 30));
  ASSERT(expectCommand(1, "volume 0", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_VOLUME, 0));
  ASSERT(expectCommand(1, "volume 15", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_VOLUME, 15));
  ASSERT(expectCommand(1, "volume 30", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_VOLUME, 30));
  ASSERT(expectCommand(1, "power on", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_POWER, 1));
  ASSERT(expectCommand(1, "power off", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_POWER, 0));
  ASSERT(expectCommand(1, "wind mix", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_MODE, 3));
  ASSERT(expectCommand(1, "screen info", 1, CAN_SERVICE_WRITE_REQUEST, CAN_SIGNAL_SCREEN_MODE, 1));
  ASSERT(expectCommand(1, "read map", 1, CAN_SERVICE_READ_REQUEST, CAN_SIGNAL_MAP, 0));
  ASSERT(expectCommand(1, "temp 17", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "temp 255", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "volume 31", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "volume 255", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "fan 9", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "fan 255", 0, 0, 0, 0));
  ASSERT(expectCommand(1, "temp cold", 0, 0, 0, 0));
  return 1;
}
