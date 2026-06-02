#include "GatewayRouter.h"

#include <Arduino.h>
#include "../../GDS.h"
#include "../can/CanMonitor.h"
#include "../can/CanProtocol.h"

static uint32_t dropUnknownIdCount = 0;
static uint32_t dropInvalidDlcCount = 0;
static uint32_t dropChecksumCount = 0;
static uint32_t dropUnknownSignalCount = 0;
static uint32_t routeCount = 0;
static uint32_t lastStatsLogMs = 0;

static uint8_t isRoutableId(uint16_t id) {
  return id == GDS_CAN_ID_CONTROL_REQUEST ||
         id == GDS_CAN_ID_CONTROL_RESPONSE ||
         id == GDS_CAN_ID_HVAC_STATUS;
}

uint8_t gatewayRouteFrame(const char* sourceName, const CanFrame& rxFrame, CanFrame& txFrame) {
  (void)sourceName;

  if (!isRoutableId(rxFrame.id)) {
    dropUnknownIdCount++;
    return 0;
  }

  if (rxFrame.dlc != GDS_CAN_DLC) {
    dropInvalidDlcCount++;
    return 0;
  }

  CanPayload payload = canPayloadFromBytes(rxFrame.data);
  canMonitorPrintPayloadSummary(payload);

  if (!canValidateChecksum(payload)) {
    dropChecksumCount++;
    return 0;
  }

  if (!canIsKnownSignal(payload.signal)) {
    dropUnknownSignalCount++;
    return 0;
  }

  txFrame = rxFrame;
  routeCount++;
  return 1;
}

void gatewayRouterPrintStats() {
  uint32_t now = millis();
  if (now - lastStatsLogMs < 1000) {
    return;
  }
  lastStatsLogMs = now;

  if (routeCount == 0 &&
      dropUnknownIdCount == 0 &&
      dropInvalidDlcCount == 0 &&
      dropChecksumCount == 0 &&
      dropUnknownSignalCount == 0) {
    return;
  }

  Serial.print("GW route:");
  Serial.print(routeCount);
  Serial.print(" drop[id:");
  Serial.print(dropUnknownIdCount);
  Serial.print(" dlc:");
  Serial.print(dropInvalidDlcCount);
  Serial.print(" checksum:");
  Serial.print(dropChecksumCount);
  Serial.print(" signal:");
  Serial.print(dropUnknownSignalCount);
  Serial.println("]");

  routeCount = 0;
  dropUnknownIdCount = 0;
  dropInvalidDlcCount = 0;
  dropChecksumCount = 0;
  dropUnknownSignalCount = 0;
}
