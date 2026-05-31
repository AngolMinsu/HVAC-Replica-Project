#include "CanTask.h"

#include "../../../../GDS.h"
#include "../../../can/CanDriver.h"
#include "../../../can/CanMonitor.h"
#include "../../../gateway/GatewayRouter.h"

void gatewayCanTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    canDriverPollHealth();

    CanFrame rxFrame = {};
    while (canDriverReceive(rxFrame)) {
      canMonitorPrintFrame("GW RX", rxFrame);

      CanFrame txFrame = {};
      if (gatewayRouteFrame(rxFrame, txFrame)) {
        if (GDS_GW_FORWARD_ENABLED) {
          uint8_t sent = canDriverSend(txFrame);
          Serial.println(sent ? "GW TX OK" : "GW TX FAIL");
        }
      }
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
