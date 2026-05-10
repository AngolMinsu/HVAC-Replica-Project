#include "../../../MKBD/can/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 4, 0, (uint8_t)(2 + i));
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
  }

  CanPayload minPayload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 0, 0, 0);
  ASSERT_EQUALS(7, canValidateChecksum(minPayload), 1);
  CanPayload maxPayload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 0xFF, 0xFF, 0xFF);
  ASSERT_EQUALS(8, canValidateChecksum(maxPayload), 1);
  return 1;
}
