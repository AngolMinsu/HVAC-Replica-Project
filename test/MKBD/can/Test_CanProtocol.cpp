#include "../../../MKBD/can/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol() {
  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 4, 0, 2);
  ASSERT_EQUALS(0, payload.reserved, 0);
  ASSERT_EQUALS(1, canValidateChecksum(payload), 1);

  uint8_t bytes[CAN_DLC];
  canPayloadToBytes(payload, bytes);
  CanPayload parsed = canPayloadFromBytes(bytes);
  ASSERT_EQUALS(2, parsed.service, payload.service);
  ASSERT_EQUALS(3, parsed.signal, payload.signal);
  ASSERT_EQUALS(4, parsed.value, payload.value);
  ASSERT_EQUALS(5, parsed.checksum, payload.checksum);

  parsed.checksum ^= 0x01;
  ASSERT_EQUALS(6, canValidateChecksum(parsed), 0);
  return 1;
}
