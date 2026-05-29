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
    if (canDriverReceive(frame)) {
      canMonitorPrintFrame("RX", frame);
      if (frame.id == GDS_CAN_ID_HVAC_STATUS && frame.dlc == GDS_CAN_DLC) {
        CanPayload payload = canPayloadFromBytes(frame.data);
        canMonitorPrintPayloadSummary(payload);
        headUnitHmiApplyCanPayload(payload);
      }
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
