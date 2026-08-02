#include "CanRxTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"
#include "../../../can/CanDriver.h"
#include "../../../can/CanMonitor.h"
#include "../../../can/CanProtocol.h"
#include "../../../hmi/HeadUnitHmi.h"

void headUnitCanRxTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    canDriverPollHealth();

    CanFrame frame;
    while (canDriverReceive(frame)) {
      canMonitorPrintFrame("HU RX", frame);

      if (frame.dlc != GDS_CAN_DLC) {
        Serial.println("HU RX DROP:DLC");
        continue;
      }

      CanPayload payload = canPayloadFromBytes(frame.data);
      canMonitorPrintPayloadSummary(payload);

      if (!canValidateChecksum(payload)) {
        Serial.println("HU RX DROP:CHECKSUM");
        continue;
      }

      if (frame.id == GDS_CAN_ID_HVAC_STATUS) {
        Serial.println("HU RX APPLY:HVAC_STATUS");
        headUnitHmiApplyCanPayload(payload);
      } else if (frame.id == GDS_CAN_ID_CONTROL_RESPONSE) {
        Serial.println("HU RX ACK:CONTROL_RESPONSE");
      } else if (frame.id == GDS_CAN_ID_CONTROL_REQUEST) {
        Serial.println("HU RX OBSERVE:CONTROL_REQUEST");
      } else {
        Serial.print("HU RX IGNORE ID:0x");
        Serial.println(frame.id, HEX);
      }
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
