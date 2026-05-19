#include "../../HU32/src/can/CanProtocol.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_Hu32CanProtocol(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_FAN_SPEED, 8, 0, 1);
    ASSERT_EQUALS(0, canValidateChecksum(payload), 1);

    uint8_t bytes[CAN_DLC] = {0};
    canPayloadToBytes(payload, bytes);
    ASSERT_EQUALS(1, bytes[0], CAN_SERVICE_WRITE_RESPONSE);
    ASSERT_EQUALS(2, bytes[1], CAN_RESULT_SUCCESS);
    ASSERT_EQUALS(3, bytes[2], CAN_SIGNAL_FAN_SPEED);
    ASSERT_EQUALS(4, bytes[3], 8);

    CanPayload parsed = canPayloadFromBytes(bytes);
    ASSERT_EQUALS(5, parsed.service, payload.service);
    ASSERT_EQUALS(6, parsed.result, payload.result);
    ASSERT_EQUALS(7, parsed.signal, payload.signal);
    ASSERT_EQUALS(8, parsed.value, payload.value);
    ASSERT_EQUALS(9, parsed.checksum, payload.checksum);

    parsed.checksum ^= 0x01;
    ASSERT_EQUALS(10, canValidateChecksum(parsed), 0);

    ASSERT_EQUALS(11, canSignalToText(CAN_SIGNAL_HU_FOCUS_NEXT)[0], 'H');
    ASSERT_EQUALS(12, canSignalToText(CAN_SIGNAL_HU_OPEN_MEDIA)[0], 'H');
    ASSERT_EQUALS(13, canServiceToText(CAN_SERVICE_WRITE_RESPONSE)[0], 'W');
  }

  return 1;
}
