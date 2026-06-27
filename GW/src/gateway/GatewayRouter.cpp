#include "GatewayRouter.h"

#include <Arduino.h>
#include "../../GDS.h"
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
  if (!isRoutableId(rxFrame.id)) {
    Serial.print("GW ROUTE ");
    Serial.print(sourceName);
    Serial.print(" DROP ID:0x");
    Serial.println(rxFrame.id, HEX);
    dropUnknownIdCount++;
    return 0;
  }

  if (rxFrame.dlc != GDS_CAN_DLC) {
    Serial.print("GW ROUTE ");
    Serial.print(sourceName);
    Serial.print(" DROP DLC:");
    Serial.println(rxFrame.dlc);
    dropInvalidDlcCount++;
    return 0;
  }

  CanPayload payload = canPayloadFromBytes(rxFrame.data);
if (!canValidateChecksum(payload)) {
    Serial.print("GW ROUTE ");
    Serial.print(sourceName);
    Serial.println(" DROP CHECKSUM");
    dropChecksumCount++;
    return 0;
  }

  if (!canIsKnownSignal(payload.signal)) {
    Serial.print("GW ROUTE ");
    Serial.print(sourceName);
    Serial.print(" DROP SIGNAL:0x");
    Serial.println(payload.signal, HEX);
    dropUnknownSignalCount++;
    return 0;
  }

  txFrame = rxFrame;
  routeCount++;
  return 1;
}

void gatewayRouterPrintStats() {
}
