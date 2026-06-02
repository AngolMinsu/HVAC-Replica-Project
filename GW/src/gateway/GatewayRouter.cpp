#include "GatewayRouter.h"

#include <Arduino.h>
#include "../../GDS.h"
#include "../can/CanMonitor.h"
#include "../can/CanProtocol.h"

static uint8_t isRoutableId(uint16_t id) {
  return id == GDS_CAN_ID_CONTROL_REQUEST ||
         id == GDS_CAN_ID_CONTROL_RESPONSE ||
         id == GDS_CAN_ID_HVAC_STATUS;
}

uint8_t gatewayRouteFrame(const char* sourceName, const CanFrame& rxFrame, CanFrame& txFrame) {
  if (!isRoutableId(rxFrame.id)) {
    Serial.print("GW DROP ");
    Serial.print(sourceName);
    Serial.print(" unknown ID:0x");
    Serial.println(rxFrame.id, HEX);
    return 0;
  }

  if (rxFrame.dlc != GDS_CAN_DLC) {
    Serial.print("GW DROP ");
    Serial.print(sourceName);
    Serial.print(" invalid DLC:");
    Serial.println(rxFrame.dlc);
    return 0;
  }

  CanPayload payload = canPayloadFromBytes(rxFrame.data);
  canMonitorPrintPayloadSummary(payload);

  if (!canValidateChecksum(payload)) {
    Serial.print("GW DROP ");
    Serial.print(sourceName);
    Serial.println(" checksum");
    return 0;
  }

  if (!canIsKnownSignal(payload.signal)) {
    Serial.print("GW DROP ");
    Serial.print(sourceName);
    Serial.print(" unknown signal:0x");
    Serial.println(payload.signal, HEX);
    return 0;
  }

  txFrame = rxFrame;
  Serial.print("GW ROUTE ");
  Serial.println(sourceName);
  return 1;
}
