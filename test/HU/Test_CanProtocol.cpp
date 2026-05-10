#include "../../HU/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_CanProtocol() {
  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 3, 0, 7);
  ASSERT_EQUALS(0, payload.service, CAN_SERVICE_WRITE_REQUEST);
  ASSERT_EQUALS(1, payload.signal, CAN_SIGNAL_FAN_SPEED);
  ASSERT_EQUALS(2, payload.value, 3);
  ASSERT_EQUALS(3, canValidateChecksum(payload), 1);

  uint8_t bytes[CAN_DLC];
  canPayloadToBytes(payload, bytes);
  CanPayload parsed = canPayloadFromBytes(bytes);

  ASSERT_EQUALS(4, parsed.service, payload.service);
  ASSERT_EQUALS(5, parsed.result, payload.result);
  ASSERT_EQUALS(6, parsed.signal, payload.signal);
  ASSERT_EQUALS(7, parsed.value, payload.value);
  ASSERT_EQUALS(8, parsed.checksum, payload.checksum);

  ASSERT_EQUALS(9, canSignalToText(CAN_SIGNAL_VOLUME)[0], 'V');
  ASSERT_EQUALS(10, canServiceToText(CAN_SERVICE_READ_REQUEST)[0], 'R');

  parsed.checksum ^= 0xFF;
  ASSERT_EQUALS(11, canValidateChecksum(parsed), 0);
  return 1;
}
