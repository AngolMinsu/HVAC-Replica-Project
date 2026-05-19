#include "CanRxTask.h"

#include "../../../can/CanDriver.h"
#include "../../../can/CanMonitor.h"
#include "../../../can/CanProtocol.h"
#include "../../../hmi/HuRtosContext.h"

void huCanRxTask(void* parameter) {
  HuRtosContext* context = static_cast<HuRtosContext*>(parameter);
  TickType_t delayTicks = pdMS_TO_TICKS(5);

  for (;;) {
    CanFrame frame;
    if (canDriverReceive(frame)) {
      canMonitorPrintFrame("RX", frame);

      if (frame.dlc == CAN_DLC && (frame.id == CAN_ID_CONTROL_RESPONSE || frame.id == CAN_ID_HVAC_STATUS)) {
        HuStateEvent event;
        event.canId = frame.id;
        event.payload = canPayloadFromBytes(frame.data);
        event.rxTick = xTaskGetTickCount();
        canMonitorPrintPayloadSummary(event.payload);
        xQueueSend(context->canRxQueue, &event, 0);
      }
    }

    vTaskDelay(delayTicks);
  }
}
