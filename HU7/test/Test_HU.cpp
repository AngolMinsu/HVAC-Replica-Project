#include <stdio.h>
#include <string.h>
#include "../src/can/CanProtocol.h"

#define CHECK(x) do { if (!(x)) { printf("[FAIL] %s:%d %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

int main() {
  CanPayload p = {GDS_CAN_SERVICE_WRITE_RESPONSE, GDS_CAN_RESULT_SUCCESS,
                  GDS_CAN_SIGNAL_VOLUME, 30, 0, 0, 9, 0};
  p.checksum = canCalculateChecksum(p);
  CHECK(canValidateChecksum(p));
  uint8_t bytes[CAN_DLC];
  canPayloadToBytes(p, bytes);
  CanPayload parsed = canPayloadFromBytes(bytes);
  CHECK(memcmp(&p, &parsed, sizeof(p)) == 0);
  parsed.value ^= 1;
  CHECK(!canValidateChecksum(parsed));
  CHECK(strcmp(canSignalToText(GDS_CAN_SIGNAL_POWER), "Power") == 0);
  CHECK(strcmp(canSignalToText(GDS_CAN_SIGNAL_HU_OPEN_MEDIA), "HuOpenMedia") == 0);
  CHECK(strcmp(canSignalToText(0xFF), "Unknown") == 0);
  CHECK(CAN_ID_CONTROL_REQUEST == 0x100);
  CHECK(CAN_ID_CONTROL_RESPONSE == 0x101);
  CHECK(CAN_ID_HVAC_STATUS == 0x300);
  puts("[PASS] HU unit tests");
  return 0;
}
