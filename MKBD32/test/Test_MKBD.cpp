#include <stdio.h>
#include <string.h>
#include "../can/CanProtocol.h"
#include "../state/State.h"

#define CHECK(x) do { if (!(x)) { printf("[FAIL] %s:%d %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

int main() {
  CanPayload p = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL,
                                CAN_SIGNAL_FAN_SPEED, 8, CAN_ERROR_NONE, 7);
  CHECK(p.reserved == 0);
  CHECK(canValidateChecksum(p));
  uint8_t bytes[CAN_DLC];
  canPayloadToBytes(p, bytes);
  CanPayload parsed = canPayloadFromBytes(bytes);
  CHECK(memcmp(&p, &parsed, sizeof(p)) == 0);
  parsed.checksum ^= 1;
  CHECK(!canValidateChecksum(parsed));

  SystemState state;
  initSystemState(state);
  CHECK(state.screenMode == SCREEN_DATC);
  CHECK(state.fanSpeed == GDS_FAN_SPEED_MIN);
  CHECK(state.driverTemp == GDS_TEMP_DEFAULT);
  CHECK(state.passengerTemp == GDS_TEMP_DEFAULT);
  CHECK(state.volume == GDS_VOLUME_DEFAULT);
  CHECK(state.mediaIndex == GDS_MEDIA_INDEX_DEFAULT);
  toggleScreenMode(state); CHECK(state.screenMode == SCREEN_INFO);
  toggleScreenMode(state); CHECK(state.screenMode == SCREEN_DATC);
  CHECK(screenModeToText(SCREEN_INFO)[0] == 'I');
  CHECK(hvacModeToText(HVAC_AUTO)[0] == 'A');
  CHECK(windModeToText(WIND_DEF)[0] == 'D');
  puts("[PASS] MKBD unit tests");
  return 0;
}
